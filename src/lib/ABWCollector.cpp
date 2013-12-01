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
#include "libabw_internal.h"

namespace libabw
{

namespace
{

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
    else
    {
      tmpText.append(i());
    }
  }
  if (iface && !tmpText.empty())
    iface->insertText(tmpText);
}

}

}

libabw::ABWParsingState::ABWParsingState() :
  m_isDocumentStarted(false),
  m_isPageSpanOpened(false),
  m_isSectionOpened(false),

  m_isSpanOpened(false),
  m_isParagraphOpened(false)
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

void libabw::ABWCollector::startSection()
{
  _openSection();
}

void libabw::ABWCollector::endSection()
{
  _closeSection();
}

void libabw::ABWCollector::openParagraph()
{
  _openParagraph();
}

void libabw::ABWCollector::closeParagraph()
{
  _closeParagraph();
}

void libabw::ABWCollector::openSpan()
{
  _openSpan();
}

void libabw::ABWCollector::closeSpan()
{
  _closeSpan();
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
  if (text.empty())
    return;

  if (!m_ps->m_isSpanOpened)
    _openSpan();


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
        separateTabsAndInsertText(m_iface, tmpText);
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
  separateTabsAndInsertText(m_iface, tmpText);
}


void libabw::ABWCollector::_openPageSpan()
{
  if (m_ps->m_isPageSpanOpened)
    return;

  if (!m_ps->m_isDocumentStarted)
    startDocument();

  librevenge::RVNGPropertyList propList;
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
