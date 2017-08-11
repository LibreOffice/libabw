/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <locale>
#include <memory>
#include <sstream>

#include <boost/spirit/include/qi.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <librevenge/librevenge.h>
#include "ABWContentCollector.h"
#include "libabw_internal.h"

#define ABW_EPSILON 1.0E-06
#define MAX_LIST_LEVEL 64 // a safeguard against damaged files

using boost::optional;

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
    else if (*(i()) == '\n'|| *(i()) == (char)0x0a)
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

void parseLang(const std::string &langStr, optional<std::string> &lang, optional<std::string> &country, optional<std::string> &script)
{
  std::vector<std::string> tags;
  tags.reserve(3);
  boost::split(tags, langStr, boost::is_any_of("-_"), boost::token_compress_off);

  if ((tags.size() > 0) && boost::all(tags[0], boost::is_lower()) && (2 <= tags[0].size()) && (3 >= tags[0].size()))
  {
    lang = tags[0];

    if (tags.size() > 1)
    {
      if (boost::all(tags[1], boost::is_upper()) && (2 == tags[1].size()))
        country = tags[1];
      else
        script = tags[1];
    }

    if ((tags.size() > 2) && bool(script))
    {
      if (boost::all(tags[2], boost::is_upper()) && (2 == tags[2].size()))
        country = tags[2];
    }
  }
}

static std::string decodeUrl(const std::string &str)
{
  using namespace boost::spirit::qi;

  if (str.empty())
    return str;

  // look for a hexadecimal number of 2 digits
  uint_parser<char,16,2,2> urlhex;
  std::string decoded_string;
  auto it = str.cbegin();
  if (parse(it, str.cend(),
            +(
              (lit('%') >> (char_('%') | urlhex))
              | (!lit('%') >> char_)
            ),
            decoded_string)
      && it == str.cend())
    return decoded_string;

  return str;
}

