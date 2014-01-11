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
#include "ABWContentCollector.h"
#include "libabw_internal.h"

#define ABW_EPSILON 1.0E-06

namespace libabw
{

namespace
{

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

static void separateTabsAndInsertText(ABWOutputElements &outputElements, const librevenge::RVNGString &text)
{
  if (text.empty())
    return;
  librevenge::RVNGString tmpText;
  librevenge::RVNGString::Iter i(text);
  for (i.rewind(); i.next();)
  {
    if (*(i()) == '\t')
    {
      if (!tmpText.empty())
      {
        outputElements.addInsertText(tmpText);
        tmpText.clear();
      }
      outputElements.addInsertTab();
    }
    else if (*(i()) == '\n')
    {
      if (!tmpText.empty())
      {
        outputElements.addInsertText(tmpText);
        tmpText.clear();
      }
      outputElements.addInsertLineBreak();
    }
    else
    {
      tmpText.append(i());
    }
  }
  if (!tmpText.empty())
    outputElements.addInsertText(tmpText);
}

static void separateSpacesAndInsertText(ABWOutputElements &outputElements, const librevenge::RVNGString &text)
{
  if (text.empty())
  {
    outputElements.addInsertText(text);
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
        separateTabsAndInsertText(outputElements, tmpText);
        tmpText.clear();
      }
      outputElements.addInsertSpace();
    }
    else
      tmpText.append(i());
  }
  separateTabsAndInsertText(outputElements, tmpText);
}

void parseTableColumns(const std::string &str, librevenge::RVNGPropertyListVector &columns)
{
  if (str.empty())
    return;

  std::string propString(boost::trim_copy_if(str, boost::is_any_of("/ ")));
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
  std::string sTabStop(boost::trim_copy_if(str, boost::is_any_of("/ ")));
  std::vector<std::string> strVec;
  boost::algorithm::split(strVec, sTabStop, boost::is_any_of("/"), boost::token_compress_on);
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
  std::string sTabStops(boost::trim_copy_if(str, boost::is_any_of(", ")));
  std::vector<std::string> strVec;
  boost::algorithm::split(strVec, sTabStops, boost::is_any_of(","), boost::token_compress_on);
  for (std::vector<std::string>::size_type i = 0; i < strVec.size(); ++i)
  {
    boost::trim(strVec[i]);
    librevenge::RVNGPropertyList tabStop;
    if (parseTabStop(strVec[i], tabStop))
      tabStops.append(tabStop);
  }
}

static std::string decodeUrl(const std::string &str)
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

libabw::ABWContentTableState::ABWContentTableState() :
  m_currentTableProperties(),
  m_currentCellProperties(),

  m_currentTableCol(-1),
  m_currentTableRow(-1),
  m_currentTableCellNumberInRow(-1),
  m_currentTableId(-1),
  m_isTableRowOpened(false),
  m_isTableColumnOpened(false),
  m_isTableCellOpened(false),
  m_isCellWithoutParagraph(false),
  m_isRowWithoutCell(false)
{
}

libabw::ABWContentTableState::ABWContentTableState(const ABWContentTableState &ts) :
  m_currentTableProperties(ts.m_currentTableProperties),
  m_currentCellProperties(ts.m_currentCellProperties),

  m_currentTableCol(ts.m_currentTableCol),
  m_currentTableRow(ts.m_currentTableRow),
  m_currentTableCellNumberInRow(ts.m_currentTableCellNumberInRow),
  m_currentTableId(ts.m_currentTableId),
  m_isTableRowOpened(ts.m_isTableRowOpened),
  m_isTableColumnOpened(ts.m_isTableColumnOpened),
  m_isTableCellOpened(ts.m_isTableCellOpened),
  m_isCellWithoutParagraph(ts.m_isCellWithoutParagraph),
  m_isRowWithoutCell(ts.m_isRowWithoutCell)
{
}

libabw::ABWContentTableState::~ABWContentTableState()
{
}

libabw::ABWContentParsingState::ABWContentParsingState() :
  m_isDocumentStarted(false),
  m_isPageSpanOpened(false),
  m_isSectionOpened(false),
  m_isHeaderOpened(false),
  m_isFooterOpened(false),

  m_isSpanOpened(false),
  m_isParagraphOpened(false),
  m_isListElementOpened(false),

  m_currentSectionStyle(),
  m_currentParagraphStyle(),
  m_currentCharacterStyle(),

  m_pageWidth(0.0),
  m_pageHeight(0.0),
  m_pageMarginTop(0.0),
  m_pageMarginBottom(0.0),
  m_pageMarginLeft(0.0),
  m_pageMarginRight(0.0),
  m_footerId(-1),
  m_footerLeftId(-1),
  m_footerFirstId(-1),
  m_footerLastId(-1),
  m_headerId(-1),
  m_headerLeftId(-1),
  m_headerFirstId(-1),
  m_headerLastId(-1),
  m_currentHeaderFooterId(-1),
  m_currentHeaderFooterOccurrence(),
  m_parsingContext(ABW_SECTION),

  m_deferredPageBreak(false),
  m_deferredColumnBreak(false),

  m_isNote(false),
  m_currentListLevel(0),
  m_currentListId(),

  m_tableStates(),
  m_listLevels()
{
}

