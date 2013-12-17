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

static bool findInt(const std::string &str, int &res)
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

static void parsePropString(const std::string &str, std::map<std::string, std::string> &props)
{
  if (str.empty())
    return;

  std::string propString = boost::trim_copy(str);
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

libabw::ABWStylesCollector::ABWStylesCollector(std::map<int, int> &tableSizes, std::map<std::string, ABWData> &data) :
  m_ps(new ABWStylesParsingState),
  m_tableSizes(tableSizes),
  m_data(data),
  m_tableCounter(0) {}

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
  m_data[name] = ABWData(mimeType, data);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
