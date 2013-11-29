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
  if (m_iface)
    m_iface->insertText(text);
}



/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
