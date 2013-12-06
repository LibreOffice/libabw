/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string.h>
#include <boost/spirit/include/classic.hpp>
#include <boost/algorithm/string.hpp>
#include <librevenge/librevenge.h>
#include "ABWCollector.h"
#include "libabw_internal.h"

#define ABW_EPSILON 1.0E-06

namespace libabw
{

namespace
{

enum ABWUnit
{
  ABW_NONE,
  ABW_CM,
  ABW_IN,
  ABW_MM,
  ABW_PI,
  ABW_PT,
  ABW_PX,
  ABW_PERCENT
};

static bool findDouble(const char *str, double &res, ABWUnit &unit)
{
  using namespace ::boost::spirit::classic;

  if (!str || !strlen(str))
    return false;

  unit = ABW_NONE;

  if (!parse(str,
             //  Begin grammar
             (
               real_p[assign_a(res)] >>
               (
                 str_p("cm")[assign_a(unit,ABW_CM)]
                 |
                 str_p("in")[assign_a(unit,ABW_IN)]
                 |
                 str_p("mm")[assign_a(unit,ABW_MM)]
                 |
                 str_p("pi")[assign_a(unit,ABW_PI)]
                 |
                 str_p("pt")[assign_a(unit,ABW_PT)]
                 |
                 str_p("px")[assign_a(unit,ABW_PT)]
                 |
                 str_p("%")[assign_a(unit,ABW_PERCENT)]
                 |
                 eps_p
               )
             ) >> end_p,
             //  End grammar
             space_p).full)
  {
    return false;
  }

  if (unit == ABW_PERCENT)
    res /= 100.0;
  if (unit == ABW_PI)
  {
    res = res / 6.0;
    unit = ABW_IN;
  }
  if (unit == ABW_PT || unit == ABW_PX)
  {
    res = res / 72.0;
    unit = ABW_IN;
  }
  if (unit == ABW_CM)
  {
    res = res / 2.54;
    unit = ABW_IN;
  }
  if (unit == ABW_MM)
  {
    res = res / 25.4;
    unit = ABW_IN;
  }
  if (unit == ABW_NONE)
    unit = ABW_PERCENT;

  return true;
}

bool findInt(const char *str, int &res)
{
  using namespace ::boost::spirit::classic;

  if (!str || !strlen(str))
    return false;

  return parse(str,
               //  Begin grammar
               (
                 int_p[assign_a(res)]
               ) >> end_p,
               //  End grammar
               space_p).full;
}

bool findBool(const char *str, bool &res)
{
  using namespace ::boost::spirit::classic;

  if (!str || !strlen(str))
    return false;

  return parse(str,
               //  Begin grammar
               (
                 str_p("true")[assign_a(res,true)]
                 |
                 str_p("false")[assign_a(res,false)]
                 |
                 str_p("TRUE")[assign_a(res,true)]
                 |
                 str_p("FALSE")[assign_a(res,false)]
               ) >> end_p,
               //  End grammar
               space_p).full;
}

static std::string getColor(const std::string &s)
{
  if (s[0] == '#')
  {
    if (s.length() != 7)
      return std::string();
    else
      return s;
  }
  else if (s.length() != 6)
    return std::string();

  std::string out = ("#");
  out.append(s);
  return out;
}

static void parsePropString(const char *str, std::map<std::string, std::string> &props)
{
  if (!str || !strlen(str))
    return;

  std::string propString(str);
  boost::trim(propString);
  std::vector<std::string> strVec;
  boost::algorithm::split(strVec, propString, boost::is_any_of(";"), boost::token_compress_on);
  for (std::vector<std::string>::size_type i = 0; i < strVec.size(); ++i)
  {
    boost::algorithm::trim(strVec[i]);
    std::vector<std::string> tmpVec;
    boost::algorithm::split(tmpVec, strVec[i], boost::is_any_of(":"), boost::token_compress_on);
    if (tmpVec.size() == 2)
      props[tmpVec[0]] = tmpVec[1];
  }
}

static void separateTabsAndInsertText(librevenge::RVNGTextInterface *iface, const librevenge::RVNGString &text)
{
  if (!iface || text.empty())
    return;
  librevenge::RVNGString tmpText;
  librevenge::RVNGString::Iter i(text);
  for (i.rewind(); i.next();)
  {
    if (*(i()) == '\t')
    {
      if (!tmpText.empty())
      {
        if (iface)
          iface->insertText(tmpText);
        tmpText.clear();
      }

      if (iface)
        iface->insertTab();
    }
    else if (*(i()) == '\n')
    {
      if (!tmpText.empty())
      {
        if (iface)
          iface->insertText(tmpText);
        tmpText.clear();
      }

      if (iface)
        iface->insertLineBreak();
    }
    else
    {
      tmpText.append(i());
    }
  }
  if (iface && !tmpText.empty())
    iface->insertText(tmpText);
}

static void separateSpacesAndInsertText(librevenge::RVNGTextInterface *iface, const librevenge::RVNGString &text)
{
  if (!iface)
    return;
  if (text.empty())
  {
    iface->insertText(text);
    return;
  }
  librevenge::RVNGString tmpText;
  int numConsecutiveSpaces = 0;
  librevenge::RVNGString::Iter i(text);
  for (i.rewind(); i.next();)
  {
    if (*(i()) == ' ')
      numConsecutiveSpaces++;
    else
      numConsecutiveSpaces = 0;

    if (numConsecutiveSpaces > 1)
    {
      if (!tmpText.empty())
      {
        separateTabsAndInsertText(iface, tmpText);
        tmpText.clear();
      }

      if (iface)
        iface->insertSpace();
    }
    else
    {
      tmpText.append(i());
    }
  }
  separateTabsAndInsertText(iface, tmpText);
}

void parseTableColumns(const char *str, librevenge::RVNGPropertyListVector &columns)
{
  if (!str || !strlen(str))
    return;

  std::string propString(str);
  boost::trim(propString);
  std::vector<std::string> strVec;
  boost::algorithm::split(strVec, propString, boost::is_any_of("/"), boost::token_compress_on);
  std::vector<double> doubleVec;
  for (std::vector<std::string>::size_type i = 0; i < strVec.size(); ++i)
  {
    ABWUnit unit(ABW_NONE);
    double value(0.0);
    boost::algorithm::trim(strVec[i]);
    if (!findDouble(strVec[i].c_str(), value, unit) || ABW_IN != unit)
      return;
    doubleVec.push_back(value);
  }
  for (std::vector<double>::const_iterator iter = doubleVec.begin(); iter != doubleVec.end(); ++iter)
  {
    librevenge::RVNGPropertyList propList;
    propList.insert("style:column-width", *iter);
    columns.append(propList);
  }
}

std::string decodeUrl(const std::string src)
{
  std::string result;
  for (unsigned i=0; i < src.length(); ++i)
  {
    if (src[i]=='%')
    {
      unsigned ii;
      sscanf(src.substr(i+1,2).c_str(), "%x", &ii);
      result += static_cast<char>(ii);
      i=i+2;
    }
    else
      result+=src[i];
  }
  return result;
}

} // anonymous namespace

} // namespace libabw

