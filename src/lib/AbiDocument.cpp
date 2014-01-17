/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <libabw/libabw.h>
#include "ABWXMLHelper.h"
#include "ABWParser.h"
#include "ABWZlibStream.h"
#include "libabw_internal.h"

/**
\mainpage libabw documentation
This document contains both the libabw API specification and the normal libabw
documentation.
\section api_docs libabw API documentation
The external libabw API is provided by the AbiDocument class. This class, combined
with the librevenge::RVNGTextInterface class, are the only two classes that will be of interest
for the application programmer using libabw.
\section lib_docs libabw documentation
If you are interested in the structure of libabw itself, this whole document
would be a good starting point for exploring the internals of libabw. Mind that
this document is a work-in-progress, and will most likely not cover libabw for
the full 100%.
*/

/**
Analyzes the content of an input stream to see if it can be parsed
\param input The input stream
\return A confidence value which represents the likelihood that the content from
the input stream can be parsed
*/
ABWAPI bool libabw::AbiDocument::isFileFormatSupported(librevenge::RVNGInputStream *input)
{
  ABW_DEBUG_MSG(("AbiDocument::isFileFormatSupported\n"));
  if (!input)
    return false;
  input->seek(0, librevenge::RVNG_SEEK_SET);
  libabw::ABWZlibStream stream(input);
  xmlTextReaderPtr reader = 0;
  try
  {
    stream.seek(0, librevenge::RVNG_SEEK_SET);
    reader = libabw::xmlReaderForStream(&stream);
    if (!reader)
      return false;
    int ret = xmlTextReaderRead(reader);
    while (ret == 1 && 1 != xmlTextReaderNodeType(reader))
      ret = xmlTextReaderRead(reader);
    if (ret != 1)
    {
      xmlFreeTextReader(reader);
      return false;
    }
    const xmlChar *name = xmlTextReaderConstName(reader);
    if (!name)
    {
      xmlFreeTextReader(reader);
      return false;
    }
    if (!xmlStrEqual(name, BAD_CAST("abiword")))
    {
      if (!xmlStrEqual(name, BAD_CAST("awml")))
      {
        xmlFreeTextReader(reader);
        return false;
      }
    }

    // Checking the namespace of AbiWord documents.
    const xmlChar *nsname = xmlTextReaderConstNamespaceUri(reader);
    if (!nsname)
    {
      xmlFreeTextReader(reader);
#if 1
      return true; // Have seen some abiword files without NS declaration
#else
      return false;
#endif
    }
    if (!xmlStrEqual(nsname, BAD_CAST("http://www.abisource.com/awml.dtd")))
    {
      xmlFreeTextReader(reader);
      return false;
    }

    xmlFreeTextReader(reader);
    return true;
  }
  catch (...)
  {
    if (reader)
      xmlFreeTextReader(reader);
    return false;
  }
}

/**
Parses the input stream content. It will make callbacks to the functions provided by a
librevenge::RVNGTextInterface class implementation when needed. This is often commonly called the
'main parsing routine'.
\param input The input stream
\param textInterface A librevenge::RVNGTextInterface implementation
\param password The password used to protect the document or NULL if the document
is not protected
\return A value that indicates whether the conversion was successful and in case it
was not, it indicates the reason of the error
*/
ABWAPI bool libabw::AbiDocument::parse(librevenge::RVNGInputStream *input, librevenge::RVNGTextInterface *textInterface)
{
  ABW_DEBUG_MSG(("AbiDocument::parse\n"));
  if (!input)
    return false;
  input->seek(0, librevenge::RVNG_SEEK_SET);
  libabw::ABWZlibStream stream(input);
  libabw::ABWParser parser(&stream, textInterface);
  if (parser.parse())
    return true;
  return false;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
