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
#include <libxml/xmlmemory.h>
#include <librevenge-stream/librevenge-stream.h>
#include "ABWXMLHelper.h"
#include "libabw_internal.h"

namespace libabw
{

namespace
{

extern "C" {

  static int abwxmlInputCloseFunc(void *)
  {
    return 0;
  }

  static int abwxmlInputReadFunc(void *context, char *buffer, int len)
  {
    librevenge::RVNGInputStream *input = (librevenge::RVNGInputStream *)context;

    if ((!input) || (!buffer) || (len < 0))
      return -1;

    if (input->isEnd())
      return 0;

    unsigned long tmpNumBytesRead = 0;
    const unsigned char *tmpBuffer = input->read((unsigned long) len, tmpNumBytesRead);

    if (tmpBuffer && tmpNumBytesRead)
      memcpy(buffer, tmpBuffer, tmpNumBytesRead);
    return int(tmpNumBytesRead);
  }

#ifdef DEBUG
  static void abwxmlReaderErrorFunc(void *arg, const char *message, xmlParserSeverities severity, xmlTextReaderLocatorPtr)
#else
  static void abwxmlReaderErrorFunc(void *arg, const char *, xmlParserSeverities severity, xmlTextReaderLocatorPtr)
#endif
  {
    const auto watcher = reinterpret_cast<ABWXMLErrorWatcher *>(arg);
    switch (severity)
    {
    case XML_PARSER_SEVERITY_VALIDITY_WARNING:
      ABW_DEBUG_MSG(("Found xml parser severity validity warning %s\n", message));
      break;
    case XML_PARSER_SEVERITY_VALIDITY_ERROR:
      ABW_DEBUG_MSG(("Found xml parser severity validity error %s\n", message));
      break;
    case XML_PARSER_SEVERITY_WARNING:
      ABW_DEBUG_MSG(("Found xml parser severity warning %s\n", message));
      break;
    case XML_PARSER_SEVERITY_ERROR:
      ABW_DEBUG_MSG(("Found xml parser severity error %s\n", message));
      if (watcher)
        watcher->setError();
      break;
    default:
      break;
    }
  }

} // extern "C"

} // anonymous namespace

ABWXMLString::ABWXMLString(xmlChar *xml)
  : m_xml(xml, xmlFree)
{
}

const xmlChar *ABWXMLString::get() const
{
  return m_xml.get();
}

ABWXMLString::operator const char *() const
{
  return reinterpret_cast<const char *>(m_xml.get());
}

ABWXMLErrorWatcher::ABWXMLErrorWatcher()
  : m_error(false)
{
}

bool ABWXMLErrorWatcher::isError() const
{
  return m_error;
}

void ABWXMLErrorWatcher::setError()
{
  m_error = true;
}

// xmlTextReader helper function

xmlTextReaderPtr xmlReaderForStream(librevenge::RVNGInputStream *input, ABWXMLErrorWatcher *watcher)
{
  xmlTextReaderPtr reader = xmlReaderForIO(abwxmlInputReadFunc, abwxmlInputCloseFunc, (void *)input, 0, 0,
                                           XML_PARSE_NOBLANKS|XML_PARSE_NOENT|XML_PARSE_NONET|XML_PARSE_RECOVER);
  xmlTextReaderSetErrorHandler(reader, abwxmlReaderErrorFunc, watcher);
  return reader;
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
