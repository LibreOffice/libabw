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

bool findDouble(const char *str, double &res, ABWUnit &unit)
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

void parsePropString(const char *str, std::map<std::string, std::string> &props)
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

} // anonymous namespace

} // namespace libabw

libabw::ABWParsingState::ABWParsingState() :
  m_isDocumentStarted(false),
  m_isPageSpanOpened(false),
  m_isSectionOpened(false),

  m_isSpanOpened(false),
  m_isParagraphOpened(false),

  m_currentParagraphStyle(),
  m_currentCharacterStyle(),

  m_paragraphStyles(),
  m_characterStyles(),

  m_pageWidth(0.0),
  m_pageHeight(0.0)
{
}

libabw::ABWParsingState::~ABWParsingState()
{
}

libabw::ABWCollector::ABWCollector(librevenge::RVNGTextInterface *iface) :
  m_ps(new ABWParsingState),
  m_iface(iface)
{
}

libabw::ABWCollector::~ABWCollector()
{
  DELETEP(m_ps);
}

void libabw::ABWCollector::collectParagraphStyle(const char *name, const char *basedon, const char *followedby, const char *props)
{
  ABWStyle style;
  style.basedon = basedon ? basedon : "";
  style.followedby = followedby ? followedby : "";
  parsePropString(props, style.properties);
  m_ps->m_paragraphStyles[name] = style;
}

void libabw::ABWCollector::collectCharacterStyle(const char *name, const char *basedon, const char *followedby, const char *props)
{
  ABWStyle style;
  style.basedon = basedon ? basedon : "";
  style.followedby = followedby ? followedby : "";
  parsePropString(props, style.properties);
  m_ps->m_characterStyles[name] = style;
}

void libabw::ABWCollector::collectParagraphProperties(const char *style, const char *props)
{
  m_ps->m_currentParagraphStyle.clear();
  if (style)
  {
    std::map<std::string, ABWStyle>::const_iterator iter = m_ps->m_paragraphStyles.find(style);
    if (iter != m_ps->m_paragraphStyles.end())
      m_ps->m_currentParagraphStyle = iter->second.properties;
  }
  std::map<std::string, std::string> pstring;
  parsePropString(props, pstring);
  for (std::map<std::string, std::string>::const_iterator iter = pstring.begin(); iter != pstring.end(); ++iter)
    m_ps->m_currentParagraphStyle[iter->first] = iter->second;
}

void libabw::ABWCollector::collectCharacterProperties(const char *style, const char *props)
{
  m_ps->m_currentCharacterStyle.clear();
  if (style)
  {
    std::map<std::string, ABWStyle>::const_iterator iter = m_ps->m_characterStyles.find(style);
    if (iter != m_ps->m_characterStyles.end())
      m_ps->m_currentCharacterStyle = iter->second.properties;
  }
  std::map<std::string, std::string> pstring;
  parsePropString(props, pstring);
  for (std::map<std::string, std::string>::const_iterator iter = pstring.begin(); iter != pstring.end(); ++iter)
    m_ps->m_currentCharacterStyle[iter->first] = iter->second;
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
  _closeParagraph();
}

void libabw::ABWCollector::closeSpan()
{
  _closeSpan();
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
  if (!m_ps->m_isSpanOpened)
    _openSpan();
}

void libabw::ABWCollector::insertPageBreak()
{
  if (!m_ps->m_isSpanOpened)
    _openSpan();
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
  if (m_ps->m_isPageSpanOpened)
    return;

  if (!m_ps->m_isDocumentStarted)
    startDocument();

  librevenge::RVNGPropertyList propList;
  propList.insert("fo:page-width", m_ps->m_pageWidth);
  propList.insert("fo:page-height", m_ps->m_pageHeight);

  if (m_iface && !m_ps->m_isPageSpanOpened)
    m_iface->openPageSpan(propList);

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
  if (!m_ps->m_isSectionOpened)
  {
    if (!m_ps->m_isPageSpanOpened)
      _openPageSpan();

    librevenge::RVNGPropertyList propList;

    librevenge::RVNGPropertyListVector columns;
    if (columns.count())
      propList.insert("style:columns", columns);
    if (!m_ps->m_isSectionOpened)
      m_iface->openSection(propList);

    m_ps->m_isSectionOpened = true;
  }
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

void libabw::ABWCollector::_openParagraph()
{
  if (!m_ps->m_isParagraphOpened)
  {
    if (!m_ps->m_isSectionOpened)
      _openSection();

    librevenge::RVNGPropertyListVector tabStops;

    librevenge::RVNGPropertyList propList;

    if (tabStops.count())
      propList.insert("style:tab-stops", tabStops);

    if (m_iface)
      m_iface->openParagraph(propList);

    m_ps->m_isParagraphOpened = true;
  }
}


void libabw::ABWCollector::_closeParagraph()
{
  if (m_ps->m_isParagraphOpened)
  {
    if (m_ps->m_isSpanOpened)
      _closeSpan();

    m_iface->closeParagraph();
  }

  m_ps->m_isParagraphOpened = false;
}


void libabw::ABWCollector::_openSpan()
{
  if (!m_ps->m_isParagraphOpened)
    _openParagraph();

  librevenge::RVNGPropertyList propList;
  if (!m_ps->m_isSpanOpened)
    m_iface->openSpan(propList);

  m_ps->m_isSpanOpened = true;
}

void libabw::ABWCollector::_closeSpan()
{
  if (m_ps->m_isSpanOpened)
    m_iface->closeSpan();

  m_ps->m_isSpanOpened = false;
}


/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