libabw::ABWParsingState::ABWParsingState() :
  m_isDocumentStarted(false),
  m_isPageSpanOpened(false),
  m_isSectionOpened(false),

  m_isSpanOpened(false),
  m_isParagraphOpened(false),

  m_currentSectionStyle(),
  m_currentParagraphStyle(),
  m_currentCharacterStyle(),

  m_textStyles(),

  m_pageWidth(0.0),
  m_pageHeight(0.0),
  m_pageMarginTop(0.0),
  m_pageMarginBottom(0.0),
  m_pageMarginLeft(0.0),
  m_pageMarginRight(0.0),

  m_deferredPageBreak(false),
  m_deferredColumnBreak(false)
{
}

libabw::ABWParsingState::ABWParsingState(const ABWParsingState &ps) :
  m_isDocumentStarted(ps.m_isDocumentStarted),
  m_isPageSpanOpened(ps.m_isPageSpanOpened),
  m_isSectionOpened(ps.m_isSectionOpened),

  m_isSpanOpened(ps.m_isSpanOpened),
  m_isParagraphOpened(ps.m_isParagraphOpened),

  m_currentSectionStyle(ps.m_currentSectionStyle),
  m_currentParagraphStyle(ps.m_currentParagraphStyle),
  m_currentCharacterStyle(ps.m_currentCharacterStyle),

  m_textStyles(ps.m_textStyles),

  m_pageWidth(ps.m_pageWidth),
  m_pageHeight(ps.m_pageHeight),
  m_pageMarginTop(ps.m_pageMarginTop),
  m_pageMarginBottom(ps.m_pageMarginBottom),
  m_pageMarginLeft(ps.m_pageMarginLeft),
  m_pageMarginRight(ps.m_pageMarginRight),

  m_deferredPageBreak(ps.m_deferredPageBreak),
  m_deferredColumnBreak(ps.m_deferredColumnBreak)
{
}