libabw::ABWContentParsingState::ABWContentParsingState(const ABWContentParsingState &ps) :
  m_isDocumentStarted(ps.m_isDocumentStarted),
  m_isPageSpanOpened(ps.m_isPageSpanOpened),
  m_isSectionOpened(ps.m_isSectionOpened),
  m_isHeaderOpened(ps.m_isHeaderOpened),
  m_isFooterOpened(ps.m_isFooterOpened),

  m_isSpanOpened(ps.m_isSpanOpened),
  m_isParagraphOpened(ps.m_isParagraphOpened),
  m_isListElementOpened(ps.m_isListElementOpened),

  m_currentSectionStyle(ps.m_currentSectionStyle),
  m_currentParagraphStyle(ps.m_currentParagraphStyle),
  m_currentCharacterStyle(ps.m_currentCharacterStyle),

  m_pageWidth(ps.m_pageWidth),
  m_pageHeight(ps.m_pageHeight),
  m_pageMarginTop(ps.m_pageMarginTop),
  m_pageMarginBottom(ps.m_pageMarginBottom),
  m_pageMarginLeft(ps.m_pageMarginLeft),
  m_pageMarginRight(ps.m_pageMarginRight),
  m_footerId(ps.m_footerId),
  m_footerLeftId(ps.m_footerLeftId),
  m_footerFirstId(ps.m_footerFirstId),
  m_footerLastId(ps.m_footerLastId),
  m_headerId(ps.m_headerId),
  m_headerLeftId(ps.m_headerLeftId),
  m_headerFirstId(ps.m_headerFirstId),
  m_headerLastId(ps.m_headerLastId),
  m_currentHeaderFooterId(ps.m_currentHeaderFooterId),
  m_currentHeaderFooterOccurrence(ps.m_currentHeaderFooterOccurrence),
  m_parsingContext(ps.m_parsingContext),

  m_deferredPageBreak(ps.m_deferredPageBreak),
  m_deferredColumnBreak(ps.m_deferredColumnBreak),

  m_isNote(ps.m_isNote),
  m_currentListLevel(ps.m_currentListLevel),
  m_currentListId(ps.m_currentListId),

  m_tableStates(ps.m_tableStates),
  m_listLevels(ps.m_listLevels)
{
}

libabw::ABWContentParsingState::~ABWContentParsingState()
{
}

libabw::ABWContentCollector::ABWContentCollector(librevenge::RVNGTextInterface *iface, const std::map<int, int> &tableSizes,
                                                 const std::map<std::string, ABWData> &data,
                                                 const std::map<librevenge::RVNGString, ABWListElement *> &listElements) :
  m_ps(new ABWContentParsingState),
  m_iface(iface),
  m_parsingStates(),
  m_dontLoop(),
  m_textStyles(),
  m_data(data),
  m_tableSizes(tableSizes),
  m_tableCounter(0),
  m_outputElements(),
  m_listElements(listElements)
{
}

libabw::ABWContentCollector::~ABWContentCollector()
{
  DELETEP(m_ps);
}

void libabw::ABWContentCollector::collectTextStyle(const char *name, const char *basedon, const char *followedby, const char *props)
{
  ABWStyle style;
  style.basedon = basedon ? basedon : std::string();
  style.followedby = followedby ? followedby : std::string();
  if (props)
    parsePropString(props, style.properties);
  if (name)
    m_textStyles[name] = style;
}

void libabw::ABWContentCollector::_recurseTextProperties(const char *name, std::map<std::string, std::string> &styleProps)
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

std::string libabw::ABWContentCollector::_findParagraphProperty(const char *name)
{
  if (!name)
    return std::string();
  std::map<std::string, std::string>::const_iterator iter = m_ps->m_currentParagraphStyle.find(name);
  if (iter != m_ps->m_currentParagraphStyle.end())
    return iter->second;
  return std::string();
}

std::string libabw::ABWContentCollector::_findTableProperty(const char *name)
{
  if (!name)
    return std::string();
  std::map<std::string, std::string>::const_iterator iter = m_ps->m_tableStates.top().m_currentTableProperties.find(name);
  if (iter != m_ps->m_tableStates.top().m_currentTableProperties.end())
    return iter->second;
  return std::string();
}

std::string libabw::ABWContentCollector::_findCellProperty(const char *name)
{
  if (!name)
    return std::string();
  std::map<std::string, std::string>::const_iterator iter = m_ps->m_tableStates.top().m_currentCellProperties.find(name);
  if (iter != m_ps->m_tableStates.top().m_currentCellProperties.end())
    return iter->second;
  return std::string();
}

std::string libabw::ABWContentCollector::_findSectionProperty(const char *name)
{
  if (!name)
    return std::string();
  std::map<std::string, std::string>::const_iterator iter = m_ps->m_currentSectionStyle.find(name);
  if (iter != m_ps->m_currentSectionStyle.end())
    return iter->second;
  return std::string();
}

