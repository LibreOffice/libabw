/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include "TextDocumentGenerator.h"

// use the BELL code to represent a TAB for now
#define UCS_TAB 0x0009

TextDocumentGenerator::TextDocumentGenerator(const bool isInfo) :
  m_isInfo(isInfo)
{
}

TextDocumentGenerator::~TextDocumentGenerator()
{
}

void TextDocumentGenerator::setDocumentMetaData(const WPXPropertyList &propList)
{
  if (!m_isInfo)
    return;
  WPXPropertyList::Iter propIter(propList);
  for (propIter.rewind(); propIter.next();)
  {
    printf("%s %s\n", propIter.key(), propIter()->getStr().cstr());
  }
}

void TextDocumentGenerator::closeParagraph()
{
  if (m_isInfo)
    return;
  printf("\n");
}

void TextDocumentGenerator::insertTab()
{
  if (m_isInfo)
    return;
  printf("%c", UCS_TAB);
}

void TextDocumentGenerator::insertText(const WPXString &text)
{
  if (m_isInfo)
    return;
  printf("%s", text.cstr());
}

void TextDocumentGenerator::insertSpace()
{
  if (m_isInfo)
    return;
  printf(" ");
}

void TextDocumentGenerator::insertLineBreak()
{
  if (m_isInfo)
    return;
  printf("\n");
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