libabw::ABWParsingState::~ABWParsingState()
{
}

libabw::ABWCollector::ABWCollector(librevenge::RVNGTextInterface *iface) :
  m_ps(new ABWParsingState),
  m_iface(iface),
  m_parsingStates(),
  m_dontLoop()
{
}

libabw::ABWCollector::~ABWCollector()
{
  DELETEP(m_ps);
}

void libabw::ABWCollector::collectTextStyle(const char *name, const char *basedon, const char *followedby, const char *props)
{
  ABWStyle style;
  style.basedon = basedon ? basedon : std::string();
  style.followedby = followedby ? followedby : std::string();
  parsePropString(props, style.properties);
  if (name)
    m_ps->m_textStyles[name] = style;
}

void libabw::ABWCollector::_recurseTextProperties(const char *name, std::map<std::string, std::string> &styleProps)
{
  if (name)
  {
    m_dontLoop.insert(name);
    std::map<std::string, ABWStyle>::const_iterator iter = m_ps->m_textStyles.find(name);
    if (iter != m_ps->m_textStyles.end())
    {
      if (!(iter->second.basedon.empty()) && !m_dontLoop.count(iter->second.basedon))
        _recurseTextProperties(iter->second.basedon.c_str(), styleProps);
      else
      {
        for (std::map<std::string, std::string>::const_iterator i = iter->second.properties.begin(); i != iter->second.properties.end(); ++i)
        {
          printf("%s --> %s\n", i->first.c_str(), i->second.c_str());
          styleProps[i->first] = i->second;
        }
      }
    }
  }
  if (!m_dontLoop.empty())
    m_dontLoop.clear();
}

void libabw::ABWCollector::collectParagraphProperties(const char *style, const char *props)
{
  m_ps->m_currentParagraphStyle.clear();
  if (style)
    _recurseTextProperties(style, m_ps->m_currentParagraphStyle);

  std::map<std::string, std::string> tmpProps;
  parsePropString(props, tmpProps);
  for (std::map<std::string, std::string>::const_iterator iter = tmpProps.begin(); iter != tmpProps.end(); ++iter)
    m_ps->m_currentParagraphStyle[iter->first] = iter->second;
}

void libabw::ABWCollector::collectCharacterProperties(const char *style, const char *props)
{
  m_ps->m_currentCharacterStyle.clear();
  if (style)
    _recurseTextProperties(style, m_ps->m_currentCharacterStyle);

  std::map<std::string, std::string> tmpProps;
  parsePropString(props, tmpProps);
  for (std::map<std::string, std::string>::const_iterator iter = tmpProps.begin(); iter != tmpProps.end(); ++iter)
    m_ps->m_currentCharacterStyle[iter->first] = iter->second;
}

