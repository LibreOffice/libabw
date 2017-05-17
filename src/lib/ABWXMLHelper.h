/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ABWXMLHELPER_H__
#define __ABWXMLHELPER_H__

#include <memory>

#include <librevenge-stream/librevenge-stream.h>

#include <libxml/xmlreader.h>
#include <libxml/xmlstring.h>

namespace libabw
{

// An exception-safe wrapper around xmlChar *
class ABWXMLString
{
public:
  ABWXMLString(xmlChar *xml);

  const xmlChar *get() const;

  operator const char *() const;

private:
  std::shared_ptr<xmlChar> m_xml;
};

class ABWXMLProgressWatcher
{
  ABWXMLProgressWatcher(const ABWXMLProgressWatcher &) = delete;
  ABWXMLProgressWatcher &operator=(const ABWXMLProgressWatcher &) = delete;

public:
  ABWXMLProgressWatcher();

  void setReader(xmlTextReaderPtr reader);

  bool isStuck() const;
  void signalError();

private:
  xmlTextReaderPtr m_reader;
  int m_line;
  int m_col;
  bool m_wasError;
  bool m_isStuck;
};

// create an xmlTextReader pointer from a librevenge::RVNGInputStream pointer
std::unique_ptr<xmlTextReader, void(*)(xmlTextReaderPtr)> xmlReaderForStream(librevenge::RVNGInputStream *input, ABWXMLProgressWatcher *watcher = 0);

} // namespace libabw

#endif // __ABWXMLHELPER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
