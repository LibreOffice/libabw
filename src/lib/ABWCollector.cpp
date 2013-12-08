/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

static bool findDouble(const std::string &str, double &res, ABWUnit &unit)
{
  using namespace ::boost::spirit::classic;

  if (str.empty())
    return false;

  unit = ABW_NONE;

  if (!parse(str.c_str(),
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

bool findInt(const std::string &str, int &res)
{
  using namespace ::boost::spirit::classic;

  if (str.empty())
    return false;

  return parse(str.c_str(),
               //  Begin grammar
               (
                 int_p[assign_a(res)]
               ) >> end_p,
               //  End grammar
               space_p).full;
}

bool findBool(const std::string &str, bool &res)
{
  using namespace ::boost::spirit::classic;

  if (str.empty())
    return false;

  return parse(str.c_str(),
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
  if (s.empty())
    return s;

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

static void parsePropString(const std::string &str, std::map<std::string, std::string> &props)
{
  if (str.empty())
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

void parseTableColumns(const std::string &str, librevenge::RVNGPropertyListVector &columns)
{
  if (str.empty())
    return;

  std::string propString(str);
  boost::trim(propString);
  std::vector<std::string> strVec;
  boost::algorithm::split(strVec, propString, boost::is_any_of("/"), boost::token_compress_on);
  for (std::vector<std::string>::size_type i = 0; i < strVec.size(); ++i)
  {
    ABWUnit unit(ABW_NONE);
    double value(0.0);
    boost::algorithm::trim(strVec[i]);
    if (findDouble(strVec[i], value, unit) || ABW_IN != unit)
    {
      librevenge::RVNGPropertyList propList;
      propList.insert("style:column-width", value);
      columns.append(propList);
    }
  }
}

bool parseTabStop(const std::string &str, librevenge::RVNGPropertyList &tabStop)
{
  if (str.empty())
    return false;
  std::vector<std::string> strVec;
  boost::algorithm::split(strVec, str, boost::is_any_of("/"), boost::token_compress_on);
  if (strVec.size() < 2)
    return false;
  boost::algorithm::trim(strVec[0]);
  ABWUnit unit(ABW_NONE);
  double value(0.0);
  if (!findDouble(strVec[0], value, unit) || ABW_IN != unit)
    return false;
  tabStop.insert("style:position", value);

  boost::algorithm::trim(strVec[1]);
  if (!strVec[1].empty())
  {
    switch (strVec[1][0])
    {
    case 'L':
      tabStop.insert("style:type", "left");
      break;
    case 'C':
      tabStop.insert("style:type", "center");
      break;
    case 'D':
      tabStop.insert("style:type", "char");
      break;
    case 'R':
      tabStop.insert("style:type", "right");
      break;
    default:
      tabStop.insert("style:type", "left");
      break;
    }
  }

  if (strVec[1].size() > 1)
  {
    switch (strVec[1][1])
    {
    case '3':
      tabStop.insert("style:leader-text", "_");
      break;
    case '2':
      tabStop.insert("style:leader-text", "-");
      break;
    case '1':
      tabStop.insert("style:leader-text", ".");
      break;
    default:
      break;
    }
  }

  return true;
}

void parseTabStops(const std::string &str, librevenge::RVNGPropertyListVector &tabStops)
{
  if (str.empty())
    return;
  std::vector<std::string> strVec;
  boost::algorithm::split(strVec, str, boost::is_any_of(","), boost::token_compress_on);
  for (std::vector<std::string>::size_type i = 0; i < strVec.size(); ++i)
  {
    boost::algorithm::trim(strVec[i]);
    librevenge::RVNGPropertyList tabStop;
    if (parseTabStop(strVec[i], tabStop))
      tabStops.append(tabStop);
  }
}

std::string decodeUrl(const std::string &str)
{
  using namespace ::boost::spirit::classic;

  if (str.empty())
    return str;

  // look for a hexadecimal number of 2 digits
  uint_parser<char,16,2,2> urlhex_p;
  std::string decoded_string;
  if (parse(str.c_str(),
            //  Begin grammar
            *(
              (ch_p('%') >>
               (
                 urlhex_p[push_back_a(decoded_string)]
                 |
                 ch_p('%')[push_back_a(decoded_string)]
               )
              )
              |
              (
                (~ch_p('%'))[push_back_a(decoded_string)]
              )
            ) >> end_p,
            //  End grammar
            space_p).full)
    return decoded_string;

  return str;
}

} // anonymous namespace

} // namespace libabw

libabw::ABWTableState::ABWTableState() :
  m_currentTableProperties(),
  m_currentCellProperties(),

  m_currentTableCol(-1),
  m_currentTableRow(-1),
  m_currentTableCellNumberInRow(-1),
  m_isTableRowOpened(false),
  m_isTableColumnOpened(false),
  m_isTableCellOpened(false),
  m_isCellWithoutParagraph(false),
  m_isRowWithoutCell(false)
{
}

libabw::ABWTableState::ABWTableState(const ABWTableState &ps) :
  m_currentTableProperties(ps.m_currentTableProperties),
  m_currentCellProperties(ps.m_currentCellProperties),

  m_currentTableCol(ps.m_currentTableCol),
  m_currentTableRow(ps.m_currentTableRow),
  m_currentTableCellNumberInRow(ps.m_currentTableCellNumberInRow),
  m_isTableRowOpened(ps.m_isTableRowOpened),
  m_isTableColumnOpened(ps.m_isTableColumnOpened),
  m_isTableCellOpened(ps.m_isTableCellOpened),
  m_isCellWithoutParagraph(ps.m_isCellWithoutParagraph),
  m_isRowWithoutCell(ps.m_isRowWithoutCell)
{
}

libabw::ABWTableState::~ABWTableState()
{
}

libabw::ABWParsingState::ABWParsingState() :
  m_isDocumentStarted(false),
  m_isPageSpanOpened(false),
  m_isSectionOpened(false),

  m_isSpanOpened(false),
  m_isParagraphOpened(false),

  m_currentSectionStyle(),
  m_currentParagraphStyle(),
  m_currentCharacterStyle(),

  m_pageWidth(0.0),
  m_pageHeight(0.0),
  m_pageMarginTop(0.0),
  m_pageMarginBottom(0.0),
  m_pageMarginLeft(0.0),
  m_pageMarginRight(0.0),

  m_deferredPageBreak(false),
  m_deferredColumnBreak(false),

  m_isNote(false),

  m_tableStates()
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

  m_pageWidth(ps.m_pageWidth),
  m_pageHeight(ps.m_pageHeight),
  m_pageMarginTop(ps.m_pageMarginTop),
  m_pageMarginBottom(ps.m_pageMarginBottom),
  m_pageMarginLeft(ps.m_pageMarginLeft),
  m_pageMarginRight(ps.m_pageMarginRight),

  m_deferredPageBreak(ps.m_deferredPageBreak),
  m_deferredColumnBreak(ps.m_deferredColumnBreak),

  m_isNote(ps.m_isNote),

  m_tableStates(ps.m_tableStates)
{
}

libabw::ABWParsingState::~ABWParsingState()
{
}

libabw::ABWCollector::ABWCollector(librevenge::RVNGTextInterface *iface) :
  m_ps(new ABWParsingState),
  m_iface(iface),
  m_parsingStates(),
  m_dontLoop(),
  m_textStyles()
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
  if (props)
    parsePropString(props, style.properties);
  if (name)
    m_textStyles[name] = style;
}

void libabw::ABWCollector::_recurseTextProperties(const char *name, std::map<std::string, std::string> &styleProps)
{
  if (name)
  {
    m_dontLoop.insert(name);
    std::map<std::string, ABWStyle>::const_iterator iter = m_textStyles.find(name);
    if (iter != m_textStyles.end() && !(iter->second.basedon.empty()) && !m_dontLoop.count(iter->second.basedon))
      _recurseTextProperties(iter->second.basedon.c_str(), styleProps);
    if (iter != m_textStyles.end())
    {
      for (std::map<std::string, std::string>::const_iterator i = iter->second.properties.begin(); i != iter->second.properties.end(); ++i)
        styleProps[i->first] = i->second;
    }
  }
  if (!m_dontLoop.empty())
    m_dontLoop.clear();
}

std::string libabw::ABWCollector::_findParagraphProperty(const char *name)
{
  std::map<std::string, std::string>::const_iterator iter = m_ps->m_currentParagraphStyle.find(name);
  if (iter != m_ps->m_currentParagraphStyle.end())
    return iter->second;
  return std::string();
}

std::string libabw::ABWCollector::_findTableProperty(const char *name)
{
  std::map<std::string, std::string>::const_iterator iter = m_ps->m_tableStates.top().m_currentTableProperties.find(name);
  if (iter != m_ps->m_tableStates.top().m_currentTableProperties.end())
    return iter->second;
  return std::string();
}

std::string libabw::ABWCollector::_findCellProperty(const char *name)
{
  std::map<std::string, std::string>::const_iterator iter = m_ps->m_tableStates.top().m_currentCellProperties.find(name);
  if (iter != m_ps->m_tableStates.top().m_currentCellProperties.end())
    return iter->second;
  return std::string();
}

std::string libabw::ABWCollector::_findCharacterProperty(const char *name)
{
  std::map<std::string, std::string>::const_iterator iter = m_ps->m_currentCharacterStyle.find(name);
  if (iter != m_ps->m_currentCharacterStyle.end())
    return iter->second;
  iter = m_ps->m_currentParagraphStyle.find(name);
  if (iter != m_ps->m_currentParagraphStyle.end())
    return iter->second;
  return std::string();
}

void libabw::ABWCollector::collectParagraphProperties(const char *style, const char *props)
{
  m_ps->m_currentParagraphStyle.clear();
  if (style)
    _recurseTextProperties(style, m_ps->m_currentParagraphStyle);

  std::map<std::string, std::string> tmpProps;
  if (props)
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
  if (props)
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
  if (props)
    parsePropString(props, tmpProps);
  ABWUnit unit(ABW_NONE);
  double value(0.0);
  for (std::map<std::string, std::string>::const_iterator iter = tmpProps.begin(); iter != tmpProps.end(); ++iter)
  {
    if (iter->first == "page-margin-right" && !iter->second.empty() && fabs(m_ps->m_pageMarginRight) < ABW_EPSILON)
    {
      if (findDouble(iter->second, value, unit))
      {
        if (unit == ABW_IN && value > 0.0 && fabs(value) > ABW_EPSILON)
          m_ps->m_pageMarginRight = value;
      }
    }
    else if (iter->first == "page-margin-left" && !iter->second.empty() && fabs(m_ps->m_pageMarginLeft) < ABW_EPSILON)
    {
      if (findDouble(iter->second, value, unit))
      {
        if (unit == ABW_IN && value > 0.0 && fabs(value) > ABW_EPSILON)
          m_ps->m_pageMarginLeft = value;
      }
    }
    else if (iter->first == "page-margin-top" && !iter->second.empty() && fabs(m_ps->m_pageMarginTop) < ABW_EPSILON)
    {
      if (findDouble(iter->second, value, unit))
      {
        if (unit == ABW_IN && value > 0.0 && fabs(value) > ABW_EPSILON)
          m_ps->m_pageMarginTop = value;
      }
    }
    else if (iter->first == "page-margin-bottom" && !iter->second.empty() && fabs(m_ps->m_pageMarginBottom) < ABW_EPSILON)
    {
      if (findDouble(iter->second, value, unit))
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
  if (findDouble(widthStr, value, unit))
  {
    if (unit == ABW_IN)
      m_ps->m_pageWidth = value;
  }
  if (findDouble(heightStr, value, unit))
  {
    if (unit == ABW_IN)
      m_ps->m_pageHeight = value;
  }
}

void libabw::ABWCollector::startDocument()
{
  if (!m_ps->m_isNote && m_ps->m_tableStates.empty())
  {

    if (m_iface && !m_ps->m_isDocumentStarted)
      m_iface->startDocument();

    m_ps->m_isDocumentStarted = true;
  }
}

void libabw::ABWCollector::endDocument()
{
  if (!m_ps->m_isNote)
  {
    if (!m_ps->m_isPageSpanOpened)
      _openSpan();

    if (m_ps->m_isParagraphOpened)
      _closeParagraph();
    if (!m_ps->m_tableStates.empty())
      _closeTable();

    // close the document nice and tight
    _closeSection();
    _closePageSpan();
    if (m_iface)
      m_iface->endDocument();
  }
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
    propList.insert("xlink:href", decodeUrl(href).c_str());
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
  if (!m_ps->m_isPageSpanOpened && !m_ps->m_isNote && m_ps->m_tableStates.empty())
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
  }
  m_ps->m_isPageSpanOpened = true;
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
  if (!m_ps->m_isSectionOpened && !m_ps->m_isNote && m_ps->m_tableStates.empty())
  {
    if (!m_ps->m_isPageSpanOpened)
      _openPageSpan();

    librevenge::RVNGPropertyList propList;

    ABWUnit unit(ABW_NONE);
    double value(0.0);
    std::map<std::string, std::string>::const_iterator iter = m_ps->m_currentSectionStyle.find("page-margin-right");
    if (iter != m_ps->m_currentSectionStyle.end())
    {
      if (findDouble(iter->second, value, unit))
      {
        if (unit == ABW_IN)
          propList.insert("fo:margin-right", value - m_ps->m_pageMarginRight);
      }
    }
    iter = m_ps->m_currentSectionStyle.find("page-margin-left");
    if (iter != m_ps->m_currentSectionStyle.end())
    {
      if (findDouble(iter->second, value, unit))
      {
        if (unit == ABW_IN)
          propList.insert("fo:margin-left", value - m_ps->m_pageMarginLeft);
      }
    }
    iter = m_ps->m_currentSectionStyle.find("section-space-after");
    if (iter != m_ps->m_currentSectionStyle.end())
    {
      if (findDouble(iter->second, value, unit))
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
      if (findInt(iter->second, intValue))
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
  }
  m_ps->m_isSectionOpened = true;
}

void libabw::ABWCollector::_openParagraph()
{
  if (!m_ps->m_isParagraphOpened)
  {
    if (!m_ps->m_tableStates.empty() && !m_ps->m_tableStates.top().m_isTableCellOpened)
      _openTableCell();

    if (!m_ps->m_isSectionOpened)
      _openSection();

    librevenge::RVNGPropertyList propList;
    ABWUnit unit(ABW_NONE);
    double value(0.0);
    int intValue(0);

    if (findDouble(_findParagraphProperty("margin-right"), value, unit))
    {
      if (unit == ABW_IN)
        propList.insert("fo:margin-right", value);
    }

    if (findDouble(_findParagraphProperty("margin-left"), value, unit))
    {
      if (unit == ABW_IN)
        propList.insert("fo:margin-left", value);
    }

    if (findDouble(_findParagraphProperty("margin-top"), value, unit))
    {
      if (unit == ABW_IN)
        propList.insert("fo:margin-top", value);
    }

    if (findDouble(_findParagraphProperty("margin-left"), value, unit))
    {
      if (unit == ABW_IN)
        propList.insert("fo:margin-left", value);
    }

    if (findDouble(_findParagraphProperty("text-indent"), value, unit))
    {
      if (unit == ABW_IN)
        propList.insert("fo:text-indent", value);
    }

    std::string sValue = _findParagraphProperty("text-align");
    if (!sValue.empty())
    {
      if (sValue == "left")
        propList.insert("fo:text-align", "start");
      else if (sValue == "right")
        propList.insert("fo:text-align", "end");
      else
        propList.insert("fo:text-align", sValue.c_str());
    }

    sValue = _findParagraphProperty("line-height");
    if (!sValue.empty())
    {
      std::string propName("fo:line-height");
      size_t position = sValue.find_last_of('+');
      if (position && position != std::string::npos)
      {
        propName = "style:line-height-at-least";
        sValue.erase(position);
      }
      if (findDouble(sValue, value, unit))
      {
        if (ABW_IN == unit)
          propList.insert(propName.c_str(), value);
        else if (ABW_PERCENT == unit)
          propList.insert(propName.c_str(), value, librevenge::RVNG_PERCENT);
      }
    }

    if (findInt(_findParagraphProperty("orphans"), intValue))
      propList.insert("fo:orphans", intValue);

    if (findInt(_findParagraphProperty("widows"), intValue))
      propList.insert("fo:widows", intValue);

    librevenge::RVNGPropertyListVector tabStops;
    parseTabStops(_findParagraphProperty("tabstops"), tabStops);

    if (tabStops.count())
      propList.insert("style:tab-stops", tabStops);

    sValue = _findParagraphProperty("dom-dir");
    if (sValue == "ltr")
      propList.insert("style:writing-mode", "lr-tb");
    else if (sValue == "rtl")
      propList.insert("style:writing-mode", "rl-tb");

    if (m_ps->m_deferredPageBreak)
      propList.insert("fo:break-before", "page");
    else if (m_ps->m_deferredColumnBreak)
      propList.insert("fo:break-before", "column");
    m_ps->m_deferredPageBreak = false;
    m_ps->m_deferredColumnBreak = false;

    if (m_iface)
      m_iface->openParagraph(propList);

    m_ps->m_isParagraphOpened = true;
    if (!m_ps->m_tableStates.empty())
      m_ps->m_tableStates.top().m_isCellWithoutParagraph = false;
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

    if (findDouble(_findCharacterProperty("font-size"), value, unit) && unit == ABW_IN)
      propList.insert("fo:font-size", value);

    std::string sValue = _findCharacterProperty("font-family");
    if (!sValue.empty())
      propList.insert("style:font-name", sValue.c_str());

    sValue = _findCharacterProperty("font-style");
    if (!sValue.empty() && sValue != "normal")
      propList.insert("fo:font-style", sValue.c_str());

    sValue = _findCharacterProperty("font-weight");
    if (!sValue.empty() && sValue != "normal")
      propList.insert("fo:font-weight", sValue.c_str());

    sValue = _findCharacterProperty("text-decoration");
    if (sValue == "underline")
      propList.insert("style:text-underline-type", "solid");
    else if (sValue == "line-through")
      propList.insert("style:text-line-through-type", "single");

    sValue = getColor(_findCharacterProperty("color"));
    if (!sValue.empty())
      propList.insert("fo:color", sValue.c_str());

    sValue = getColor(_findCharacterProperty("bgcolor"));
    if (!sValue.empty())
      propList.insert("fo:background-color", sValue.c_str());

    sValue = _findCharacterProperty("text-position");
    if (sValue == "subscript")
      propList.insert("style:text-position", "sub");
    else if (sValue == "superscript")
      propList.insert("style:text-position", "super");

    if (m_iface)
      m_iface->openSpan(propList);
  }
  m_ps->m_isSpanOpened = true;
}

void libabw::ABWCollector::_closeSection()
{
  if (m_ps->m_isSectionOpened)
  {
    if (!m_ps->m_tableStates.empty())
      _closeTable();

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

void libabw::ABWCollector::_openTable()
{
  if (m_ps->m_isParagraphOpened)
    _closeParagraph();

  if (!m_ps->m_isSectionOpened)
    _openSection();

  librevenge::RVNGPropertyList propList;
  if (m_ps->m_deferredPageBreak)
    propList.insert("fo:break-before", "page");
  else if (m_ps->m_deferredColumnBreak)
    propList.insert("fo:break-before", "column");
  m_ps->m_deferredPageBreak = false;
  m_ps->m_deferredColumnBreak = false;

  librevenge::RVNGPropertyListVector columns;
  parseTableColumns(_findTableProperty("table-column-props"), columns);
  if (columns.count())
    propList.insert("librevenge:table-columns", columns);

  ABWUnit unit(ABW_NONE);
  double value(0.0);
  if (findDouble(_findTableProperty("table-column-leftpos"), value, unit) && unit == ABW_IN)
  {
    propList.insert("fo:margin-left", value);
    propList.insert("table:align", "margins");
  }
  else
    propList.insert("table:align", "left");

  if (m_iface)
    m_iface->openTable(propList);

  m_ps->m_tableStates.top().m_currentTableRow = (-1);
  m_ps->m_tableStates.top().m_currentTableCol = (-1);
  m_ps->m_tableStates.top().m_currentTableCellNumberInRow = (-1);
}

void libabw::ABWCollector::_closeTable()
{
  if (!m_ps->m_tableStates.empty())
  {
    if (m_ps->m_tableStates.top().m_isTableRowOpened)
      _closeTableRow();

    if (m_iface)
      m_iface->closeTable();
  }

  m_ps->m_tableStates.top().m_currentTableRow = (-1);
  m_ps->m_tableStates.top().m_currentTableCol = (-1);
  m_ps->m_tableStates.top().m_currentTableCellNumberInRow = (-1);
}

void libabw::ABWCollector::_openTableRow()
{
  if (m_ps->m_tableStates.top().m_isTableRowOpened)
    _closeTableRow();

  m_ps->m_tableStates.top().m_currentTableCol = 0;
  m_ps->m_tableStates.top().m_currentTableCellNumberInRow = 0;

  if (m_iface)
    m_iface->openTableRow(librevenge::RVNGPropertyList());

  m_ps->m_tableStates.top().m_isTableRowOpened = true;
  m_ps->m_tableStates.top().m_isRowWithoutCell = true;
  m_ps->m_tableStates.top().m_currentTableRow++;
}

void libabw::ABWCollector::_closeTableRow()
{
  if (m_ps->m_tableStates.top().m_isTableRowOpened)
  {
    if (m_ps->m_tableStates.top().m_isTableCellOpened)
      _closeTableCell();

    if (m_ps->m_tableStates.top().m_isRowWithoutCell)
    {
      m_ps->m_tableStates.top().m_isRowWithoutCell = false;
      if (m_iface)
        m_iface->insertCoveredTableCell(librevenge::RVNGPropertyList());
    }
    if (m_iface)
      m_iface->closeTableRow();
  }
  m_ps->m_tableStates.top().m_isTableRowOpened = false;
}

void libabw::ABWCollector::_openTableCell()
{
  if (m_ps->m_tableStates.top().m_isTableCellOpened)
    _closeTableCell();

  librevenge::RVNGPropertyList propList;
  propList.insert("librevenge:column", m_ps->m_tableStates.top().m_currentTableCol);
  propList.insert("librevenge:row", m_ps->m_tableStates.top().m_currentTableRow);

  int rightAttach(0);
  if (findInt(_findCellProperty("right-attach"), rightAttach))
    propList.insert("table:number-columns-spanned", rightAttach - m_ps->m_tableStates.top().m_currentTableCol);

  int botAttach(0);
  if (findInt(_findCellProperty("bot-attach"), botAttach))
    propList.insert("table:number-rows-spanned", botAttach - m_ps->m_tableStates.top().m_currentTableRow);

  std::string bgColor = getColor(_findCellProperty("background-color"));
  if (!bgColor.empty())
    propList.insert("fo:background-color", bgColor.c_str());

  if (m_iface)
    m_iface->openTableCell(propList);

  m_ps->m_tableStates.top().m_currentTableCellNumberInRow++;
  m_ps->m_tableStates.top().m_isTableCellOpened = true;
  m_ps->m_tableStates.top().m_isCellWithoutParagraph = true;
  m_ps->m_tableStates.top().m_isRowWithoutCell = false;
}

void libabw::ABWCollector::_closeTableCell()
{
  if (m_ps->m_tableStates.top().m_isTableCellOpened)
  {
    if (m_ps->m_tableStates.top().m_isCellWithoutParagraph)
      _openSpan();
    if (m_ps->m_isParagraphOpened)
      _closeParagraph();

    if (m_iface)
      m_iface->closeTableCell();
  }
  m_ps->m_tableStates.top().m_isTableCellOpened = false;
}

void libabw::ABWCollector::openFoot(const char *id)
{
  if (!m_ps->m_isParagraphOpened)
    _openSpan();
  _closeSpan();

  librevenge::RVNGPropertyList propList;
  if (id)
    propList.insert("librevenge:number", id);
  if (m_iface)
    m_iface->openFootnote(propList);

  m_parsingStates.push(m_ps);
  m_ps = new ABWParsingState();

  m_ps->m_isNote = true;
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

  librevenge::RVNGPropertyList propList;
  if (id)
    propList.insert("librevenge:number", id);
  if (m_iface)
    m_iface->openEndnote(propList);

  m_parsingStates.push(m_ps);
  m_ps = new ABWParsingState();

  m_ps->m_isNote = true;
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

void libabw::ABWCollector::openTable(const char *props)
{
  if (!m_ps->m_isSectionOpened && m_ps->m_tableStates.empty())
    _openSection();

  m_ps->m_tableStates.push(ABWTableState());
  if (props)
    parsePropString(props, m_ps->m_tableStates.top().m_currentTableProperties);

  _openTable();
}

void libabw::ABWCollector::closeTable()
{
  _closeTable();

  if (!m_ps->m_tableStates.empty())
    m_ps->m_tableStates.pop();
}

void libabw::ABWCollector::openCell(const char *props)
{
  if (props)
    parsePropString(props, m_ps->m_tableStates.top().m_currentCellProperties);
  int currentRow(0);
  if (!findInt(_findCellProperty("top-attach"), currentRow))
    currentRow = m_ps->m_tableStates.top().m_currentTableRow + 1;
  while (m_ps->m_tableStates.top().m_currentTableRow < currentRow)
  {
    if (m_ps->m_tableStates.top().m_currentTableRow >= 0)
      _closeTableRow();
    _openTableRow();
  }

  if (!findInt(_findCellProperty("left-attach"), m_ps->m_tableStates.top().m_currentTableCol))
    m_ps->m_tableStates.top().m_currentTableCol++;
}

void libabw::ABWCollector::closeCell()
{
  _closeTableCell();
  m_ps->m_tableStates.top().m_currentCellProperties.clear();
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