void libabw::ABWCollector::collectSectionProperties(const char *props)
{
  double pageMarginLeft = m_ps->m_pageMarginLeft;
  double pageMarginRight = m_ps->m_pageMarginRight;
  double pageMarginTop = m_ps->m_pageMarginTop;
  double pageMarginBottom = m_ps->m_pageMarginBottom;

  m_ps->m_currentSectionStyle.clear();
  std::map<std::string, std::string> tmpProps;
  parsePropString(props, tmpProps);
  ABWUnit unit(ABW_NONE);
  double value(0.0);
  for (std::map<std::string, std::string>::const_iterator iter = tmpProps.begin(); iter != tmpProps.end(); ++iter)
  {
    if (iter->first == "page-margin-right" && !iter->second.empty() && fabs(m_ps->m_pageMarginRight) < ABW_EPSILON)
    {
      if (findDouble(iter->second.c_str(), value, unit))
      {
        if (unit == ABW_IN && value > 0.0 && fabs(value) > ABW_EPSILON)
          m_ps->m_pageMarginRight = value;
      }
    }
    else if (iter->first == "page-margin-left" && !iter->second.empty() && fabs(m_ps->m_pageMarginLeft) < ABW_EPSILON)
    {
      if (findDouble(iter->second.c_str(), value, unit))
      {
        if (unit == ABW_IN && value > 0.0 && fabs(value) > ABW_EPSILON)
          m_ps->m_pageMarginLeft = value;
      }
    }
    else if (iter->first == "page-margin-top" && !iter->second.empty() && fabs(m_ps->m_pageMarginTop) < ABW_EPSILON)
    {
      if (findDouble(iter->second.c_str(), value, unit))
      {
        if (unit == ABW_IN && value > 0.0 && fabs(value) > ABW_EPSILON)
          m_ps->m_pageMarginTop = value;
      }
    }
    else if (iter->first == "page-margin-bottom" && !iter->second.empty() && fabs(m_ps->m_pageMarginBottom) < ABW_EPSILON)
    {
      if (findDouble(iter->second.c_str(), value, unit))
      {
        if (unit == ABW_IN && value > 0.0 && fabs(value) > ABW_EPSILON)
          m_ps->m_pageMarginBottom = value;
      }
    }

    m_ps->m_currentSectionStyle[iter->first] = iter->second;
  }
  if (fabs(m_ps->m_pageMarginRight) < ABW_EPSILON)
    m_ps->m_pageMarginRight = 1.0;
  if (fabs(m_ps->m_pageMarginLeft) < ABW_EPSILON)
    m_ps->m_pageMarginLeft = 1.0;
  if (fabs(m_ps->m_pageMarginTop) < ABW_EPSILON)
    m_ps->m_pageMarginTop = 1.0;
  if (fabs(m_ps->m_pageMarginBottom) < ABW_EPSILON)
    m_ps->m_pageMarginBottom = 1.0;

  if (fabs(pageMarginLeft-m_ps->m_pageMarginLeft) > ABW_EPSILON ||
      fabs(pageMarginRight-m_ps->m_pageMarginRight) > ABW_EPSILON ||
      fabs(pageMarginTop-m_ps->m_pageMarginTop) > ABW_EPSILON ||
      fabs(pageMarginBottom-m_ps->m_pageMarginBottom) > ABW_EPSILON)
  {
    _closePageSpan();
  }
  else
  {
    _closeSection();
  }
}

void libabw::ABWCollector::collectPageSize(const char *width, const char *height, const char *units, const char * /* pageScale */)
{
  std::string widthStr(width);
  std::string heightStr(height);
  if (units)
  {
    widthStr.append(units);
    heightStr.append(units);
  }
  ABWUnit unit;
  double value;
  if (findDouble(widthStr.c_str(), value, unit))
  {
    if (unit == ABW_IN)
      m_ps->m_pageWidth = value;
  }
  if (findDouble(heightStr.c_str(), value, unit))
  {
    if (unit == ABW_IN)
      m_ps->m_pageHeight = value;
  }
}

void libabw::ABWCollector::startDocument()
{
  if (m_iface && !m_ps->m_isDocumentStarted)
    m_iface->startDocument();

  m_ps->m_isDocumentStarted = true;
}

void libabw::ABWCollector::endDocument()
{
  if (!m_ps->m_isPageSpanOpened)
    _openSpan();

  if (m_ps->m_isParagraphOpened)
    _closeParagraph();

  // close the document nice and tight
  _closeSection();
  _closePageSpan();
  if (m_iface)
    m_iface->endDocument();
}

void libabw::ABWCollector::endSection()
{
  _closeSection();
}

void libabw::ABWCollector::closeParagraph()
{
  // we have an empty paragraph, insert it
  if (!m_ps->m_isParagraphOpened)
    _openSpan();
  _closeParagraph();
  m_ps->m_currentParagraphStyle.clear();
}

void libabw::ABWCollector::openLink(const char *href)
{
  if (m_ps->m_isSpanOpened)
    _closeSpan();
  if (!m_ps->m_isParagraphOpened)
    _openParagraph();
  librevenge::RVNGPropertyList propList;
  if (href)
  {
    std::string sHref = decodeUrl(href);
    propList.insert("xlink:href", sHref.c_str());
  }
  if (m_iface)
    m_iface->openLink(propList);
  if (!m_ps->m_isSpanOpened)
    _openSpan();
}

