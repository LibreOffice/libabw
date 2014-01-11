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
#include "ABWStylesCollector.h"
#include "libabw_internal.h"

#define ABW_EPSILON 1.0E-06

namespace libabw
{

namespace
{

enum ABWListType
{
  NUMBERED_LIST = 0,
  LOWERCASE_LIST = 1,
  UPPERCASE_LIST = 2,
  LOWERROMAN_LIST = 3,
  UPPERROMAN_LIST = 4,

  BULLETED_LIST = 5,
  DASHED_LIST = 6,
  SQUARE_LIST = 7,
  TRIANGLE_LIST = 8,
  DIAMOND_LIST = 9,
  STAR_LIST = 10,
  IMPLIES_LIST = 11,
  TICK_LIST = 12,
  BOX_LIST = 13,
  HAND_LIST = 14,
  HEART_LIST = 15,
  ARROWHEAD_LIST = 16,

  LAST_BULLETED_LIST = 17,
  OTHER_NUMBERED_LISTS = 0x7f,
  ARABICNUMBERED_LIST = 0x80,
  HEBREW_LIST = 0x81,
  NOT_A_LIST = 0xff
};

static int abw_unichar_to_utf8(uint32_t c, char *outbuf)
{
  uint8_t len = 1;
  uint8_t first = 0;

  if (c < 0x80)
  {
    first = 0;
    len = 1;
  }
  else if (c < 0x800)
  {
    first = 0xc0;
    len = 2;
  }
  else if (c < 0x10000)
  {
    first = 0xe0;
    len = 3;
  }
  else if (c < 0x200000)
  {
    first = 0xf0;
    len = 4;
  }
  else if (c < 0x4000000)
  {
    first = 0xf8;
    len = 5;
  }
  else
  {
    first = 0xfc;
    len = 6;
  }

  if (outbuf)
  {
    for (uint8_t i = (uint8_t)(len - 1); i > 0; --i)
    {
      outbuf[i] = (char)((c & 0x3f) | 0x80);
      c >>= 6;
    }
    outbuf[0] = (char)(c | first);
  }

  return len;
}

static void appendUCS4(librevenge::RVNGString &str, uint32_t ucs4)
{
  int charLength = abw_unichar_to_utf8(ucs4, 0);
  char *utf8 = new char[charLength+1];
  utf8[charLength] = '\0';
  abw_unichar_to_utf8(ucs4, utf8);
  str.append(utf8);

  delete[] utf8;
}

} // anonymous namespace

} // namespace libabw

libabw::ABWStylesTableState::ABWStylesTableState() :
  m_currentCellProperties(),
  m_currentTableWidth(0),
  m_currentTableRow(-1),
  m_currentTableId(-1) {}

libabw::ABWStylesTableState::ABWStylesTableState(const ABWStylesTableState &ts) :
  m_currentCellProperties(ts.m_currentCellProperties),
  m_currentTableWidth(ts.m_currentTableWidth),
  m_currentTableRow(ts.m_currentTableRow),
  m_currentTableId(ts.m_currentTableId) {}

libabw::ABWStylesTableState::~ABWStylesTableState() {}

libabw::ABWStylesParsingState::ABWStylesParsingState() :
  m_tableStates() {}

libabw::ABWStylesParsingState::ABWStylesParsingState(const ABWStylesParsingState &ps) :
  m_tableStates(ps.m_tableStates) {}

libabw::ABWStylesParsingState::~ABWStylesParsingState() {}

libabw::ABWStylesCollector::ABWStylesCollector(std::map<int, int> &tableSizes,
                                               std::map<std::string, ABWData> &data,
                                               std::map<librevenge::RVNGString, ABWListElement *> &listElements) :
  m_ps(new ABWStylesParsingState),
  m_tableSizes(tableSizes),
  m_data(data),
  m_tableCounter(0),
  m_listElements(listElements) {}

libabw::ABWStylesCollector::~ABWStylesCollector()
{
  DELETEP(m_ps);
}

void libabw::ABWStylesCollector::openTable(const char *)
{
  m_ps->m_tableStates.push(ABWStylesTableState());
  m_ps->m_tableStates.top().m_currentTableId = m_tableCounter++;
  m_ps->m_tableStates.top().m_currentTableRow = -1;
  m_ps->m_tableStates.top().m_currentTableWidth = 0;
}

void libabw::ABWStylesCollector::closeTable()
{
  m_tableSizes[m_ps->m_tableStates.top().m_currentTableId]
    = m_ps->m_tableStates.top().m_currentTableWidth;
  if (!m_ps->m_tableStates.empty())
    m_ps->m_tableStates.pop();
}

