/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <librevenge/librevenge.h>
#include "ABWCollector.h"

libabw::ABWCollector::ABWCollector(librevenge::RVNGTextInterface *iface) :
  m_iface(iface)
{
}

libabw::ABWCollector::~ABWCollector()
{
}

void libabw::ABWCollector::startDocument()
{
  if (m_iface)
    m_iface->startDocument();
}

void libabw::ABWCollector::endDocument()
{
  if (m_iface)
    m_iface->endDocument();
}

void libabw::ABWCollector::startSection()
{
}

void libabw::ABWCollector::endSection()
{
}

void libabw::ABWCollector::openParagraph()
{
  if (m_iface)
    m_iface->openParagraph(librevenge::RVNGPropertyList());
}

void libabw::ABWCollector::closeParagraph()
{
  if (m_iface)
    m_iface->closeParagraph();
}

void libabw::ABWCollector::openSpan()
{
  if (m_iface)
    m_iface->openSpan(librevenge::RVNGPropertyList());
}

void libabw::ABWCollector::closeSpan()
{
  if (m_iface)
    m_iface->closeSpan();
}

void libabw::ABWCollector::insertLineBreak()
{
  if (m_iface)
    m_iface->insertLineBreak();
}

void libabw::ABWCollector::insertColumnBreak()
{
}

void libabw::ABWCollector::insertPageBreak()
{
}

void libabw::ABWCollector::insertText(const librevenge::RVNGString &text)
{
  if (text.len() <= 0)
    return;

  librevenge::RVNGString tmpText;
  const char ASCII_SPACE = 0x0020;

  int numConsecutiveSpaces = 0;
  librevenge::RVNGString::Iter i(text);
  for (i.rewind(); i.next();)
  {
    if (*(i()) == ASCII_SPACE)
      numConsecutiveSpaces++;
    else
      numConsecutiveSpaces = 0;

    if (numConsecutiveSpaces > 1)
    {
      if (tmpText.len() > 0)
      {
        if (m_iface)
          m_iface->insertText(tmpText);
        tmpText.clear();
      }

      if (m_iface)
        m_iface->insertSpace();
    }
    else
    {
      tmpText.append(i());
    }
  }

  if (m_iface)
    m_iface->insertText(tmpText);
}



/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
