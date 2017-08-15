/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <boost/spirit/include/qi.hpp>

#include "ABWCollector.h"

bool libabw::findInt(const std::string &str, int &res)
{
  using namespace boost::spirit::qi;

  if (str.empty())
    return false;

  auto it = str.cbegin();
  return phrase_parse(it, str.cend(), int_, space, res) && it == str.cend();
}

void libabw::parsePropString(const std::string &str, ABWPropertyMap &props)
{
  if (str.empty())
    return;

  std::string propString(boost::trim_copy(str));
  std::vector<std::string> strVec;
  boost::algorithm::split(strVec, propString, boost::is_any_of(";"), boost::token_compress_on);
  for (auto &i : strVec)
  {
    boost::algorithm::trim(i);
    std::vector<std::string> tmpVec;
    boost::algorithm::split(tmpVec, i, boost::is_any_of(":"), boost::token_compress_on);
    if (tmpVec.size() == 2)
      props[tmpVec[0]] = tmpVec[1];
  }
}

bool libabw::findDouble(const std::string &str, double &res, ABWUnit &unit)
{
  using namespace boost::spirit::qi;

  if (str.empty())
    return false;

  symbols<char, std::pair<ABWUnit, double>> units;
  units.add
  ("cm", {ABW_IN, 2.54})
  ("inch", {ABW_IN, 1.0})
  ("in", {ABW_IN, 1.0})
  ("mm", {ABW_IN, 25.4})
  ("pi", {ABW_IN, 6.0})
  ("pt", {ABW_IN, 72.0})
  ("px", {ABW_IN, 72.0})
  ("%", {ABW_PERCENT, 100.0})
  ;

  boost::optional<std::pair<ABWUnit, double>> u;

  auto it = str.cbegin();
  if (!phrase_parse(it, str.cend(), double_ >> -units, space, res, u) || it != str.cend())
    return false;

  if (u)
  {
    unit = get(u).first;
    res /= get(u).second;
  }
  else
  {
    unit = ABW_PERCENT;
  }

  return true;
}

void libabw::ABWListElement::writeOut(librevenge::RVNGPropertyList &propList) const
{
  if (m_listLevel > 0)
    propList.insert("librevenge:level", m_listLevel);
  propList.insert("text:min-label-width", m_minLabelWidth);
  propList.insert("text:space-before", m_spaceBefore);
}

void libabw::ABWOrderedListElement::writeOut(librevenge::RVNGPropertyList &propList) const
{
  libabw::ABWListElement::writeOut(propList);
  propList.insert("style:num-format", m_numFormat);
  if (!m_numPrefix.empty())
    propList.insert("style:num-prefix", m_numPrefix);
  if (!m_numSuffix.empty())
    propList.insert("style:num-suffix", m_numSuffix);
  if (m_startValue >= 0)
    propList.insert("text:start-value", m_startValue);
}

void libabw::ABWUnorderedListElement::writeOut(librevenge::RVNGPropertyList &propList) const
{
  libabw::ABWListElement::writeOut(propList);
  propList.insert("text:bullet-char", m_bulletChar);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