std::string libabw::ABWContentCollector::_findCharacterProperty(const char *name)
{
  if (!name)
    return std::string();
  std::map<std::string, std::string>::const_iterator iter = m_ps->m_currentCharacterStyle.find(name);
  if (iter != m_ps->m_currentCharacterStyle.end())
    return iter->second;
  iter = m_ps->m_currentParagraphStyle.find(name);
  if (iter != m_ps->m_currentParagraphStyle.end())
    return iter->second;
  return std::string();
}

void libabw::ABWContentCollector::collectParagraphProperties(const char *level, const char *listid, const char * /*parentid*/, const char *style, const char *props)
{
  _closeParagraph();
  _closeListElement();
  if (!level || !findInt(level, m_ps->m_currentListLevel) || m_ps->m_currentListLevel < 1)
    m_ps->m_currentListLevel = 0;
  if (listid)
    m_ps->m_currentListId = listid;
  else
    m_ps->m_currentListId.clear();

  m_ps->m_currentParagraphStyle.clear();
  if (style)
    _recurseTextProperties(style, m_ps->m_currentParagraphStyle);
  else
    _recurseTextProperties("Normal", m_ps->m_currentParagraphStyle);

  std::map<std::string, std::string> tmpProps;
  if (props)
    parsePropString(props, tmpProps);
  for (std::map<std::string, std::string>::const_iterator iter = tmpProps.begin(); iter != tmpProps.end(); ++iter)
    m_ps->m_currentParagraphStyle[iter->first] = iter->second;
}

void libabw::ABWContentCollector::collectCharacterProperties(const char *style, const char *props)
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

void libabw::ABWContentCollector::collectSectionProperties(const char *footer, const char *footerLeft, const char *footerFirst, const char *footerLast,
                                                           const char *header, const char *headerLeft, const char *headerFirst, const char *headerLast,
                                                           const char *props)
{
  _closeHeader();
  _closeFooter();
  _closeSection();
  double pageMarginLeft = m_ps->m_pageMarginLeft;
  double pageMarginRight = m_ps->m_pageMarginRight;
  double pageMarginTop = m_ps->m_pageMarginTop;
  double pageMarginBottom = m_ps->m_pageMarginBottom;
  int headerId = m_ps->m_headerId;
  int headerLeftId = m_ps->m_headerLeftId;
  int headerFirstId = m_ps->m_headerFirstId;
  int headerLastId = m_ps->m_headerLastId;
  int footerId = m_ps->m_footerId;
  int footerLeftId = m_ps->m_footerLeftId;
  int footerFirstId = m_ps->m_footerFirstId;
  int footerLastId = m_ps->m_footerLastId;

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
      if (findDouble(iter->second, value, unit) && unit == ABW_IN && value > 0.0 && fabs(value) > ABW_EPSILON)
        m_ps->m_pageMarginRight = value;
    }
    else if (iter->first == "page-margin-left" && !iter->second.empty() && fabs(m_ps->m_pageMarginLeft) < ABW_EPSILON)
    {
      if (findDouble(iter->second, value, unit) && unit == ABW_IN && value > 0.0 && fabs(value) > ABW_EPSILON)
        m_ps->m_pageMarginLeft = value;
    }
    else if (iter->first == "page-margin-top" && !iter->second.empty() && fabs(m_ps->m_pageMarginTop) < ABW_EPSILON)
    {
      if (findDouble(iter->second, value, unit) && unit == ABW_IN && value > 0.0 && fabs(value) > ABW_EPSILON)
        m_ps->m_pageMarginTop = value;
    }
    else if (iter->first == "page-margin-bottom" && !iter->second.empty() && fabs(m_ps->m_pageMarginBottom) < ABW_EPSILON)
    {
      if (findDouble(iter->second, value, unit) && unit == ABW_IN && value > 0.0 && fabs(value) > ABW_EPSILON)
        m_ps->m_pageMarginBottom = value;
    }
    m_ps->m_currentSectionStyle[iter->first] = iter->second;
  }

  int intValue(0);
  if (footer && findInt(footer, intValue) && intValue >= 0)
    m_ps->m_footerId = intValue;
  else
    m_ps->m_footerId = -1;

  if (footerLeft && findInt(footerLeft, intValue) && intValue >= 0)
    m_ps->m_footerLeftId = intValue;
  else
    m_ps->m_footerLeftId = -1;

  if (footerFirst && findInt(footerFirst, intValue) && intValue >= 0)
    m_ps->m_footerFirstId = intValue;
  else
    m_ps->m_footerFirstId = -1;

  if (footerLast && findInt(footerLast, intValue) && intValue >= 0)
    m_ps->m_footerLastId = intValue;
  else
    m_ps->m_footerLastId = -1;

  if (header && findInt(header, intValue) && intValue >= 0)
    m_ps->m_headerId = intValue;
  else
    m_ps->m_headerId = -1;

  if (headerLeft && findInt(headerLeft, intValue) && intValue >= 0)
    m_ps->m_headerLeftId = intValue;
  else
    m_ps->m_headerLeftId = -1;

  if (headerFirst && findInt(headerFirst, intValue) && intValue >= 0)
    m_ps->m_headerFirstId = intValue;
  else
    m_ps->m_headerFirstId = -1;

  if (headerLast && findInt(headerLast, intValue) && intValue >= 0)
    m_ps->m_headerLastId = intValue;
  else
    m_ps->m_headerLastId = -1;

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
      fabs(pageMarginBottom-m_ps->m_pageMarginBottom) > ABW_EPSILON ||
      footerId != m_ps->m_footerId || footerLeftId != m_ps->m_footerLeftId ||
      footerFirstId != m_ps->m_footerFirstId || footerLastId != m_ps->m_footerLastId ||
      headerId != m_ps->m_headerId || headerLeftId != m_ps->m_headerLeftId ||
      headerFirstId != m_ps->m_headerFirstId || headerLastId != m_ps->m_headerLastId)
  {
    _closePageSpan();
  }
}