void libabw::ABWCollector::closeLink()
{
  if (m_ps->m_isSpanOpened)
    _closeSpan();
  if (m_iface)
    m_iface->closeLink();
}

void libabw::ABWCollector::closeSpan()
{
  _closeSpan();
  m_ps->m_currentCharacterStyle.clear();
}

void libabw::ABWCollector::insertLineBreak()
{
  if (!m_ps->m_isSpanOpened)
    _openSpan();

  if (m_iface)
    m_iface->insertLineBreak();
}

void libabw::ABWCollector::insertColumnBreak()
{
  _closeParagraph();
  m_ps->m_deferredColumnBreak = true;
}

void libabw::ABWCollector::insertPageBreak()
{
  _closeParagraph();
  m_ps->m_deferredPageBreak = true;
}

void libabw::ABWCollector::insertText(const librevenge::RVNGString &text)
{
  if (!m_ps->m_isSpanOpened)
    _openSpan();

  if (m_iface)
    separateSpacesAndInsertText(m_iface, text);
}


void libabw::ABWCollector::_openPageSpan()
{
  if (!m_ps->m_isPageSpanOpened)
  {

    if (!m_ps->m_isDocumentStarted)
      startDocument();

    librevenge::RVNGPropertyList propList;
    propList.insert("fo:page-width", m_ps->m_pageWidth);
    propList.insert("fo:page-height", m_ps->m_pageHeight);
    propList.insert("fo:margin-left", m_ps->m_pageMarginLeft);
    propList.insert("fo:margin-right", m_ps->m_pageMarginRight);
    propList.insert("fo:margin-top", m_ps->m_pageMarginTop);
    propList.insert("fo:margin-bottom", m_ps->m_pageMarginBottom);

    if (m_iface && !m_ps->m_isPageSpanOpened)
      m_iface->openPageSpan(propList);

    m_ps->m_isPageSpanOpened = true;
  }
}

void libabw::ABWCollector::_closePageSpan()
{
  if (m_ps->m_isPageSpanOpened)
  {
    if (m_ps->m_isSectionOpened)
      _closeSection();

    if (m_iface)
      m_iface->closePageSpan();
  }
  m_ps->m_isPageSpanOpened = false;
}

void libabw::ABWCollector::_openSection()
{
  if (!m_ps->m_isSectionOpened)
  {
    if (!m_ps->m_isPageSpanOpened)
      _openPageSpan();

    librevenge::RVNGPropertyList propList;

    ABWUnit unit(ABW_NONE);
    double value(0.0);
    std::map<std::string, std::string>::const_iterator iter = m_ps->m_currentSectionStyle.find("page-margin-right");
    if (iter != m_ps->m_currentSectionStyle.end())
    {
      if (findDouble(iter->second.c_str(), value, unit))
      {
        if (unit == ABW_IN)
          propList.insert("fo:margin-right", value - m_ps->m_pageMarginRight);
      }
    }
    iter = m_ps->m_currentSectionStyle.find("page-margin-left");
    if (iter != m_ps->m_currentSectionStyle.end())
    {
      if (findDouble(iter->second.c_str(), value, unit))
      {
        if (unit == ABW_IN)
          propList.insert("fo:margin-left", value - m_ps->m_pageMarginLeft);
      }
    }
    iter = m_ps->m_currentSectionStyle.find("section-space-after");
    if (iter != m_ps->m_currentSectionStyle.end())
    {
      if (findDouble(iter->second.c_str(), value, unit))
      {
        if (unit == ABW_IN)
          propList.insert("librevenge:margin-bottom", value);
      }
    }
    iter = m_ps->m_currentSectionStyle.find("dom-dir");
    if (iter != m_ps->m_currentSectionStyle.end())
    {
      if (iter->second == "ltr")
        propList.insert("style:writing-mode", "lr-tb");
      else if (iter->second == "rtl")
        propList.insert("style:writing-mode", "rl-tb");
    }

    iter = m_ps->m_currentSectionStyle.find("columns");
    if (iter != m_ps->m_currentSectionStyle.end())
    {
      int intValue(0);
      if (findInt(iter->second.c_str(), intValue))
      {
        if (intValue > 1)
        {
          librevenge::RVNGPropertyListVector columns;
          for (int i = 0; i < intValue; ++i)
          {
            librevenge::RVNGPropertyList column;
            column.insert("style:rel-width", 1.0 / (double)intValue, librevenge::RVNG_PERCENT);
            columns.append(column);
          }
          if (columns.count())
          {
            propList.insert("style:columns", columns);
            propList.insert("text:dont-balance-text-columns", true);
          }
        }
      }
    }

    if (!m_ps->m_isSectionOpened)
      m_iface->openSection(propList);

    m_ps->m_isSectionOpened = true;
  }
}

