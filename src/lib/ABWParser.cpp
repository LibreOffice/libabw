/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlstring.h>
#include <librevenge-stream/librevenge-stream.h>
#include <boost/algorithm/string.hpp>
#include "ABWParser.h"
#include "ABWCollector.h"
#include "libabw_internal.h"
#include "ABWXMLHelper.h"
#include "ABWXMLTokenMap.h"


libabw::ABWParser::ABWParser(librevenge::RVNGInputStream *input, librevenge::RVNGTextInterface *iface)
  : m_input(input), m_iface(iface), m_collector(0)
{
}

libabw::ABWParser::~ABWParser()
{
}

bool libabw::ABWParser::parse()
{
  if (!m_input)
    return false;

  try
  {
    ABWCollector collector(m_iface);
    m_collector = &collector;
    m_input->seek(0, librevenge::RVNG_SEEK_SET);
    if (!processXmlDocument(m_input))
      return false;

    return true;
  }
  catch (...)
  {
    return false;
  }
}

bool libabw::ABWParser::processXmlDocument(librevenge::RVNGInputStream *input)
{
  if (!input)
    return false;

  xmlTextReaderPtr reader = xmlReaderForStream(input, 0, 0, XML_PARSE_NOBLANKS|XML_PARSE_NOENT|XML_PARSE_NONET|XML_PARSE_RECOVER);
  if (!reader)
    return false;
  int ret = xmlTextReaderRead(reader);
  while (1 == ret)
  {
    int tokenType = xmlTextReaderNodeType(reader);
    if (XML_READER_TYPE_SIGNIFICANT_WHITESPACE != tokenType)
      processXmlNode(reader);

    ret = xmlTextReaderRead(reader);
  }
  xmlFreeTextReader(reader);

  if (m_collector)
    m_collector->endDocument();
  return true;
}

void libabw::ABWParser::processXmlNode(xmlTextReaderPtr reader)
{
  if (!reader)
    return;
  int tokenId = getElementToken(reader);
  int tokenType = xmlTextReaderNodeType(reader);
  switch (tokenId)
  {
  case XML_METADATA:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readMetadata(reader);
    break;
  case XML_HISTORY:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readHistory(reader);
    break;
  case XML_REVISIONS:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readRevisions(reader);
    break;
  case XML_IGNOREDWORDS:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readIgnoredWords(reader);
    break;
  case XML_STYLES:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readStyles(reader);
    break;
  case XML_LISTS:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readLists(reader);
    break;
  case XML_PAGESIZE:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readPageSize(reader);
    break;
  case XML_SECTION:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readSection(reader);
    break;
  case XML_DATA:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readData(reader);
    break;
  default:
    break;
  }

#ifdef DEBUG
  const xmlChar *name = xmlTextReaderConstName(reader);
  const xmlChar *value = xmlTextReaderConstValue(reader);
  int isEmptyElement = xmlTextReaderIsEmptyElement(reader);

  ABW_DEBUG_MSG(("%i %i %s", isEmptyElement, tokenType, name ? (const char *)name : ""));
  if (xmlTextReaderNodeType(reader) == 1)
  {
    while (xmlTextReaderMoveToNextAttribute(reader))
    {
      const xmlChar *name1 = xmlTextReaderConstName(reader);
      const xmlChar *value1 = xmlTextReaderConstValue(reader);
      printf(" %s=\"%s\"", name1, value1);
    }
  }

  if (!value)
    ABW_DEBUG_MSG(("\n"));
  else
  {
    ABW_DEBUG_MSG((" %s\n", value));
  }
#endif
}

int libabw::ABWParser::getElementToken(xmlTextReaderPtr reader)
{
  return ABWXMLTokenMap::getTokenId(xmlTextReaderConstName(reader));
}