void libabw::ABWContentCollector::collectHeaderFooter(const char *id, const char *type)
{
  if (!id || !findInt(id, m_ps->m_currentHeaderFooterId))
    m_ps->m_currentHeaderFooterId = -1;

  if (!type)
    m_ps->m_currentHeaderFooterId = -1;

  std::string sType(type);
  boost::trim(sType);
  std::vector<std::string> strVec;
  boost::algorithm::split(strVec, sType, boost::is_any_of("-"), boost::token_compress_on);
  if (strVec.size() >= 2)
    m_ps->m_currentHeaderFooterOccurrence = strVec[1].c_str();
  else
    m_ps->m_currentHeaderFooterOccurrence = "all";
  if (!strVec.empty())
  {
    if (strVec[0] == "header")
      m_ps->m_parsingContext = ABW_HEADER;
    else if (strVec[0] == "footer")
      m_ps->m_parsingContext = ABW_FOOTER;
    else
      m_ps->m_parsingContext = ABW_SECTION;
  }
  else
    m_ps->m_parsingContext = ABW_SECTION;
}

void libabw::ABWContentCollector::collectPageSize(const char *width, const char *height, const char *units, const char * /* pageScale */)
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
  if (findDouble(widthStr, value, unit) && unit == ABW_IN)
    m_ps->m_pageWidth = value;
  if (findDouble(heightStr, value, unit) && unit == ABW_IN)
    m_ps->m_pageHeight = value;
}

void libabw::ABWContentCollector::startDocument()
{
  if (!m_ps->m_isNote && m_ps->m_tableStates.empty())
  {

    if (m_iface && !m_ps->m_isDocumentStarted)
      m_iface->startDocument(librevenge::RVNGPropertyList());

    m_ps->m_isDocumentStarted = true;
  }
}

void libabw::ABWContentCollector::endDocument()
{
  if (!m_ps->m_isNote)
  {
    if (!m_ps->m_isPageSpanOpened)
      _openSpan();

    _closeParagraph();
    _closeListElement();

    m_ps->m_currentListLevel = 0;
    _changeList(); // flush the list

    // close the document nice and tight
    _closeSection();
    _closeHeader();
    _closeFooter();

    _closePageSpan();

    if (m_iface)
    {
      m_outputElements.write(m_iface);
      m_iface->endDocument();
    }
  }
}

void libabw::ABWContentCollector::endSection()
{
  m_ps->m_currentListLevel = 0;
  _changeList(); // flush the list exterior
  _closeHeader();
  _closeFooter();
  _closeSection();
}

void libabw::ABWContentCollector::closeParagraphOrListElement()
{
  // we have an empty paragraph, insert it
  if (!m_ps->m_isParagraphOpened && !m_ps->m_isListElementOpened)
    _openSpan();
  _closeParagraph();
  _closeListElement();
  m_ps->m_currentParagraphStyle.clear();
}

void libabw::ABWContentCollector::openLink(const char *href)
{
  if (m_ps->m_isSpanOpened)
    _closeSpan();
  if (!m_ps->m_isParagraphOpened && !m_ps->m_isListElementOpened)
  {
    _changeList();
    if (m_ps->m_currentListLevel == 0)
      _openParagraph();
    else
      _openListElement();
  }
  librevenge::RVNGPropertyList propList;
  if (href)
    propList.insert("xlink:href", decodeUrl(href).c_str());
  m_outputElements.addOpenLink(propList);
  if (!m_ps->m_isSpanOpened)
    _openSpan();
}

void libabw::ABWContentCollector::closeLink()
{
  if (m_ps->m_isSpanOpened)
    _closeSpan();
  m_outputElements.addCloseLink();
}

void libabw::ABWContentCollector::closeSpan()
{
  _closeSpan();
  m_ps->m_currentCharacterStyle.clear();
}

void libabw::ABWContentCollector::insertLineBreak()
{
  if (!m_ps->m_isSpanOpened)
    _openSpan();

  m_outputElements.addInsertLineBreak();
}

void libabw::ABWContentCollector::insertColumnBreak()
{
  _closeParagraph();
  _closeListElement();
  m_ps->m_deferredColumnBreak = true;
}

