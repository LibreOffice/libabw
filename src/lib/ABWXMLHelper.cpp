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
#include <libwpd-stream/libwpd-stream.h>
#include "ABWXMLHelper.h"
#include "libabw_internal.h"

namespace
{

extern "C" {

  static int abwxmlInputCloseFunc(void *)
  {
    return 0;
  }

  static int abwxmlInputReadFunc(void *context, char *buffer, int len)
  {
    WPXInputStream *input = (WPXInputStream *)context;

    if ((!input) || (!buffer) || (len < 0))
      return -1;

    if (input->atEOS())
      return 0;

    unsigned long tmpNumBytesRead = 0;
    const unsigned char *tmpBuffer = input->read(len, tmpNumBytesRead);

    if (tmpBuffer && tmpNumBytesRead)
      memcpy(buffer, tmpBuffer, tmpNumBytesRead);
    return tmpNumBytesRead;
  }

#ifdef DEBUG
  static void abwxmlReaderErrorFunc(void *, const char *message, xmlParserSeverities severity, xmlTextReaderLocatorPtr)
#else
  static void abwxmlReaderErrorFunc(void *, const char *, xmlParserSeverities severity, xmlTextReaderLocatorPtr)
#endif
  {
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
      break;
    default:
      break;
    }
  }

} // extern "C"

} // anonymous namespace

// xmlTextReader helper function

xmlTextReaderPtr libabw::xmlReaderForStream(WPXInputStream *input, const char *URL, const char *encoding, int options)
{
  xmlTextReaderPtr reader = xmlReaderForIO(abwxmlInputReadFunc, abwxmlInputCloseFunc, (void *)input, URL, encoding, options);
  xmlTextReaderSetErrorHandler(reader, abwxmlReaderErrorFunc, 0);
  return reader;
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
