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
  : m_input(input), m_iface(iface), m_collector(0), m_inParagraph(false)
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
  int emptyToken = xmlTextReaderIsEmptyElement(reader);
  if (XML_READER_TYPE_TEXT == tokenType)
  {
    librevenge::RVNGString text((const char *)xmlTextReaderConstValue(reader));
    ABW_DEBUG_MSG(("ABWParser::processXmlNode: text %s\n", text.cstr()));
    if (m_inParagraph && m_collector)
      m_collector->insertText(text);
  }
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
  case XML_S:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readS(reader);
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
    if (XML_READER_TYPE_END_ELEMENT == tokenType || emptyToken > 0)
      if (m_collector)
        m_collector->endSection();
    break;
  case XML_DATA:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readData(reader);
    break;
  case XML_P:
    if (XML_READER_TYPE_ELEMENT == tokenType)
    {
      readP(reader);
      m_inParagraph = true;
    }
    if (XML_READER_TYPE_END_ELEMENT == tokenType || emptyToken > 0)
    {
      m_inParagraph = false;
      if (m_collector)
        m_collector->closeParagraph();
    }
    break;
  case XML_C:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readC(reader);
    if (XML_READER_TYPE_END_ELEMENT == tokenType || emptyToken > 0)
      if (m_collector)
        m_collector->closeSpan();
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
  case XML_A:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readA(reader);
    if (XML_READER_TYPE_END_ELEMENT == tokenType || emptyToken > 0)
      m_collector->closeLink();
    break;
  case XML_FOOT:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readFoot(reader);
    if (XML_READER_TYPE_END_ELEMENT == tokenType || emptyToken > 0)
      m_collector->closeFoot();
    break;
  case XML_ENDNOTE:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readEndnote(reader);
    if (XML_READER_TYPE_END_ELEMENT == tokenType || emptyToken > 0)
      m_collector->closeEndnote();
    break;
  case XML_TABLE:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readTable(reader);
    if (XML_READER_TYPE_END_ELEMENT == tokenType || emptyToken > 0)
      m_collector->closeTable();
    break;
  case XML_CELL:
    if (XML_READER_TYPE_ELEMENT == tokenType)
      readCell(reader);
    if (XML_READER_TYPE_END_ELEMENT == tokenType || emptyToken > 0)
      m_collector->closeCell();
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
    default:
      break;
    }
  }
  while ((XML_IGNOREDWORDS != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
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
}

void libabw::ABWParser::readSection(xmlTextReaderPtr reader)
{
  xmlChar *props = xmlTextReaderGetAttribute(reader, BAD_CAST("props"));
  if (m_collector)
    m_collector->collectSectionProperties((const char *)props);
  if (props)
    xmlFree(props);
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
    default:
      break;
    }
  }
  while ((XML_DATA != tokenId || XML_READER_TYPE_END_ELEMENT != tokenType) && 1 == ret);
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
      case 'C':
        m_collector->collectTextStyle((const char *)name, (const char *)basedon, (const char *)followedby, (const char *)props);
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
}

void libabw::ABWParser::readA(xmlTextReaderPtr reader)
{
  xmlChar *href = xmlTextReaderGetAttribute(reader, BAD_CAST("xlink:href"));
  if (m_collector)
    m_collector->openLink((const char *)href);
  if (href)
    xmlFree(href);
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

}

void libabw::ABWParser::readEndnote(xmlTextReaderPtr reader)
{
  xmlChar *id = xmlTextReaderGetAttribute(reader, BAD_CAST("endnote-id"));
  if (m_collector)
    m_collector->openEndnote((const char *)id);
  if (id)
    xmlFree(id);
}

void libabw::ABWParser::readFoot(xmlTextReaderPtr reader)
{
  xmlChar *id = xmlTextReaderGetAttribute(reader, BAD_CAST("footnote-id"));
  if (m_collector)
    m_collector->openFoot((const char *)id);
  if (id)
    xmlFree(id);
}

void libabw::ABWParser::readTable(xmlTextReaderPtr reader)
{
  xmlChar *props = xmlTextReaderGetAttribute(reader, BAD_CAST("props"));
  if (m_collector)
    m_collector->openTable((const char *)props);
  if (props)
    xmlFree(props);
}

void libabw::ABWParser::readCell(xmlTextReaderPtr reader)
{
  xmlChar *props = xmlTextReaderGetAttribute(reader, BAD_CAST("props"));
  if (m_collector)
    m_collector->openCell((const char *)props);
  if (props)
    xmlFree(props);
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