void libabw::ABWContentCollector::insertPageBreak()
{
  _closeParagraph();
  _closeListElement();
  m_ps->m_deferredPageBreak = true;
}

void libabw::ABWContentCollector::insertText(const librevenge::RVNGString &text)
{
  if (!m_ps->m_isSpanOpened)
    _openSpan();

  separateSpacesAndInsertText(m_outputElements, text);
}

void libabw::ABWContentCollector::_openPageSpan()
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

    if (!m_ps->m_isPageSpanOpened)
      m_outputElements.addOpenPageSpan(propList,
                                       m_ps->m_footerId, m_ps->m_footerLeftId,
                                       m_ps->m_footerFirstId, m_ps->m_footerLastId,
                                       m_ps->m_headerId, m_ps->m_headerLeftId,
                                       m_ps->m_headerFirstId, m_ps->m_headerLastId);
  }
  m_ps->m_isPageSpanOpened = true;
}

void libabw::ABWContentCollector::_closePageSpan()
{
  if (m_ps->m_isPageSpanOpened)
  {
    _closeHeader();
    _closeFooter();
    _closeSection();

    m_outputElements.addClosePageSpan();
  }
  m_ps->m_isPageSpanOpened = false;
}

void libabw::ABWContentCollector::_openSection()
{
  if (!m_ps->m_isSectionOpened && !m_ps->m_isNote && m_ps->m_tableStates.empty())
  {
    if (!m_ps->m_isPageSpanOpened)
      _openPageSpan();

    librevenge::RVNGPropertyList propList;

    ABWUnit unit(ABW_NONE);
    double value(0.0);
    if (findDouble(_findSectionProperty("page-margin-right"), value, unit) && unit == ABW_IN)
      propList.insert("fo:margin-right", value - m_ps->m_pageMarginRight);

    if (findDouble(_findSectionProperty("page-margin-left"), value, unit) && unit == ABW_IN)
      propList.insert("fo:margin-left", value - m_ps->m_pageMarginLeft);

    if (findDouble(_findSectionProperty("section-space-after"), value, unit) && unit == ABW_IN)
      propList.insert("librevenge:margin-bottom", value);

    std::string sValue = _findSectionProperty("dom-dir");
    if (sValue == "ltr")
      propList.insert("style:writing-mode", "lr-tb");
    else if (sValue == "rtl")
      propList.insert("style:writing-mode", "rl-tb");

    int intValue(0);
    if (findInt(_findSectionProperty("columns"), intValue) && intValue > 1)
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
    m_outputElements.addOpenSection(propList);
  }
  m_ps->m_isSectionOpened = true;
}

void libabw::ABWContentCollector::_openFooter()
{
  if (!m_ps->m_isFooterOpened && !m_ps->m_isNote && m_ps->m_tableStates.empty())
  {
    librevenge::RVNGPropertyList propList;
    propList.insert("librevenge:occurrence", m_ps->m_currentHeaderFooterOccurrence);

    m_outputElements.addOpenFooter(propList, m_ps->m_currentHeaderFooterId);
  }
  m_ps->m_isFooterOpened = true;
}

void libabw::ABWContentCollector::_openHeader()
{
  if (!m_ps->m_isHeaderOpened && !m_ps->m_isNote && m_ps->m_tableStates.empty())
  {
    librevenge::RVNGPropertyList propList;
    propList.insert("librevenge:occurrence", m_ps->m_currentHeaderFooterOccurrence);

    m_outputElements.addOpenHeader(propList, m_ps->m_currentHeaderFooterId);
  }
  m_ps->m_isHeaderOpened = true;
}

void libabw::ABWContentCollector::_fillParagraphProperties(librevenge::RVNGPropertyList &propList)
{
  ABWUnit unit(ABW_NONE);
  double value(0.0);
  int intValue(0);

  if (findDouble(_findParagraphProperty("margin-right"), value, unit) && unit == ABW_IN)
    propList.insert("fo:margin-right", value);

  if (findDouble(_findParagraphProperty("margin-left"), value, unit) && unit == ABW_IN)
    propList.insert("fo:margin-left", value);

  if (findDouble(_findParagraphProperty("margin-top"), value, unit) && unit == ABW_IN)
    propList.insert("fo:margin-top", value);

  if (findDouble(_findParagraphProperty("margin-left"), value, unit) && unit == ABW_IN)
    propList.insert("fo:margin-left", value);

  if (findDouble(_findParagraphProperty("text-indent"), value, unit) && unit == ABW_IN)
    propList.insert("fo:text-indent", value);

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
}

