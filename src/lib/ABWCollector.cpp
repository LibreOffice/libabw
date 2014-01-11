/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/classic.hpp>
#include "ABWCollector.h"

bool libabw::findInt(const std::string &str, int &res)
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

void libabw::parsePropString(const std::string &str, std::map<std::string, std::string> &props)
{
  if (str.empty())
    return;

  std::string propString(boost::trim_copy(str));
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

bool libabw::findDouble(const std::string &str, double &res, ABWUnit &unit)
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
                 str_p("inch")[assign_a(unit,ABW_IN)]
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

void libabw::ABWListElement::writeOut(WPXPropertyList &propList) const
{
  if (m_listLevel > 0)
    propList.insert("libwpd:level", m_listLevel);
  propList.insert("text:min-label-width", m_minLabelWidth);
  propList.insert("text:space-before", m_spaceBefore);
}

void libabw::ABWOrderedListElement::writeOut(WPXPropertyList &propList) const
{
  libabw::ABWListElement::writeOut(propList);
  propList.insert("style:num-format", m_numFormat);
  if (m_numPrefix.len())
    propList.insert("style:num-prefix", m_numPrefix);
  if (m_numSuffix.len())
    propList.insert("style:num-suffix", m_numSuffix);
  if (m_startValue >= 0)
    propList.insert("text:start-value", m_startValue);
}

void libabw::ABWUnorderedListElement::writeOut(WPXPropertyList &propList) const
{
  libabw::ABWListElement::writeOut(propList);
  if (m_bulletChar.len())
    propList.insert("text:bullet-char", m_bulletChar);
}


/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