std::string findProperty(const ABWPropertyMap &propMap, const char *const name)
{
  if (!name)
    return std::string();
  ABWPropertyMap::const_iterator iter = propMap.find(name);
  if (iter != propMap.end())
    return iter->second;
  return std::string();
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

  m_isPageFrame(false),

  m_isSpanOpened(false),
  m_isParagraphOpened(false),
  m_isListElementOpened(false),
  m_inParagraphOrListElement(false),

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
  m_currentListId(0),
  m_isFirstTextInListElement(false),

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

  m_isPageFrame(ps.m_isPageFrame),

  m_isSpanOpened(ps.m_isSpanOpened),
  m_isParagraphOpened(ps.m_isParagraphOpened),
  m_isListElementOpened(ps.m_isListElementOpened),
  m_inParagraphOrListElement(ps.m_inParagraphOrListElement),

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
  m_isFirstTextInListElement(ps.m_isFirstTextInListElement),

  m_tableStates(ps.m_tableStates),
  m_listLevels(ps.m_listLevels)
{
}

libabw::ABWContentParsingState::~ABWContentParsingState()
{
}

libabw::ABWContentCollector::ABWContentCollector(librevenge::RVNGTextInterface *iface, const std::map<int, int> &tableSizes,
                                                 const std::map<std::string, ABWData> &data,
                                                 const std::map<int, std::shared_ptr<ABWListElement>> &listElements) :
  m_ps(new ABWContentParsingState),
  m_iface(iface),
  m_parsingStates(),
  m_dontLoop(),
  m_textStyles(),
  m_documentStyle(),
  m_metadata(),
  m_data(data),
  m_tableSizes(tableSizes),
  m_tableCounter(0),
  m_outputElements(),
  m_pageOutputElements(),
  m_listElements(listElements),
  m_dummyListElements()
{
}

libabw::ABWContentCollector::~ABWContentCollector()
{
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

void libabw::ABWContentCollector::_recurseTextProperties(const char *name, ABWPropertyMap &styleProps)
{
  if (name)
  {
    m_dontLoop.insert(name);
    std::map<std::string, ABWStyle>::const_iterator iter = m_textStyles.find(name);
    if (iter != m_textStyles.end() && !(iter->second.basedon.empty()) && !m_dontLoop.count(iter->second.basedon))
      _recurseTextProperties(iter->second.basedon.c_str(), styleProps);
    if (iter != m_textStyles.end())
    {
      for (ABWPropertyMap::const_iterator i = iter->second.properties.begin(); i != iter->second.properties.end(); ++i)
        styleProps[i->first] = i->second;
    }

    // Styles based on "Heading X" style are recognized as headings.
    if (boost::starts_with(name, "Heading "))
    {
      int level = 0;
      const std::string levelStr = std::string(name).substr(8);
      if (findInt(levelStr, level))
      {
        // Abiword only has 4 levels of headings, but allow some more
        if ((0 < level) && (10 > level))
          styleProps["libabw:outline-level"] = levelStr;
      }
    }
  }
  if (!m_dontLoop.empty())
    m_dontLoop.clear();
}

std::string libabw::ABWContentCollector::_findDocumentProperty(const char *const name)
{
  return findProperty(m_documentStyle, name);
}

std::string libabw::ABWContentCollector::_findParagraphProperty(const char *name)
{
  return findProperty(m_ps->m_currentParagraphStyle, name);
}

std::string libabw::ABWContentCollector::_findTableProperty(const char *name)
{
  assert(!m_ps->m_tableStates.empty());
  return findProperty(m_ps->m_tableStates.top().m_currentTableProperties, name);
}

std::string libabw::ABWContentCollector::_findCellProperty(const char *name)
{
  assert(!m_ps->m_tableStates.empty());
  return findProperty(m_ps->m_tableStates.top().m_currentCellProperties, name);
}

std::string libabw::ABWContentCollector::_findSectionProperty(const char *name)
{
  return findProperty(m_ps->m_currentSectionStyle, name);
}

std::string libabw::ABWContentCollector::_findCharacterProperty(const char *name)
{
  std::string prop = findProperty(m_ps->m_currentCharacterStyle, name);
  if (prop.empty())
    prop = findProperty(m_ps->m_currentParagraphStyle, name);
  return prop;
}

std::string libabw::ABWContentCollector::_findMetadataEntry(const char *const name)
{
  return findProperty(m_metadata, name);
}

void libabw::ABWContentCollector::collectDocumentProperties(const char *const props)
{
  if (props)
    parsePropString(props, m_documentStyle);
}

void libabw::ABWContentCollector::_addBorderProperties(const std::map<std::string, std::string> &map, librevenge::RVNGPropertyList &propList, const std::string &defaultUndefBorderProp)
{
  int setBorders=0;
  static char const *odtWh[4]= {"fo:border-left", "fo:border-right", "fo:border-top", "fo:border-bottom"};
  for (int i=0, depl=1; i<4; ++i, depl*=2)
  {
    static char const *wh[4]= {"left", "right", "top", "bot"};
    std::string whString(wh[i]);
    std::string tmp=whString+"-color";
    if (map.find(tmp)==map.end()) continue;
    std::string color=getColor(map.find(tmp)->second);
    if (color.empty())
      continue;
    int style;
    tmp=whString+"-style";
    if (map.find(tmp)==map.end() || !findInt(map.find(tmp)->second, style))
      style=1;
    else if (style<=0 || style>=4)
    {
      if (style==0) setBorders |= depl;
      continue;
    }
    ABWUnit unit(ABW_NONE);
    double width(0.0);
    tmp=whString+"-thickness";
    if (map.find(tmp)==map.end() || !findDouble(map.find(tmp)->second, width, unit))
      width=0.01;
    else if (width<=0 || unit != ABW_IN)
      continue;
    std::stringstream s;
    s.imbue(std::locale("C")); // be sure that we use standart double
    s << width << "in ";
    if (style==2) s << "dotted ";
    else if (style==3) s << "dashed ";
    else s << "solid ";
    s << color;
    propList.insert(odtWh[i], s.str().c_str());
    setBorders|=depl;
  }
  if (defaultUndefBorderProp.empty()) return;
  for (int i=0, depl=1; i<4; ++i, depl*=2)
  {
    if (setBorders&depl) continue;
    propList.insert(odtWh[i], defaultUndefBorderProp.c_str());
  }
}

void libabw::ABWContentCollector::collectParagraphProperties(const char *level, const char *listid, const char * /*parentid*/, const char *style, const char *props)
{
  _closeBlock();
  if (!level || !findInt(level, m_ps->m_currentListLevel) || m_ps->m_currentListLevel < 1)
    m_ps->m_currentListLevel = 0;
  if (m_ps->m_currentListLevel > MAX_LIST_LEVEL)
    m_ps->m_currentListLevel = MAX_LIST_LEVEL;
  if (!listid || !findInt(listid, m_ps->m_currentListId) || m_ps->m_currentListId < 0)
    m_ps->m_currentListId = 0;

  m_ps->m_currentParagraphStyle.clear();
  if (style)
    _recurseTextProperties(style, m_ps->m_currentParagraphStyle);
  else
    _recurseTextProperties("Normal", m_ps->m_currentParagraphStyle);

  ABWPropertyMap tmpProps;
  if (props)
    parsePropString(props, tmpProps);
  for (ABWPropertyMap::const_iterator iter = tmpProps.begin(); iter != tmpProps.end(); ++iter)
    m_ps->m_currentParagraphStyle[iter->first] = iter->second;
  m_ps->m_inParagraphOrListElement = true;
}

void libabw::ABWContentCollector::collectCharacterProperties(const char *style, const char *props)
{
  // We started a new span without closing the last one. That can actually happen, because <p>
  // allows mixed content, so there can be text spans not enclosed in <c>.
  if (m_ps->m_isSpanOpened)
    _closeSpan();

  m_ps->m_currentCharacterStyle.clear();
  if (style)
    _recurseTextProperties(style, m_ps->m_currentCharacterStyle);

  ABWPropertyMap tmpProps;
  if (props)
    parsePropString(props, tmpProps);
  for (ABWPropertyMap::const_iterator iter = tmpProps.begin(); iter != tmpProps.end(); ++iter)
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
  ABWPropertyMap tmpProps;
  if (props)
    parsePropString(props, tmpProps);

  ABWUnit unit(ABW_NONE);
  double value(0.0);
  for (ABWPropertyMap::const_iterator iter = tmpProps.begin(); iter != tmpProps.end(); ++iter)
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

  std::string sType(type ? type : "");
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
  std::string widthStr(width ? width : "");
  std::string heightStr(height ? height : "");
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
    {
      m_iface->startDocument(librevenge::RVNGPropertyList());
      _setMetadata();
    }

    m_ps->m_isDocumentStarted = true;
  }
}

void libabw::ABWContentCollector::endDocument()
{
  if (!m_ps->m_isNote)
  {
    if (!m_ps->m_isPageSpanOpened)
      _openPageSpan();

    _closeBlock();

    m_ps->m_currentListLevel = 0;
    _changeList(); // flush the list

    // close the document nice and tight
    _closeSection();
    _closeHeader();
    _closeFooter();

    _closePageSpan();

    if (m_iface)
    {
      m_pageOutputElements.write(m_iface);
      m_outputElements.write(m_iface);
      m_iface->endDocument();
    }
  }
}

void libabw::ABWContentCollector::_setMetadata()
{
  librevenge::RVNGPropertyList propList;

  const std::string dcKeys[] = { "creator", "language", "publisher", "source", "subject", "title", "type" };

  for (std::size_t i = 0; i != ABW_NUM_ELEMENTS(dcKeys); ++i)
  {
    const std::string abwKey = "dc." + dcKeys[i];
    const std::string rvngKey = "dc:" + dcKeys[i];
    const std::string prop = _findMetadataEntry(abwKey.c_str());
    if (!prop.empty())
      propList.insert(rvngKey.c_str(), prop.c_str());
  }

  std::string prop = _findMetadataEntry("abiword.keywords");
  if (!prop.empty())
    propList.insert("meta:keyword", prop.c_str());

  prop = _findMetadataEntry("meta:initial-creator");
  if (!prop.empty())
    propList.insert("meta:initial-creator", prop.c_str());

  if (m_iface)
    m_iface->setDocumentMetaData(propList);
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
  _closeBlock();
  m_ps->m_currentParagraphStyle.clear();
  m_ps->m_inParagraphOrListElement = false;
}

void libabw::ABWContentCollector::openLink(const char *href)
{
  if (m_ps->m_isSpanOpened)
    _closeSpan();
  _openBlock();
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
  _closeBlock();
  m_ps->m_deferredColumnBreak = true;
}

void libabw::ABWContentCollector::insertPageBreak()
{
  _closeBlock();
  m_ps->m_deferredPageBreak = true;
}

void libabw::ABWContentCollector::insertText(const char *text)
{
  if (!m_ps->m_inParagraphOrListElement)
    return;
  if (m_ps->m_isFirstTextInListElement && text && text[0]==' ' && text[1]==0)
    return;
  if (!m_ps->m_isSpanOpened)
    _openSpan();
  if (!text)
    return;
  if (m_ps->m_isFirstTextInListElement && text[0] == '\t')
    separateSpacesAndInsertText(m_outputElements, text+1);
  else
    separateSpacesAndInsertText(m_outputElements, text);
  m_ps->m_isFirstTextInListElement = false;
}

void libabw::ABWContentCollector::_openPageSpan()
{
  if (!m_ps->m_isPageSpanOpened && !m_ps->m_isNote && m_ps->m_tableStates.empty())
  {

    if (!m_ps->m_isDocumentStarted)
      startDocument();

    librevenge::RVNGPropertyList propList;
    // assume default page size is A4
    propList.insert("fo:page-width", m_ps->m_pageWidth>0 ? m_ps->m_pageWidth : 8.27);
    propList.insert("fo:page-height", m_ps->m_pageHeight>0 ? m_ps->m_pageHeight : 11.7);
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
    if (sValue.empty()) // try document default
      sValue = _findDocumentProperty("dom-dir");
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

void libabw::ABWContentCollector::_fillParagraphProperties(librevenge::RVNGPropertyList &propList,
                                                           bool isListElement)
{
  ABWUnit unit(ABW_NONE);
  double value(0.0);
  int intValue(0);
  std::string sValue;

  if (findDouble(_findParagraphProperty("margin-right"), value, unit) && unit == ABW_IN)
    propList.insert("fo:margin-right", value);

  if (findDouble(_findParagraphProperty("margin-top"), value, unit) && unit == ABW_IN)
    propList.insert("fo:margin-top", value);

  if (findDouble(_findParagraphProperty("margin-bottom"), value, unit) && unit == ABW_IN)
    propList.insert("fo:margin-bottom", value);

  if (!isListElement)
  {
    if (findDouble(_findParagraphProperty("margin-left"), value, unit) && unit == ABW_IN)
      propList.insert("fo:margin-left", value);

    if (findDouble(_findParagraphProperty("text-indent"), value, unit) && unit == ABW_IN)
      propList.insert("fo:text-indent", value);

    // TODO: Numbered headings should probably not be handled as lists.
    // Just do not make them headings for now.
    sValue = _findParagraphProperty("libabw:outline-level");
    if (!sValue.empty())
      propList.insert("text:outline-level", sValue.c_str());
  }

  sValue = _findParagraphProperty("text-align");
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
  _addBorderProperties(m_ps->m_currentParagraphStyle, propList);
  m_ps->m_deferredPageBreak = false;
  m_ps->m_deferredColumnBreak = false;
}

void libabw::ABWContentCollector::_openBlock()
{
  if (m_ps->m_isParagraphOpened || m_ps->m_isListElementOpened)
    return;
  if (m_ps->m_currentListLevel == 0)
    _openParagraph();
  else
    _openListElement();
}

void libabw::ABWContentCollector::_closeBlock()
{
  if (!m_ps->m_isParagraphOpened && !m_ps->m_isListElementOpened)
    return;
  if (m_ps->m_isSpanOpened)
    _closeSpan();
  if (m_ps->m_isParagraphOpened)
    _closeParagraph();
  if (m_ps->m_isListElementOpened)
    _closeListElement();
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
    case ABW_FRAME_IMAGE:
      ABW_DEBUG_MSG(("libabw::ABWContentCollector::_openParagraph: can not open a paragraph\n"));
      m_ps->m_parsingContext=ABW_FRAME_UNKNOWN;
      break;
    case ABW_FRAME_TEXTBOX:
    case ABW_FRAME_UNKNOWN:
      break;
    }

    if (!m_ps->m_tableStates.empty() && !m_ps->m_tableStates.top().m_isTableCellOpened)
      _openTableCell();

    _changeList();

    librevenge::RVNGPropertyList propList;
    _fillParagraphProperties(propList, false);

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
    case ABW_FRAME_IMAGE:
      ABW_DEBUG_MSG(("libabw::ABWContentCollector::_openListElement: can not open a list\n"));
      m_ps->m_parsingContext=ABW_FRAME_UNKNOWN;
      break;
    case ABW_FRAME_TEXTBOX:
    case ABW_FRAME_UNKNOWN:
      break;
    }

    if (!m_ps->m_tableStates.empty() && !m_ps->m_tableStates.top().m_isTableCellOpened)
      _openTableCell();

    _changeList();

    librevenge::RVNGPropertyList propList;
    _fillParagraphProperties(propList, true);

    m_outputElements.addOpenListElement(propList);

    m_ps->m_isListElementOpened = true;
    if (!m_ps->m_tableStates.empty())
      m_ps->m_tableStates.top().m_isCellWithoutParagraph = false;
    m_ps->m_isFirstTextInListElement = true;
  }
}