void libabw::ABWCollector::_openParagraph()
{
  if (!m_ps->m_isParagraphOpened)
  {
    if (!m_ps->m_isSectionOpened)
      _openSection();

    librevenge::RVNGPropertyList propList;
    ABWUnit unit(ABW_NONE);
    double value(0.0);
    int intValue(0);

    std::map<std::string, std::string>::const_iterator iter = m_ps->m_currentParagraphStyle.find("margin-right");
    if (iter != m_ps->m_currentParagraphStyle.end())
    {
      if (findDouble(iter->second.c_str(), value, unit))
      {
        if (unit == ABW_IN)
          propList.insert("fo:margin-right", value);
      }
    }
    iter = m_ps->m_currentParagraphStyle.find("margin-left");
    if (iter != m_ps->m_currentParagraphStyle.end())
    {
      if (findDouble(iter->second.c_str(), value, unit))
      {
        if (unit == ABW_IN)
          propList.insert("fo:margin-left", value);
      }
    }
    iter = m_ps->m_currentParagraphStyle.find("margin-top");
    if (iter != m_ps->m_currentParagraphStyle.end())
    {
      if (findDouble(iter->second.c_str(), value, unit))
      {
        if (unit == ABW_IN)
          propList.insert("fo:margin-top", value);
      }
    }
    iter = m_ps->m_currentParagraphStyle.find("margin-left");
    if (iter != m_ps->m_currentParagraphStyle.end())
    {
      if (findDouble(iter->second.c_str(), value, unit))
      {
        if (unit == ABW_IN)
          propList.insert("fo:margin-left", value);
      }
    }
    iter = m_ps->m_currentParagraphStyle.find("text-indent");
    if (iter != m_ps->m_currentParagraphStyle.end())
    {
      if (findDouble(iter->second.c_str(), value, unit))
      {
        if (unit == ABW_IN)
          propList.insert("fo:text-indent", value);
      }
    }
    iter = m_ps->m_currentParagraphStyle.find("text-align");
    if (iter != m_ps->m_currentParagraphStyle.end())
    {
      if (iter->second == "left")
        propList.insert("fo:text-align", "start");
      else if (iter->second == "right")
        propList.insert("fo:text-align", "end");
      else
        propList.insert("fo:text-align", iter->second.c_str());
    }
    iter = m_ps->m_currentParagraphStyle.find("line-height");
    if (iter != m_ps->m_currentParagraphStyle.end())
    {
      std::string propName("fo:line-height");
      std::string lineHeight = iter->second;
      size_t position = lineHeight.find_last_of('+');
      if (position && position != std::string::npos)
      {
        propName = "style:line-height-at-least";
        lineHeight.erase(position);
      }
      if (findDouble(lineHeight.c_str(), value, unit))
      {
        if (ABW_IN == unit)
          propList.insert(propName.c_str(), value);
        else if (ABW_PERCENT == unit)
          propList.insert(propName.c_str(), value, librevenge::RVNG_PERCENT);
      }
    }
    iter = m_ps->m_currentParagraphStyle.find("orphans");
    if (iter != m_ps->m_currentParagraphStyle.end())
    {
      if (findInt(iter->second.c_str(), intValue))
        propList.insert("fo:orphans", intValue);
    }
    iter = m_ps->m_currentParagraphStyle.find("widows");
    if (iter != m_ps->m_currentParagraphStyle.end())
    {
      if (findInt(iter->second.c_str(), intValue))
        propList.insert("fo:widows", intValue);
    }

    iter = m_ps->m_currentParagraphStyle.find("dom-dir");
    if (iter != m_ps->m_currentParagraphStyle.end())
    {
      if (iter->second == "ltr")
        propList.insert("style:writing-mode", "lr-tb");
      else if (iter->second == "rtl")
        propList.insert("style:writing-mode", "rl-tb");
    }

    if (m_ps->m_deferredPageBreak)
      propList.insert("fo:break-before", "page");
    else if (m_ps->m_deferredColumnBreak)
      propList.insert("fo:break-before", "column");
    m_ps->m_deferredPageBreak = false;
    m_ps->m_deferredColumnBreak = false;

    librevenge::RVNGPropertyListVector tabStops;

    if (tabStops.count())
      propList.insert("style:tab-stops", tabStops);

    if (m_iface)
      m_iface->openParagraph(propList);

    m_ps->m_isParagraphOpened = true;
  }
}