void libabw::ABWContentCollector::_openParagraph()
{
  if (!m_ps->m_isParagraphOpened)
  {
    switch (m_ps->m_parsingContext)
    {
    case ABW_HEADER:
      if (!m_ps->m_isHeaderOpened)
        _openHeader();
      break;
    case ABW_FOOTER:
      if (!m_ps->m_isFooterOpened)
        _openFooter();
      break;
    case ABW_SECTION:
    default:
      if (!m_ps->m_isSectionOpened)
        _openSection();
      break;
    }

    if (!m_ps->m_tableStates.empty() && !m_ps->m_tableStates.top().m_isTableCellOpened)
      _openTableCell();

    _changeList();

    librevenge::RVNGPropertyList propList;
    _fillParagraphProperties(propList);

    m_ps->m_deferredPageBreak = false;
    m_ps->m_deferredColumnBreak = false;

    m_outputElements.addOpenParagraph(propList);

    m_ps->m_isParagraphOpened = true;
    if (!m_ps->m_tableStates.empty())
      m_ps->m_tableStates.top().m_isCellWithoutParagraph = false;
  }
}

void libabw::ABWContentCollector::_openListElement()
{
  if (!m_ps->m_isListElementOpened)
  {
    switch (m_ps->m_parsingContext)
    {
    case ABW_HEADER:
      if (!m_ps->m_isHeaderOpened)
        _openHeader();
      break;
    case ABW_FOOTER:
      if (!m_ps->m_isFooterOpened)
        _openFooter();
      break;
    case ABW_SECTION:
    default:
      if (!m_ps->m_isSectionOpened)
        _openSection();
      break;
    }

    if (!m_ps->m_tableStates.empty() && !m_ps->m_tableStates.top().m_isTableCellOpened)
      _openTableCell();

    _changeList();

    librevenge::RVNGPropertyList propList;
    _fillParagraphProperties(propList);

    m_outputElements.addOpenListElement(propList);

    m_ps->m_isListElementOpened = true;
    if (!m_ps->m_tableStates.empty())
      m_ps->m_tableStates.top().m_isCellWithoutParagraph = false;
  }
}

void libabw::ABWContentCollector::_openSpan()
{
  if (!m_ps->m_isSpanOpened)
  {
    if (!m_ps->m_isParagraphOpened && !m_ps->m_isListElementOpened)
    {
      if (m_ps->m_currentListLevel == 0)
        _openParagraph();
      else
        _openListElement();
    }

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
    {
      propList.insert("style:text-underline-type", "single");
      propList.insert("style:text-underline-style", "solid");
    }
    else if (sValue == "line-through")
    {
      propList.insert("style:text-line-through-type", "single");
      propList.insert("style:text-line-through-style", "solid");
    }

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

    m_outputElements.addOpenSpan(propList);
  }
  m_ps->m_isSpanOpened = true;
}

void libabw::ABWContentCollector::_closeSection()
{
  if (m_ps->m_isSectionOpened)
  {
    while (!m_ps->m_tableStates.empty())
      _closeTable();

    _closeParagraph();
    _closeListElement();
    m_ps->m_currentListLevel = 0;
    _changeList();

    m_outputElements.addCloseSection();

    m_ps->m_isSectionOpened = false;
  }
}

void libabw::ABWContentCollector::_closeHeader()
{
  if (m_ps->m_isHeaderOpened)
  {
    while (!m_ps->m_tableStates.empty())
      _closeTable();

    _closeParagraph();
    _closeListElement();
    m_ps->m_currentListLevel = 0;
    _changeList();

    m_outputElements.addCloseHeader();

    m_ps->m_isHeaderOpened = false;
  }
  m_ps->m_currentHeaderFooterId = -1;
  m_ps->m_currentHeaderFooterOccurrence.clear();
}

void libabw::ABWContentCollector::_closeFooter()
{
  if (m_ps->m_isFooterOpened)
  {
    while (!m_ps->m_tableStates.empty())
      _closeTable();

    _closeParagraph();
    _closeListElement();
    m_ps->m_currentListLevel = 0;
    _changeList();

    m_outputElements.addCloseFooter();

    m_ps->m_isFooterOpened = false;
  }
  m_ps->m_currentHeaderFooterId = -1;
  m_ps->m_currentHeaderFooterOccurrence.clear();
}

void libabw::ABWContentCollector::_closeParagraph()
{
  if (m_ps->m_isParagraphOpened)
  {
    if (m_ps->m_isSpanOpened)
      _closeSpan();

    m_outputElements.addCloseParagraph();
  }
  m_ps->m_isParagraphOpened = false;
}

void libabw::ABWContentCollector::_closeSpan()
{
  if (m_ps->m_isSpanOpened)
    m_outputElements.addCloseSpan();

  m_ps->m_isSpanOpened = false;
}