void libabw::ABWParser::readMetadata(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      ABW_DEBUG_MSG(("ABWParser::readMetadata: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_M:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readM(reader);
      break;
    default:
      break;
    }
  }
  while ((XML_METADATA != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libabw::ABWParser::readHistory(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      ABW_DEBUG_MSG(("ABWParser::readHistory: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_VERSION:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readVersion(reader);
      break;
    default:
      break;
    }
  }
  while ((XML_HISTORY != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libabw::ABWParser::readRevisions(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      ABW_DEBUG_MSG(("ABWParser::readRevisions: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    (void)tokenType;
    switch (tokenId)
    {
    default:
      break;
    }
  }
  while ((XML_REVISIONS != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libabw::ABWParser::readIgnoredWords(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      ABW_DEBUG_MSG(("ABWParser::readIgnoreWords: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_IW:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readIw(reader);
      break;
    default:
      break;
    }
  }
  while ((XML_IGNOREDWORDS != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libabw::ABWParser::readStyles(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      ABW_DEBUG_MSG(("ABWParser::readStyles: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_S:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readS(reader);
      break;
    default:
      break;
    }
  }
  while ((XML_STYLES != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libabw::ABWParser::readLists(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      ABW_DEBUG_MSG(("ABWParser::readLists: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_L:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readL(reader);
      break;
    default:
      break;
    }
  }
  while ((XML_LISTS != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libabw::ABWParser::readPageSize(xmlTextReaderPtr reader)
{
  xmlChar *width = xmlTextReaderGetAttribute(reader, BAD_CAST("width"));
  xmlChar *height = xmlTextReaderGetAttribute(reader, BAD_CAST("height"));
  xmlChar *units = xmlTextReaderGetAttribute(reader, BAD_CAST("units"));
  xmlChar *pageScale = xmlTextReaderGetAttribute(reader, BAD_CAST("page-scale"));
  if (m_collector)
    m_collector->collectPageSize((const char *)width, (const char *)height, (const char *)units, (const char *)pageScale);
  if (width)
    xmlFree(width);
  if (height)
    xmlFree(height);
  if (units)
    xmlFree(units);
  if (pageScale)
    xmlFree(pageScale);

  xmlTextReaderRead(reader);
}

void libabw::ABWParser::readSection(xmlTextReaderPtr reader)
{
  xmlChar *props = xmlTextReaderGetAttribute(reader, BAD_CAST("props"));
  if (m_collector)
    m_collector->collectSectionProperties((const char *)props);
  if (props)
    xmlFree(props);

  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      ABW_DEBUG_MSG(("ABWParser::readSection: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_P:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readP(reader);
      break;
#if 0
    case XML_TABLE:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readTable(reader);
      break;
    case XML_FRAME:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readFrame(reader);
      break;
#endif
    default:
      break;
    }
  }
  while ((XML_SECTION != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (m_collector)
    m_collector->endSection();
}

void libabw::ABWParser::readData(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      ABW_DEBUG_MSG(("ABWParser::readData: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    switch (tokenId)
    {
    case XML_D:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readD(reader);
      break;
    default:
      break;
    }
  }
  while ((XML_DATA != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libabw::ABWParser::readM(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      ABW_DEBUG_MSG(("ABWParser::readM: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    (void)tokenType;
    switch (tokenId)
    {
    default:
      break;
    }
  }
  while ((XML_M != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libabw::ABWParser::readIw(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      ABW_DEBUG_MSG(("ABWParser::readIw: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    (void)tokenType;
    switch (tokenId)
    {
    default:
      break;
    }
  }
  while ((XML_IW != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

void libabw::ABWParser::readVersion(xmlTextReaderPtr reader)
{
  xmlTextReaderRead(reader);
}

void libabw::ABWParser::readS(xmlTextReaderPtr reader)
{
  xmlChar *type = xmlTextReaderGetAttribute(reader, BAD_CAST("type"));
  xmlChar *name = xmlTextReaderGetAttribute(reader, BAD_CAST("name"));
  xmlChar *basedon = xmlTextReaderGetAttribute(reader, BAD_CAST("basedon"));
  xmlChar *followedby = xmlTextReaderGetAttribute(reader, BAD_CAST("followedby"));
  xmlChar *props = xmlTextReaderGetAttribute(reader, BAD_CAST("props"));
  if (type)
  {
    if (m_collector)
    {
      switch (type[0])
      {
      case 'P':
        m_collector->collectParagraphStyle((const char *)name, (const char *)basedon, (const char *)followedby, (const char *)props);
        break;
      case 'C':
        m_collector->collectCharacterStyle((const char *)name, (const char *)basedon, (const char *)followedby, (const char *)props);
        break;
      default:
        break;
      }
    }
    xmlFree(type);
  }
  if (name)
    xmlFree(name);
  if (basedon)
    xmlFree(basedon);
  if (followedby)
    xmlFree(followedby);
  if (props)
    xmlFree(props);

  xmlTextReaderRead(reader);
}

void libabw::ABWParser::readL(xmlTextReaderPtr reader)
{
  xmlTextReaderRead(reader);
}

void libabw::ABWParser::readP(xmlTextReaderPtr reader)
{
  xmlChar *style = xmlTextReaderGetAttribute(reader, BAD_CAST("style"));
  xmlChar *props = xmlTextReaderGetAttribute(reader, BAD_CAST("props"));
  if (m_collector)
    m_collector->collectParagraphProperties((const char *)style, (const char *)props);
  if (style)
    xmlFree(style);
  if (props)
    xmlFree(props);

  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      ABW_DEBUG_MSG(("ABWParser::readP: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    if (XML_READER_TYPE_TEXT == tokenType)
    {
      librevenge::RVNGString text((const char *)xmlTextReaderConstValue(reader));
      ABW_DEBUG_MSG(("ABWParser::readP: text %s\n", text.cstr()));
      m_collector->insertText(text);
    }
    switch (tokenId)
    {
    case XML_C:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        readC(reader);
      break;
    case XML_CBR:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        m_collector->insertColumnBreak();
      break;
    case XML_PBR:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        m_collector->insertPageBreak();
      break;
    case XML_BR:
      if (XML_READER_TYPE_ELEMENT == tokenType)
        m_collector->insertLineBreak();
      break;
    default:
      break;
    }
  }
  while ((XML_P != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (m_collector)
    m_collector->closeParagraph();

}

void libabw::ABWParser::readC(xmlTextReaderPtr reader)
{
  xmlChar *style = xmlTextReaderGetAttribute(reader, BAD_CAST("style"));
  xmlChar *props = xmlTextReaderGetAttribute(reader, BAD_CAST("props"));
  if (m_collector)
    m_collector->collectCharacterProperties((const char *)style, (const char *)props);
  if (style)
    xmlFree(style);
  if (props)
    xmlFree(props);

  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      ABW_DEBUG_MSG(("ABWParser::readC: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    if (XML_READER_TYPE_TEXT == tokenType)
    {
      librevenge::RVNGString text((const char *)xmlTextReaderConstValue(reader));
      ABW_DEBUG_MSG(("ABWParser::readC: text %s\n", text.cstr()));
      if (m_collector)
        m_collector->insertText(text);
    }
    switch (tokenId)
    {
    case XML_CBR:
      if (m_collector && XML_READER_TYPE_ELEMENT == tokenType)
        m_collector->insertColumnBreak();
      break;
    case XML_PBR:
      if (m_collector && XML_READER_TYPE_ELEMENT == tokenType)
        m_collector->insertPageBreak();
      break;
    case XML_BR:
      if (m_collector && XML_READER_TYPE_ELEMENT == tokenType)
        m_collector->insertLineBreak();
      break;
    default:
      break;
    }
  }
  while ((XML_C != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
  if (m_collector)
    m_collector->closeSpan();
}

void libabw::ABWParser::readD(xmlTextReaderPtr reader)
{
  int ret = 1;
  int tokenId = XML_TOKEN_INVALID;
  int tokenType = -1;
  do
  {
    ret = xmlTextReaderRead(reader);
    tokenId = getElementToken(reader);
    if (XML_TOKEN_INVALID == tokenId)
    {
      ABW_DEBUG_MSG(("ABWParser::readD: unknown token %s\n", xmlTextReaderConstName(reader)));
    }
    tokenType = xmlTextReaderNodeType(reader);
    (void)tokenType;
    switch (tokenId)
    {
    default:
      break;
    }
  }
  while ((XML_D != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