void libabw::ABWCollector::_openSpan()
{
  if (!m_ps->m_isSpanOpened)
  {
    if (!m_ps->m_isParagraphOpened)
      _openParagraph();

    librevenge::RVNGPropertyList propList;
    ABWUnit unit(ABW_NONE);
    double value(0.0);

    std::map<std::string, std::string>::const_iterator iter = m_ps->m_currentCharacterStyle.find("font-size");
    if (iter != m_ps->m_currentCharacterStyle.end())
    {
      if (findDouble(iter->second.c_str(), value, unit))
      {
        if (unit == ABW_IN)
          propList.insert("fo:font-size", value);
      }
    }
    else
    {
      iter = m_ps->m_currentParagraphStyle.find("font-size");
      if (iter != m_ps->m_currentParagraphStyle.end())
      {
        if (findDouble(iter->second.c_str(), value, unit))
        {
          if (unit == ABW_IN)
            propList.insert("fo:font-size", value);
        }
      }
    }

    iter = m_ps->m_currentCharacterStyle.find("font-family");
    if (iter != m_ps->m_currentCharacterStyle.end())
    {
      if (!iter->second.empty())
        propList.insert("style:font-name", iter->second.c_str());
    }
    else
    {
      iter = m_ps->m_currentParagraphStyle.find("font-family");
      if (iter != m_ps->m_currentParagraphStyle.end())
      {
        {
          if (!iter->second.empty())
            propList.insert("style:font-name", iter->second.c_str());
        }
      }
    }

    iter = m_ps->m_currentCharacterStyle.find("font-style");
    if (iter != m_ps->m_currentCharacterStyle.end())
    {
      if (!iter->second.empty() && iter->second != "normal")
        propList.insert("fo:font-style", iter->second.c_str());
    }
    else
    {
      iter = m_ps->m_currentParagraphStyle.find("font-style");
      if (iter != m_ps->m_currentParagraphStyle.end())
      {
        {
          if (!iter->second.empty() && iter->second != "normal")
            propList.insert("fo:font-style", iter->second.c_str());
        }
      }
    }

    iter = m_ps->m_currentCharacterStyle.find("font-weight");
    if (iter != m_ps->m_currentCharacterStyle.end())
    {
      if (!iter->second.empty() && iter->second != "normal")
        propList.insert("fo:font-weight", iter->second.c_str());
    }
    else
    {
      iter = m_ps->m_currentParagraphStyle.find("font-weight");
      if (iter != m_ps->m_currentParagraphStyle.end())
      {
        {
          if (!iter->second.empty() && iter->second != "normal")
            propList.insert("fo:font-weight", iter->second.c_str());
        }
      }
    }

    iter = m_ps->m_currentCharacterStyle.find("text-decoration");
    if (iter != m_ps->m_currentCharacterStyle.end())
    {
      if (!iter->second.empty())
      {
        if (iter->second == "underline")
          propList.insert("style:text-underline-type", "solid");
        else if (iter->second == "line-through")
          propList.insert("style:text-line-through-type", "single");
      }
    }
    else
    {
      iter = m_ps->m_currentParagraphStyle.find("text-decoration");
      if (iter != m_ps->m_currentParagraphStyle.end())
      {
        if (!iter->second.empty())
        {
          if (iter->second == "underline")
            propList.insert("style:text-underline-type", "solid");
          else if (iter->second == "line-through")
            propList.insert("style:text-line-through-type", "single");
        }
      }
    }

    iter = m_ps->m_currentCharacterStyle.find("color");
    if (iter != m_ps->m_currentCharacterStyle.end())
    {
      std::string color = getColor(iter->second);
      if (!color.empty())
        propList.insert("fo:color", color.c_str());
    }
    else
    {
      iter = m_ps->m_currentParagraphStyle.find("color");
      if (iter != m_ps->m_currentParagraphStyle.end())
      {
        std::string color = getColor(iter->second);
        if (!color.empty())
          propList.insert("fo:color", color.c_str());
      }
    }

    iter = m_ps->m_currentCharacterStyle.find("bgcolor");
    if (iter != m_ps->m_currentCharacterStyle.end())
    {
      std::string color = getColor(iter->second);
      if (!color.empty())
        propList.insert("fo:background-color", color.c_str());
    }
    else
    {
      iter = m_ps->m_currentParagraphStyle.find("bgcolor");
      if (iter != m_ps->m_currentParagraphStyle.end())
      {
        std::string color = getColor(iter->second);
        if (!color.empty())
          propList.insert("fo:background-color", color.c_str());
      }
    }

    iter = m_ps->m_currentCharacterStyle.find("text-position");
    if (iter != m_ps->m_currentCharacterStyle.end())
    {
      if (!iter->second.empty())
      {
        if (iter->second == "subscript")
          propList.insert("style:text-position", "sub");
        else if (iter->second == "superscript")
          propList.insert("style:text-position", "super");
      }
    }
    else
    {
      iter = m_ps->m_currentParagraphStyle.find("text-position");
      if (iter != m_ps->m_currentParagraphStyle.end())
      {
        if (!iter->second.empty())
        {
          if (iter->second == "subscript")
            propList.insert("style:text-position", "sub");
          else if (iter->second == "superscript")
            propList.insert("style:text-position", "super");
        }
      }
    }

    if (m_iface)
      m_iface->openSpan(propList);
  }
  m_ps->m_isSpanOpened = true;
}