void libabw::ABWContentCollector::_openTable()
{
  switch (m_ps->m_parsingContext)
  {
  case ABW_HEADER:
    if (!m_ps->m_isHeaderOpened)
      _openHeader();
    break;
  case ABW_FOOTER:
    if (!m_ps->m_isFooterOpened)
      _openFooter();
    break;
  case ABW_SECTION:
  default:
    if (!m_ps->m_isSectionOpened)
      _openSection();
    break;
  }

  librevenge::RVNGPropertyList propList;
  if (m_ps->m_deferredPageBreak)
    propList.insert("fo:break-before", "page");
  else if (m_ps->m_deferredColumnBreak)
    propList.insert("fo:break-before", "column");
  m_ps->m_deferredPageBreak = false;
  m_ps->m_deferredColumnBreak = false;

  librevenge::RVNGPropertyListVector columns;
  parseTableColumns(_findTableProperty("table-column-props"), columns);
  if (!columns.count())
  {
    std::map<int, int>::const_iterator iter = m_tableSizes.find(m_ps->m_tableStates.top().m_currentTableId);
    if (iter != m_tableSizes.end())
    {
      for (int j = 0; j < iter->second; ++j)
        columns.append(librevenge::RVNGPropertyList());
    }
  }
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

  m_outputElements.addOpenTable(propList);

  m_ps->m_tableStates.top().m_currentTableRow = (-1);
  m_ps->m_tableStates.top().m_currentTableCol = (-1);
  m_ps->m_tableStates.top().m_currentTableCellNumberInRow = (-1);
}

void libabw::ABWContentCollector::_closeTable()
{
  if (!m_ps->m_tableStates.empty())
  {
    if (m_ps->m_tableStates.top().m_isTableRowOpened)
      _closeTableRow();

    m_outputElements.addCloseTable();

    m_ps->m_tableStates.pop();
  }
}

void libabw::ABWContentCollector::_openTableRow()
{
  if (m_ps->m_tableStates.top().m_isTableRowOpened)
    _closeTableRow();

  m_ps->m_tableStates.top().m_currentTableCol = 0;
  m_ps->m_tableStates.top().m_currentTableCellNumberInRow = 0;

  m_outputElements.addOpenTableRow(librevenge::RVNGPropertyList());

  m_ps->m_tableStates.top().m_isTableRowOpened = true;
  m_ps->m_tableStates.top().m_isRowWithoutCell = true;
  m_ps->m_tableStates.top().m_currentTableRow++;
}

void libabw::ABWContentCollector::_closeTableRow()
{
  if (m_ps->m_tableStates.top().m_isTableRowOpened)
  {
    if (m_ps->m_tableStates.top().m_isTableCellOpened)
      _closeTableCell();

    if (m_ps->m_tableStates.top().m_isRowWithoutCell)
    {
      m_ps->m_tableStates.top().m_isRowWithoutCell = false;
      m_outputElements.addInsertCoveredTableCell(librevenge::RVNGPropertyList());
    }
    m_outputElements.addCloseTableRow();
  }
  m_ps->m_tableStates.top().m_isTableRowOpened = false;
}

void libabw::ABWContentCollector::_openTableCell()
{
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

  m_outputElements.addOpenTableCell(propList);

  m_ps->m_tableStates.top().m_currentTableCellNumberInRow++;
  m_ps->m_tableStates.top().m_isTableCellOpened = true;
  m_ps->m_tableStates.top().m_isCellWithoutParagraph = true;
  m_ps->m_tableStates.top().m_isRowWithoutCell = false;
}

void libabw::ABWContentCollector::_closeTableCell()
{
  if (m_ps->m_tableStates.top().m_isTableCellOpened)
  {
    if (m_ps->m_tableStates.top().m_isCellWithoutParagraph)
      _openSpan();
    _closeParagraph();
    _closeListElement();
    m_ps->m_currentListLevel = 0;
    _changeList();

    m_outputElements.addCloseTableCell();
  }
  m_ps->m_tableStates.top().m_isTableCellOpened = false;
}

void libabw::ABWContentCollector::openFoot(const char *id)
{
  if (!m_ps->m_isParagraphOpened && !m_ps->m_isListElementOpened)
    _openSpan();
  _closeSpan();

  librevenge::RVNGPropertyList propList;
  if (id)
    propList.insert("librevenge:number", id);
  m_outputElements.addOpenFootnote(propList);

  m_parsingStates.push(m_ps);
  m_ps = new ABWContentParsingState();

  m_ps->m_isNote = true;
}

void libabw::ABWContentCollector::closeFoot()
{
  _closeParagraph();
  _closeListElement();
  m_ps->m_currentListLevel = 0;
  _changeList();

  m_outputElements.addCloseFootnote();

  if (!m_parsingStates.empty())
  {
    delete m_ps;
    m_ps = m_parsingStates.top();
    m_parsingStates.pop();
  }
}

void libabw::ABWContentCollector::openEndnote(const char *id)
{
  if (!m_ps->m_isParagraphOpened && !m_ps->m_isListElementOpened)
    _openSpan();
  _closeSpan();

  librevenge::RVNGPropertyList propList;
  if (id)
    propList.insert("librevenge:number", id);
  m_outputElements.addOpenEndnote(propList);

  m_parsingStates.push(m_ps);
  m_ps = new ABWContentParsingState();

  m_ps->m_isNote = true;
}

void libabw::ABWContentCollector::closeEndnote()
{
  _closeParagraph();
  _closeListElement();
  m_ps->m_currentListLevel = 0;
  _changeList();

  m_outputElements.addCloseEndnote();

  if (!m_parsingStates.empty())
  {
    delete m_ps;
    m_ps = m_parsingStates.top();
    m_parsingStates.pop();
  }
}