void libabw::ABWContentCollector::_openSpan()
{
  if (!m_ps->m_isSpanOpened)
  {
    _openBlock();

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

    sValue = _findCharacterProperty("display");
    if (!sValue.empty() && sValue == "none")
      propList.insert("text:display", "none");

    sValue = _findCharacterProperty("dir-override");
    if (!sValue.empty() && sValue == "rtl")
      propList.insert("style:writing-mode", "rl-tb");

    sValue = _findCharacterProperty("text-decoration");
    std::vector<std::string> listDecorations;
    boost::split(listDecorations, sValue, boost::is_any_of(" "), boost::token_compress_on);
    for (size_t j=0; j<listDecorations.size(); ++j)
    {
      std::string const &decoration=listDecorations[j];
      if (decoration == "underline")
      {
        propList.insert("style:text-underline-type", "single");
        propList.insert("style:text-underline-style", "solid");
      }
      else if (decoration == "line-through")
      {
        propList.insert("style:text-line-through-type", "single");
        propList.insert("style:text-line-through-style", "solid");
      }
      else if (decoration == "overline")
      {
        propList.insert("style:text-overline-type", "single");
        propList.insert("style:text-overline-style", "solid");
      }
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

    sValue = _findCharacterProperty("lang");
    if (sValue.empty()) // try document default
      sValue = _findDocumentProperty("lang");

    if (!sValue.empty())
    {
      optional<std::string> lang;
      optional<std::string> country;
      optional<std::string> script;

      parseLang(sValue, lang, country, script);

      if (bool(lang))
        propList.insert("fo:language", get(lang).c_str());
      if (bool(country))
        propList.insert("fo:country", get(country).c_str());
      if (bool(script))
        propList.insert("fo:script", get(script).c_str());
    }

    // do we need to check "font-stretch" here or it is always equal to normal ?
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

    _closeBlock();
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

    _closeBlock();
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

    _closeBlock();
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
  case ABW_FRAME_IMAGE:
  case ABW_FRAME_TEXTBOX:
  case ABW_FRAME_UNKNOWN:
    break;
  }

  librevenge::RVNGPropertyList propList;
  if (m_ps->m_deferredPageBreak)
    propList.insert("fo:break-before", "page");
  else if (m_ps->m_deferredColumnBreak)
    propList.insert("fo:break-before", "column");
  m_ps->m_deferredPageBreak = false;
  m_ps->m_deferredColumnBreak = false;

  librevenge::RVNGPropertyListVector tmpColumns;
  parseTableColumns(_findTableProperty("table-column-props"), tmpColumns);
  unsigned numColumns = unsigned(tmpColumns.count());
  std::map<int, int>::const_iterator iter = m_tableSizes.find(m_ps->m_tableStates.top().m_currentTableId);
  if (iter != m_tableSizes.end())
    numColumns = unsigned(iter->second);
  librevenge::RVNGPropertyListVector columns;
  for (unsigned j = 0; j < numColumns; ++j)
  {
    if (j < tmpColumns.count())
      columns.append(tmpColumns[j]);
    else
      columns.append(librevenge::RVNGPropertyList());
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

  // by default, table cells have a small border
  _addBorderProperties(m_ps->m_tableStates.top().m_currentCellProperties, propList,"0.01in solid #000000");
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
    _closeBlock();
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
  m_ps = std::make_shared<ABWContentParsingState>();

  m_ps->m_isNote = true;
}

void libabw::ABWContentCollector::closeFoot()
{
  _closeBlock();
  m_ps->m_currentListLevel = 0;
  _changeList();

  m_outputElements.addCloseFootnote();

  if (!m_parsingStates.empty())
  {
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
  m_ps = std::make_shared<ABWContentParsingState>();

  m_ps->m_isNote = true;
}

void libabw::ABWContentCollector::closeEndnote()
{
  _closeBlock();
  m_ps->m_currentListLevel = 0;
  _changeList();

  m_outputElements.addCloseEndnote();

  if (!m_parsingStates.empty())
  {
    m_ps = m_parsingStates.top();
    m_parsingStates.pop();
  }
}

void libabw::ABWContentCollector::openField(const char *type, const char * /*id*/)
{
  if (!type || type[0]==0)
  {
    ABW_DEBUG_MSG(("libabw::ABWContentCollector::openField: called without type\n"));
    return;
  }
  if (!m_ps->m_inParagraphOrListElement)
    return;
  librevenge::RVNGPropertyList propList;
  std::string typ(type);
  size_t len=typ.length();
  // see fp_Fields.h
  switch (typ[0])
  {
  case 'a':
    if (len>4 && typ.substr(0,4)=="app_")
    {
      // app_ver, app_compiledate, app_compiletime, app_id, app_target, app_options
      return;
    }
    break;
  case 'c':
    if (len==10 && typ=="char_count")
      propList.insert("librevenge:field-type", "text:character-count");
    break;
  case 'd':
    if (len==4 && typ=="date")
    {
      propList.insert("librevenge:field-type", "text:date");
      propList.insert("number:automatic-order", "true");
      librevenge::RVNGPropertyListVector pVect;
      _convertFieldDTFormat("%A, %B %d,%Y",pVect);
      propList.insert("librevenge:value-type", "date");
      propList.insert("librevenge:format", pVect);
      break;
    }
    if (len>5 && typ.substr(0,5)=="date_")
    {
      propList.insert("librevenge:field-type", "text:date");
      propList.insert("number:automatic-order", "true");
      librevenge::RVNGPropertyListVector pVect;
      if (len==10 && typ=="date_ntdlf") // default
        ;
      else if (len==11 && typ=="date_mmddyy")
        _convertFieldDTFormat("%m/%d/%y",pVect);
      else if (len==11 && typ=="date_ddmmyy")
        _convertFieldDTFormat("%d/%m/%y",pVect);
      else if (len==8 && typ=="date_mdy")
        _convertFieldDTFormat("%B %d,%Y",pVect);
      else if (len==10 && typ=="date_mthdy")
        _convertFieldDTFormat("%b %d,%Y",pVect);
      else if (len==8 && typ=="date_dfl")
        _convertFieldDTFormat("%a %b %d %H:%M:%S %Y",pVect);
      else if (len==10 && typ=="date_wkday")
        _convertFieldDTFormat("%A",pVect);
      else if (len==8 && typ=="date_doy") // normally this is the day of the year
        _convertFieldDTFormat("%d",pVect);
      if (!pVect.empty())
      {
        propList.insert("librevenge:value-type", "date");
        propList.insert("librevenge:format", pVect);
      }
      break;
    }
    if (len==15 && typ=="datetime_custom")   // TODO add format
    {
      propList.insert("librevenge:field-type", "text:date");
      propList.insert("number:automatic-order", "true");
      librevenge::RVNGPropertyListVector pVect;
      _convertFieldDTFormat("%d/%m/%y %H:%M:%S",pVect);
      propList.insert("librevenge:value-type", "date");
      propList.insert("librevenge:format", pVect);
      break;
    }
    break;
  case 'e':
    if (len==12 && typ=="endnote_anch")
      return;
    if (len==11 && typ=="endnote_ref")
      return;
    break;
  case 'f':
    if (len==9 && typ=="file_name")
    {
      propList.insert("librevenge:field-type", "text:file-name");
      propList.insert("text:display", "full");
      break;
    }
    if (len==13 && typ=="footnote_anch")
      return;
    if (len==12 && typ=="footnote_ref")
      return;
    break;
  case 'l':
    if (len==10 && typ=="list_label")
      return;
    // line_count
    break;
  case 'm':
    if (len>5 && typ.substr(0,5)=="meta_")
    {
      if (len==10 && typ=="meta_title")
        propList.insert("librevenge:field-type", "text:title");
      else if (len==12 && typ=="meta_subject")
        propList.insert("librevenge:field-type", "text:subject");
      else if (len==12 && typ=="meta_creator")
        propList.insert("librevenge:field-type", "text:creator");
      else if (len==14 && typ=="meta_publisher")
        propList.insert("librevenge:field-type", "text:printed-by");
      //else if (len==16 && typ=="meta_contributor")
      // else if (len==9 && typ=="meta_type")
      else if (len==13 && typ=="meta_keywords")
        propList.insert("librevenge:field-type", "text:keywords");
      //else if (len==13 && typ=="meta_language")
      else if (len==16 && typ=="meta_description")
        propList.insert("librevenge:field-type", "text:description");
      //else if (len==13 && typ=="meta_coverage")
      //else if (len==11 && typ=="meta_rights")
      else if (len==9 && typ=="meta_date")
        propList.insert("librevenge:field-type", "text:creation-date");
      else if (len==22 && typ=="meta_date_last_changed")
        propList.insert("librevenge:field-type", "text:modification-date");
      break;
    }
    if (len==10 && typ=="mail_merge") // a datafield?
      return;
    break;
  case 'n':
    // nbsp_count
    break;
  case 'p':
    if (len>5 && typ.substr(0,5)=="page_")
    {
      if (len==11 && typ=="page_number")
        propList.insert("librevenge:field-type", "text:page-number");
      else if (len==10 && typ=="page_count")
        propList.insert("librevenge:field-type", "text:page-count");
      // page_ref ?
      break;
    }
    if (len>5 && typ.substr(0,5)=="para_")
    {
      if (len==10 && typ=="para_count")
        propList.insert("librevenge:field-type", "text:paragraph-count");
      break;
    }
    break;
  case 's':
    if (len==15 && typ=="short_file_name")
    {
      propList.insert("librevenge:field-type", "text:file-name");
      propList.insert("text:display", "full"); // checkme
      break;
    }
    if (len>4 && typ.substr(0,4)=="sum_")
    {
      // sum_cols, sum_rows
      break;
    }
    break;
  case 't':
    if (len==4 && typ=="time")
    {
      propList.insert("librevenge:field-type", "text:time");
      propList.insert("number:automatic-order", "true");
      break;
    }
    if (len>5 && typ.substr(0,5)=="time_")   // TODO add format
    {
      propList.insert("librevenge:field-type", "text:time");
      propList.insert("number:automatic-order", "true");
      librevenge::RVNGPropertyListVector pVect;
      if (len==9 && typ=="time_ampm")
        _convertFieldDTFormat("%I:%M:%S %p",pVect);
      if (len==9 && typ=="time_zone") // CEST, ...
        return;
      // if (len==10 && typ=="time_epoch") //second since ...
      if (len==12 && typ=="time_miltime") // ""
        return;
      if (!pVect.empty())
      {
        propList.insert("librevenge:value-type", "time");
        propList.insert("librevenge:format", pVect);
      }
      break;
    }
    if (len>4 && typ.substr(0,4)=="toc_")
    {
      if (len==14 && typ=="toc_list_label")
        return;
      // toc_page_number
      break;
    }
    break;
  case 'w':
    if (len==11 && typ=="word_count")
      propList.insert("librevenge:field-type", "text:word-count");
    break;
  default:
    break;
  }
  if (propList.empty())
  {
    ABW_DEBUG_MSG(("libabw::ABWContentCollector::openField: sorry, unknown type=%s\n", type));
  }
  else
  {
    if (!m_ps->m_isSpanOpened)
      _openSpan();
    m_outputElements.addInsertField(propList);
    m_ps->m_isFirstTextInListElement = false;
  }
}

bool libabw::ABWContentCollector::_convertFieldDTFormat(std::string const &dtFormat, librevenge::RVNGPropertyListVector &propVect)
{
  propVect.clear();
  size_t len=dtFormat.size();
  std::string text("");
  librevenge::RVNGPropertyList list;
  for (size_t c=0; c < len; ++c)
  {
    if (dtFormat[c]!='%' || c+1==len)
    {
      text+=dtFormat[c];
      continue;
    }
    char ch=dtFormat[++c];
    if (ch=='%')
    {
      text += '%';
      continue;
    }
    if (!text.empty())
    {
      list.clear();
      list.insert("librevenge:value-type", "text");
      list.insert("librevenge:text", text.c_str());
      propVect.append(list);
      text.clear();
    }
    list.clear();
    switch (ch)
    {
    case 'Y':
      list.insert("number:style", "long");
    /* FALLTHRU */
    case 'y':
      list.insert("librevenge:value-type", "year");
      propVect.append(list);
      break;
    case 'B':
      list.insert("number:style", "long");
    /* FALLTHRU */
    case 'b':
    case 'h':
      list.insert("librevenge:value-type", "month");
      list.insert("number:textual", true);
      propVect.append(list);
      break;
    case 'm':
      list.insert("librevenge:value-type", "month");
      propVect.append(list);
      break;
    case 'e':
      list.insert("number:style", "long");
    /* FALLTHRU */
    case 'd':
      list.insert("librevenge:value-type", "day");
      propVect.append(list);
      break;
    case 'A':
      list.insert("number:style", "long");
    /* FALLTHRU */
    case 'a':
      list.insert("librevenge:value-type", "day-of-week");
      propVect.append(list);
      break;

    case 'H':
      list.insert("number:style", "long");
    /* FALLTHRU */
    case 'I':
      list.insert("librevenge:value-type", "hours");
      propVect.append(list);
      break;
    case 'M':
      list.insert("librevenge:value-type", "minutes");
      list.insert("number:style", "long");
      propVect.append(list);
      break;
    case 'S':
      list.insert("librevenge:value-type", "seconds");
      list.insert("number:style", "long");
      propVect.append(list);
      break;
    case 'p':
      list.clear();
      list.insert("librevenge:value-type", "am-pm");
      propVect.append(list);
      break;
#if !defined(__clang__)
    default:
      ABW_DEBUG_MSG(("libabw::ABWContentCollector::_convertFieldDTFormat: find unimplement command %c(ignored)\n", ch));
#endif
    }
  }
  if (!text.empty())
  {
    list.clear();
    list.insert("librevenge:value-type", "text");
    list.insert("librevenge:text", text.c_str());
    propVect.append(list);
  }
  return propVect.count()!=0;
}

void libabw::ABWContentCollector::closeField()
{
}

void libabw::ABWContentCollector::openTable(const char *props)
{
  _closeBlock();
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
    case ABW_FRAME_IMAGE:
    case ABW_FRAME_TEXTBOX:
    case ABW_FRAME_UNKNOWN:
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
  _closeBlock();
  m_ps->m_currentListLevel = 0;
  _changeList();
  _closeTable();
}

void libabw::ABWContentCollector::openCell(const char *props)
{
  if (!m_ps->m_tableStates.empty())
  {
    if (props)
      parsePropString(props, m_ps->m_tableStates.top().m_currentCellProperties);
    const int currentRow(getCellPos("top-attach", "bottom-attach", m_ps->m_tableStates.top().m_currentTableRow + 1));

    while (m_ps->m_tableStates.top().m_currentTableRow < currentRow)
    {
      if (m_ps->m_tableStates.top().m_currentTableRow >= 0)
        _closeTableRow();
      _openTableRow();
    }

    m_ps->m_tableStates.top().m_currentTableCol =
      getCellPos("left-attach", "right-attach", m_ps->m_tableStates.top().m_currentTableCol + 1);
  }
}

int libabw::ABWContentCollector::getCellPos(const char *startProp, const char *endProp, int defStart)
{
  int startAttach(0);
  const bool haveStart(findInt(_findCellProperty(startProp), startAttach));
  int endAttach(0);
  const bool haveEnd(findInt(_findCellProperty(endProp), endAttach));

  int newStartAttach(startAttach);

  if (haveStart && haveEnd)
  {
    if (endAttach <= startAttach && endAttach > 0)
      newStartAttach = endAttach - 1;
  }
  else if (haveStart && !haveEnd)
  {
    if (startAttach / 1000 > defStart) // likely a damaged input
      newStartAttach = defStart;
  }
  else if (!haveStart && haveEnd)
  {
    if (endAttach <= 0 || endAttach / 1000 > defStart) // likely a damaged input
      newStartAttach = defStart;
    else
      newStartAttach = endAttach - 1;
  }
  else
  {
    newStartAttach = defStart;
  }

  return newStartAttach;
}

void libabw::ABWContentCollector::closeCell()
{
  if (!m_ps->m_tableStates.empty())
  {
    _closeTableCell();
    m_ps->m_tableStates.top().m_currentCellProperties.clear();
  }
}

void libabw::ABWContentCollector::openFrame(const char *props, const char *imageId, const char */*title*/, const char */*alt*/)
{
  ABWPropertyMap propMap;
  if (props)
    parsePropString(props, propMap);
  ABWPropertyMap::const_iterator iter;

  librevenge::RVNGPropertyList propList;
  ABWUnit unit(ABW_NONE);
  double value(0.0);
  // size
  iter = propMap.find("frame-height");
  if (iter != propMap.end() && findDouble(iter->second, value, unit) && ABW_IN == unit)
    propList.insert("svg:height", value);
  iter = propMap.find("frame-width");
  if (iter != propMap.end() && findDouble(iter->second, value, unit) && ABW_IN == unit)
    propList.insert("svg:width", value);
  // position
  bool isParagraph=true;
  iter = propMap.find("position-to");
  if (iter != propMap.end())
  {
    if (iter->second=="page-above-text")
      isParagraph=false;
    else if (iter->second=="column-above-text")
      /* unsure how to retrieve that, so check if the page positions
         are defined, if yes, use a page anchor. */
      isParagraph=(propMap.find("frame-page-ypos")==propMap.end());
    else if (iter->second!="block-above-text")
    {
      ABW_DEBUG_MSG(("libabw::ABWContentCollector::openFrame: sorry, unknown pos: %s asume paragraph\n", iter->second.c_str()));
    }
  }
  iter = propMap.find(isParagraph ? "xpos" : "frame-page-xpos");
  if (iter != propMap.end() && findDouble(iter->second, value, unit) && ABW_IN == unit)
    propList.insert("svg:x", value);
  iter = propMap.find(isParagraph ? "ypos" : "frame-page-ypos");
  if (iter != propMap.end() && findDouble(iter->second, value, unit) && ABW_IN == unit)
    propList.insert("svg:y", value);
  if (!isParagraph)
  {
    propList.insert("style:vertical-rel", "page");
    propList.insert("style:horizontal-rel", "page");
  }
  if (!isParagraph)
  {
    iter = propMap.find("frame-pref-page");
    int page=0;
    if (iter != propMap.end() && findInt(iter->second, page))
      propList.insert("text:anchor-page-number", page+1);
  }
  // style
  int intValue;
  iter = propMap.find("bg-style"); // 0: none, 1: color=background-color
  if (iter != propMap.end() && findInt(iter->second, intValue) && intValue==1)
  {
    iter = propMap.find("background-color");
    if (iter != propMap.end())
    {
      std::string color("#");
      color+=iter->second;
      propList.insert("fo:background-color", color.c_str());
    }
  }
  propList.insert("text:anchor-type", isParagraph ? "paragraph" : "page");
  iter = propMap.find("wrap-mode");
  if (iter != propMap.end())
  {
    if (iter->second=="wrapped-to-left")
      propList.insert("style:wrap", "left");
    else if (iter->second=="wrapped-to-right")
      propList.insert("style:wrap", "right");
    else if (iter->second=="wrapped-to-both")
      propList.insert("style:wrap", "parallel");
    else if (iter->second=="above-text")
    {
      propList.insert("style:wrap", "dynamic");
      propList.insert("style:run-through", "foreground");
    }
    else if (iter->second=="below-text")
    {
      propList.insert("style:wrap", "dynamic");
      propList.insert("style:run-through", "background");
    }
    else
    {
      ABW_DEBUG_MSG(("libabw::ABWContentCollector::openFrame: sorry, unknown wrap mode: %s\n", iter->second.c_str()));
    }
  }
  m_ps->m_isPageFrame=!isParagraph;
  m_outputElements.addOpenFrame(propList);

  iter = propMap.find("frame-type");
  if (iter==propMap.end())
  {
    ABW_DEBUG_MSG(("libabw::ABWContentCollector::openFrame: can not find the frame type\n"));
  }
  else if (iter->second=="image")
  {
    m_ps->m_parsingContext=ABW_FRAME_IMAGE;
    std::map<std::string, ABWData>::const_iterator imIter = m_data.end();
    if (imageId) imIter= m_data.find(imageId);
    if (imIter==m_data.end())
    {
      ABW_DEBUG_MSG(("libabw::ABWContentCollector::openFrame: can not find the image\n"));
      return;
    }

    propList.clear();
    propList.insert("librevenge:mime-type", imIter->second.m_mimeType);
    propList.insert("office:binary-data", imIter->second.m_binaryData);
    m_outputElements.addInsertBinaryObject(propList);

    return;
  }
  else if (iter->second=="textbox")
  {
    m_ps->m_parsingContext=ABW_FRAME_TEXTBOX;
    propList.clear();
    m_outputElements.addOpenTextBox(propList);
    return;
  }
  m_ps->m_parsingContext=ABW_FRAME_UNKNOWN;
  ABW_DEBUG_MSG(("libabw::ABWContentCollector::openFrame: sorry, unknown frame type: %s\n", iter->second.c_str()));
}

void libabw::ABWContentCollector::closeFrame(libabw::ABWOutputElements *(&elements), bool &pageFrame)
{
  elements=0;
  pageFrame=false;
  if (m_ps->m_isNote)
  {
    ABW_DEBUG_MSG(("libabw::ABWContentCollector::closeFrame: sorry, oops, sorry, a note is not closed\n"));
    return;
  }
  if (m_ps->m_parsingContext!=ABW_FRAME_IMAGE && m_ps->m_parsingContext!=ABW_FRAME_TEXTBOX)
    return;

  while (!m_ps->m_tableStates.empty())
    _closeTable();
  _closeBlock();
  m_ps->m_currentListLevel = 0;
  _changeList(); // flush the list

  if (m_ps->m_parsingContext==ABW_FRAME_TEXTBOX)
    m_outputElements.addCloseTextBox();
  m_outputElements.addCloseFrame();
  elements=&m_outputElements;
  pageFrame=m_ps->m_isPageFrame;
}

void libabw::ABWContentCollector::addFrameElements(ABWOutputElements &elements, bool pageFrame)
{
  if (pageFrame)
    m_pageOutputElements.splice(elements);
  else
  {
    _openBlock();
    m_outputElements.splice(elements);
  }
}

void libabw::ABWContentCollector::collectData(const char *, const char *, const librevenge::RVNGBinaryData &)
{
}

void libabw::ABWContentCollector::insertImage(const char *dataid, const char *props)
{
  if (!m_ps->m_isSpanOpened)
    _openSpan();

  ABWPropertyMap properties;
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
      ABWPropertyMap::const_iterator i = properties.find("height");
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

void libabw::ABWContentCollector::_writeOutDummyListLevels(int oldLevel, int newLevel)
{
  if (oldLevel < newLevel)
  {
    _writeOutDummyListLevels(oldLevel, newLevel-1);
    m_dummyListElements.push_back(std::make_shared<ABWUnorderedListElement>());
    m_dummyListElements.back()->m_listLevel = newLevel;
    m_ps->m_listLevels.push(std::make_pair(newLevel, m_dummyListElements.back()));
    librevenge::RVNGPropertyList propList;
    m_dummyListElements.back()->writeOut(propList);
    m_outputElements.addOpenUnorderedListLevel(propList);
  }
}

void libabw::ABWContentCollector::_recurseListLevels(int oldLevel, int newLevel, int newListId)
{
  if (oldLevel >= newLevel)
    return;
  const auto iter = m_listElements.find(newListId);
  if (iter != m_listElements.end() && iter->second)
  {
    if (iter->second->m_parentId)
      _recurseListLevels(oldLevel, newLevel-1, iter->second->m_parentId);
    else
      _writeOutDummyListLevels(oldLevel, newLevel-1);
    m_ps->m_listLevels.push(std::make_pair(newLevel, iter->second));
    librevenge::RVNGPropertyList propList;
    iter->second->writeOut(propList);
    // osnola: use the element list id if set, if not use newListId
    propList.insert("librevenge:list-id",
                    iter->second->m_listId ? iter->second->m_listId : newListId);
    if (iter->second->getType() == ABW_UNORDERED)
      m_outputElements.addOpenUnorderedListLevel(propList);
    else
      m_outputElements.addOpenOrderedListLevel(propList);
  }
}

void libabw::ABWContentCollector::_changeList()
{
  _closeBlock();
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
  m_ps->m_isFirstTextInListElement = false;
}

void libabw::ABWContentCollector::addMetadataEntry(const char *const key, const char *const value)
{
  assert(key);
  assert(value);

  m_metadata[key] = value;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