void libabw::ABWStylesCollector::openCell(const char *props)
{
  if (props)
    parsePropString(props, m_ps->m_tableStates.top().m_currentCellProperties);
  int currentRow(0);
  if (!findInt(_findCellProperty("top-attach"), currentRow))
    currentRow = m_ps->m_tableStates.top().m_currentTableRow + 1;
  while (m_ps->m_tableStates.top().m_currentTableRow < currentRow)
    m_ps->m_tableStates.top().m_currentTableRow++;

  if (!m_ps->m_tableStates.empty() && 0 == m_ps->m_tableStates.top().m_currentTableRow)
  {
    int leftAttach(0);
    int rightAttach(0);
    if (findInt(_findCellProperty("left-attach"), leftAttach) && findInt(_findCellProperty("right-attach"), rightAttach))
      m_ps->m_tableStates.top().m_currentTableWidth += rightAttach - leftAttach;
    else
      m_ps->m_tableStates.top().m_currentTableWidth++;
  }
}

void libabw::ABWStylesCollector::closeCell()
{
  m_ps->m_tableStates.top().m_currentCellProperties.clear();
}

std::string libabw::ABWStylesCollector::_findCellProperty(const char *name)
{
  std::map<std::string, std::string>::const_iterator iter = m_ps->m_tableStates.top().m_currentCellProperties.find(name);
  if (iter != m_ps->m_tableStates.top().m_currentCellProperties.end())
    return iter->second;
  return std::string();
}

void libabw::ABWStylesCollector::collectData(const char *name, const char *mimeType, const librevenge::RVNGBinaryData &data)
{
  if (!name)
    return;
  m_data[name] = ABWData(mimeType ? mimeType : "", data);
}

void libabw::ABWStylesCollector::_processList(const char *id, const char *listDelim,
                                              const char *parentid, const char *startValue, int type)
{
  using namespace boost;
  using namespace boost::algorithm;

  if (type >= BULLETED_LIST && type < LAST_BULLETED_LIST)
  {
    ABWUnorderedListElement *tmpElement = new ABWUnorderedListElement();
    switch (type)
    {
    case BULLETED_LIST:
      appendUCS4(tmpElement->m_bulletChar, 0x2022);
      break;
    case DASHED_LIST:
      appendUCS4(tmpElement->m_bulletChar, 0x002D);
      break;
    case SQUARE_LIST:
      appendUCS4(tmpElement->m_bulletChar, 0x25A0);
      break;
    case TRIANGLE_LIST:
      appendUCS4(tmpElement->m_bulletChar, 0x25B2);
      break;
    case DIAMOND_LIST:
      appendUCS4(tmpElement->m_bulletChar, 0x2666);
      break;
    case STAR_LIST:
      appendUCS4(tmpElement->m_bulletChar, 0x2733);
      break;
    case IMPLIES_LIST:
      appendUCS4(tmpElement->m_bulletChar, 0x21D2);
      break;
    case TICK_LIST:
      appendUCS4(tmpElement->m_bulletChar, 0x2713);
      break;
    case BOX_LIST:
      appendUCS4(tmpElement->m_bulletChar, 0x2752);
      break;
    case HAND_LIST:
      appendUCS4(tmpElement->m_bulletChar, 0x261E);
      break;
    case HEART_LIST:
      appendUCS4(tmpElement->m_bulletChar, 0x2665);
      break;
    case ARROWHEAD_LIST:
      appendUCS4(tmpElement->m_bulletChar, 0x27A3);
      break;
    default:
      tmpElement->m_bulletChar = "*"; // for the while
      break;
    }
    m_listElements[id] = tmpElement;
  }
  else
  {
    ABWOrderedListElement *tmpElement = new ABWOrderedListElement();
    switch (type)
    {
    case NUMBERED_LIST:
      tmpElement->m_numFormat = "1";
      break;
    case LOWERCASE_LIST:
      tmpElement->m_numFormat = "a";
      break;
    case UPPERCASE_LIST:
      tmpElement->m_numFormat = "A";
      break;
    case LOWERROMAN_LIST:
      tmpElement->m_numFormat = "i";
      break;
    case UPPERROMAN_LIST:
      tmpElement->m_numFormat = "I";
      break;
    default:
      tmpElement->m_numFormat = "1";
      break;
    }
    if (!startValue || !findInt(startValue, tmpElement->m_startValue))
      tmpElement->m_startValue = 0;

    // get prefix and suffix by splitting the listDelim
    if (listDelim)
    {
      std::string delim(listDelim);
      std::vector<librevenge::RVNGString> strVec;

      for (split_iterator<std::string::iterator> It =
             make_split_iterator(delim, first_finder("%L", is_iequal()));
           It != split_iterator<std::string::iterator>(); ++It)
      {
        strVec.push_back(copy_range<std::string>(*It).c_str());
      }
      if (2 <= strVec.size())
      {
        tmpElement->m_numPrefix = strVec[0];
        tmpElement->m_numSuffix = strVec[1];
      }
    }
    m_listElements[id] = tmpElement;
  }
  if (parentid)
    m_listElements[id]->m_parentId = parentid;
}