void libabw::ABWContentCollector::openTable(const char *props)
{
  _closeParagraph();
  _closeListElement();
  m_ps->m_currentListLevel = 0;
  _changeList();

  if (m_ps->m_tableStates.empty())
  {
    switch (m_ps->m_parsingContext)
    {
    case ABW_HEADER:
      if (!m_ps->m_isHeaderOpened)
        _openHeader();
      break;
    case ABW_FOOTER:
      if (!m_ps->m_isFooterOpened)
        _openFooter();
      break;
    case ABW_SECTION:
    default:
      if (!m_ps->m_isSectionOpened)
        _openSection();
      break;
    }
  }

  m_ps->m_tableStates.push(ABWContentTableState());
  m_ps->m_tableStates.top().m_currentTableId = m_tableCounter++;
  if (props)
    parsePropString(props, m_ps->m_tableStates.top().m_currentTableProperties);

  _openTable();
}

void libabw::ABWContentCollector::closeTable()
{
  _closeParagraph();
  _closeListElement();
  m_ps->m_currentListLevel = 0;
  _changeList();
  _closeTable();
}

void libabw::ABWContentCollector::openCell(const char *props)
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

void libabw::ABWContentCollector::closeCell()
{
  _closeTableCell();
  m_ps->m_tableStates.top().m_currentCellProperties.clear();
}

void libabw::ABWContentCollector::collectData(const char *, const char *, const librevenge::RVNGBinaryData &)
{
}

void libabw::ABWContentCollector::insertImage(const char *dataid, const char *props)
{
  if (!m_ps->m_isSpanOpened)
    _openSpan();

  std::map<std::string, std::string> properties;
  if (props)
    parsePropString(props, properties);
  if (dataid)
  {
    std::map<std::string, ABWData>::const_iterator iter = m_data.find(dataid);
    if (iter != m_data.end())
    {
      librevenge::RVNGPropertyList propList;
      ABWUnit unit(ABW_NONE);
      double value(0.0);
      std::map<std::string, std::string>::const_iterator i = properties.find("height");
      if (i != properties.end() && findDouble(i->second, value, unit) && ABW_IN == unit)
        propList.insert("svg:height", value);
      i = properties.find("width");
      if (i != properties.end() && findDouble(i->second, value, unit) && ABW_IN == unit)
        propList.insert("svg:width", value);
      propList.insert("text:anchor-type", "as-char");

      m_outputElements.addOpenFrame(propList);

      propList.clear();
      propList.insert("librevenge:mime-type", iter->second.m_mimeType);
      propList.insert("office:binary-data", iter->second.m_binaryData);
      m_outputElements.addInsertBinaryObject(propList);

      m_outputElements.addCloseFrame();
    }
  }
}

void libabw::ABWContentCollector::_handleListChange()
{
  int oldListLevel;
  if (m_ps->m_listLevels.empty())
    oldListLevel = 0;
  else
    oldListLevel = m_ps->m_listLevels.top().first;

  if (m_ps->m_currentListLevel > oldListLevel)
  {
    if (!m_ps->m_isSectionOpened)
      _openSection();
    _recurseListLevels(oldListLevel, m_ps->m_currentListLevel, m_ps->m_currentListId);
  }
  else if (m_ps->m_currentListLevel < oldListLevel)
  {
    while (!m_ps->m_listLevels.empty() && m_ps->m_listLevels.top().first > m_ps->m_currentListLevel)
    {
      if (!m_ps->m_listLevels.top().second || m_ps->m_listLevels.top().second->getType() == ABW_UNORDERED)
        m_outputElements.addCloseUnorderedListLevel();
      else
        m_outputElements.addCloseOrderedListLevel();
      ABW_DEBUG_MSG(("Popped level %i off the list level stack\n", m_ps->m_listLevels.top().first));
      m_ps->m_listLevels.pop();
    }
  }
}

void libabw::ABWContentCollector::_recurseListLevels(int oldLevel, int newLevel, const librevenge::RVNGString &newListId)
{
  if (oldLevel >= newLevel || newListId.empty())
    return;
  std::map<librevenge::RVNGString, ABWListElement *>::const_iterator iter = m_listElements.find(newListId);
  if (iter != m_listElements.end() && iter->second)
  {
    _recurseListLevels(oldLevel, newLevel-1, iter->second->m_parentId);
    m_ps->m_listLevels.push(std::make_pair(newLevel, iter->second));
    librevenge::RVNGPropertyList propList;
    iter->second->writeOut(propList);
    propList.insert("librevenge:list-id", newListId);
    if (iter->second->getType() == ABW_UNORDERED)
      m_outputElements.addOpenUnorderedListLevel(propList);
    else
      m_outputElements.addOpenOrderedListLevel(propList);
  }
}

void libabw::ABWContentCollector::_changeList()
{
  _closeParagraph();
  _closeListElement();
  _handleListChange();
}

void libabw::ABWContentCollector::_closeListElement()
{
  if (m_ps->m_isListElementOpened)
  {
    if (m_ps->m_isSpanOpened)
      _closeSpan();

    m_outputElements.addCloseListElement();
  }
  m_ps->m_isListElementOpened = false;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
