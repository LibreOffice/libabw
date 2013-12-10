/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ABWCOLLECTOR_H__
#define __ABWCOLLECTOR_H__

#include <librevenge/librevenge.h>

namespace libabw
{

struct ABWData
{
  ABWData()
    : m_mimeType(), m_binaryData() {}
  ABWData(const ABWData &data)
    : m_mimeType(data.m_mimeType), m_binaryData(data.m_binaryData) {}
  ABWData(const librevenge::RVNGString &mimeType, const librevenge::RVNGBinaryData binaryData)
    : m_mimeType(mimeType), m_binaryData(binaryData) {}
  ~ABWData() {}

  librevenge::RVNGString m_mimeType;
  librevenge::RVNGBinaryData m_binaryData;
};

class ABWCollector
{
public:
  ABWCollector() {}
  virtual ~ABWCollector() {}

  // collector functions

  virtual void collectTextStyle(const char *name, const char *basedon, const char *followedby, const char *props) = 0;
  virtual void collectParagraphProperties(const char *style, const char *props) = 0;
  virtual void collectSectionProperties(const char *id, const char *type, const char *header, const char *footer, const char *props) = 0;
  virtual void collectCharacterProperties(const char *style, const char *props) = 0;
  virtual void collectPageSize(const char *width, const char *height, const char *units, const char *pageScale) = 0;
  virtual void closeParagraph() = 0;
  virtual void closeSpan() = 0;
  virtual void openLink(const char *href) = 0;
  virtual void closeLink() = 0;
  virtual void openFoot(const char *id) = 0;
  virtual void closeFoot() = 0;
  virtual void openEndnote(const char *id) = 0;
  virtual void closeEndnote() = 0;
  virtual void endSection() = 0;
  virtual void startDocument() = 0;
  virtual void endDocument() = 0;
  virtual void insertLineBreak() = 0;
  virtual void insertColumnBreak() = 0;
  virtual void insertPageBreak() = 0;
  virtual void insertText(const librevenge::RVNGString &text) = 0;
  virtual void insertImage(const char *dataid, const char *props) = 0;

  virtual void collectData(const char *name, const char *mimeType, const librevenge::RVNGBinaryData &data) = 0;

  virtual void openTable(const char *props) = 0;
  virtual void closeTable() = 0;
  virtual void openCell(const char *props) = 0;
  virtual void closeCell() = 0;
};

} // namespace libabw

#endif /* __ABWCOLLECTOR_H__ */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