void libabw::ABWStylesCollector::collectList(const char *id, const char *, const char *listDelim,
                                             const char *parentid, const char *startValue, const char *type)
{
  if (!id)
    return;
  if (m_listElements[id])
    delete m_listElements[id];
  int intType;
  if (!type || !findInt(type, intType))
    intType = 5;
  _processList(id, listDelim, parentid, startValue, intType);
}

void libabw::ABWStylesCollector::collectParagraphProperties(const char *level, const char *listid, const char *parentid, const char * /* style */, const char *props)
{
  std::map<std::string, std::string> properties;
  if (props)
    parsePropString(props, properties);

  if (listid)
  {
    std::map<librevenge::RVNGString, ABWListElement *>::iterator iter = m_listElements.find(listid);
    if (iter == m_listElements.end() || !iter->second)
    {
      std::map<std::string, std::string>::const_iterator i = properties.find("list-style");
      int listStyle = BULLETED_LIST;
      if (i != properties.end())
      {
        if (i->second == "Numbered List")
          listStyle = NUMBERED_LIST;
        else if (i->second == "Lower Case List")
          listStyle = LOWERCASE_LIST;
        else if (i->second == "Upper Case List")
          listStyle = UPPERCASE_LIST;
        else if (i->second == "Lower Roman List")
          listStyle = LOWERROMAN_LIST;
        else if (i->second == "Upper Roman List")
          listStyle = UPPERROMAN_LIST;
        else if (i->second == "Hebrew List")
          listStyle = HEBREW_LIST;
        else if (i->second == "Arabic List")
          listStyle = ARABICNUMBERED_LIST;
        else if (i->second == "Bullet List")
          listStyle = BULLETED_LIST;
        else if (i->second == "Dashed List")
          listStyle = DASHED_LIST;
        else if (i->second == "Square List")
          listStyle = SQUARE_LIST;
        else if (i->second == "Triangle List")
          listStyle = TRIANGLE_LIST;
        else if (i->second == "Diamond List")
          listStyle = DIAMOND_LIST;
        else if (i->second == "Star List")
          listStyle = STAR_LIST;
        else if (i->second == "Implies List")
          listStyle = IMPLIES_LIST;
        else if (i->second == "Tick List")
          listStyle = TICK_LIST;
        else if (i->second == "Box List")
          listStyle = BOX_LIST;
        else if (i->second == "Hand List")
          listStyle = HAND_LIST;
        else if (i->second == "Heart List")
          listStyle = HEART_LIST;
        else if (i->second == "Arrowhead List")
          listStyle = ARROWHEAD_LIST;
      }
      i = properties.find("start-value");
      std::string startValue;
      if (i != properties.end())
        startValue = i->second;
      _processList(listid, "%L", parentid, startValue.empty() ? 0 : startValue.c_str(), listStyle);
      iter = m_listElements.find(listid);
    }
    if (iter != m_listElements.end() && iter->second)
    {
      ABWListElement *listElement = iter->second;

      if (!level || !findInt(level, listElement->m_listLevel))
        listElement->m_listLevel = 0;

      std::map<std::string, std::string>::const_iterator i = properties.find("margin-left");
      ABWUnit unit(ABW_NONE);
      double marginLeft(0.0);
      if (i == properties.end() || !findDouble(i->second, marginLeft, unit) || unit != ABW_IN)
        marginLeft = 0.0;
      i = properties.find("text-indent");
      double textIndent(0.0);
      if (i == properties.end() || !findDouble(i->second, textIndent, unit) || unit != ABW_IN)
        textIndent = 0.0;
      listElement->m_minLabelWidth = -textIndent;
      listElement->m_spaceBefore = marginLeft + textIndent;
    }
  }
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
