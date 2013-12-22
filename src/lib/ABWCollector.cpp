/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ABWCollector.h"

void libabw::ABWListElement::writeOut(librevenge::RVNGPropertyList &propList) const
{
  if (m_listLevel >= 0)
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
  if (!m_bulletChar.empty())
    propList.insert("text:bullet-char", m_bulletChar);
}


/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