void libabw::ABWCollector::_closeSection()
{
  if (m_ps->m_isSectionOpened)
  {
    if (m_ps->m_isParagraphOpened)
      _closeParagraph();

    m_iface->closeSection();

    m_ps->m_isSectionOpened = false;
  }
}

void libabw::ABWCollector::_closeParagraph()
{
  if (m_ps->m_isParagraphOpened)
  {
    if (m_ps->m_isSpanOpened)
      _closeSpan();

    if (m_iface)
      m_iface->closeParagraph();
  }

  m_ps->m_isParagraphOpened = false;
}


void libabw::ABWCollector::_closeSpan()
{
  if (m_ps->m_isSpanOpened && m_iface)
    m_iface->closeSpan();

  m_ps->m_isSpanOpened = false;
}

void libabw::ABWCollector::openFoot(const char *id)
{
  if (!m_ps->m_isParagraphOpened)
    _openSpan();
  _closeSpan();

  m_parsingStates.push(m_ps);
  m_ps = new ABWParsingState(*(m_parsingStates.top()));
  m_ps->m_isParagraphOpened = false;
  m_ps->m_currentParagraphStyle.clear();
  m_ps->m_currentCharacterStyle.clear();
  m_ps->m_deferredPageBreak = false;
  m_ps->m_deferredColumnBreak = false;

  librevenge::RVNGPropertyList propList;
  if (id)
    propList.insert("librevenge:number", id);
  if (m_iface)
    m_iface->openFootnote(propList);
}

void libabw::ABWCollector::closeFoot()
{
  if (m_iface)
    m_iface->closeFootnote();

  if (!m_parsingStates.empty())
  {
    delete m_ps;
    m_ps = m_parsingStates.top();
    m_parsingStates.pop();
  }
}

void libabw::ABWCollector::openEndnote(const char *id)
{
  if (!m_ps->m_isParagraphOpened)
    _openSpan();
  _closeSpan();

  m_parsingStates.push(m_ps);
  m_ps = new ABWParsingState(*(m_parsingStates.top()));
  m_ps->m_isParagraphOpened = false;
  m_ps->m_currentParagraphStyle.clear();
  m_ps->m_currentCharacterStyle.clear();
  m_ps->m_deferredPageBreak = false;
  m_ps->m_deferredColumnBreak = false;

  librevenge::RVNGPropertyList propList;
  if (id)
    propList.insert("librevenge:number", id);
  if (m_iface)
    m_iface->openEndnote(propList);
}

void libabw::ABWCollector::closeEndnote()
{
  if (m_iface)
    m_iface->closeEndnote();

  if (!m_parsingStates.empty())
  {
    delete m_ps;
    m_ps = m_parsingStates.top();
    m_parsingStates.pop();
  }
}




/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
