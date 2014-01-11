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

#include <string>
#include <map>
#include <libwpd/libwpd.h>

namespace libabw
{

enum ABWUnit
{
  ABW_NONE,
  ABW_CM,
  ABW_IN,
  ABW_MM,
  ABW_PI,
  ABW_PT,
  ABW_PX,
  ABW_PERCENT
};

enum ABWListType
{
  ABW_ORDERED,
  ABW_UNORDERED
};

bool findInt(const std::string &str, int &res);
bool findDouble(const std::string &str, double &res, ABWUnit &unit);
void parsePropString(const std::string &str, std::map<std::string, std::string> &props);

struct ABWData
{
  ABWData()
    : m_mimeType(), m_binaryData() {}
  ABWData(const ABWData &data)
    : m_mimeType(data.m_mimeType), m_binaryData(data.m_binaryData) {}
  ABWData(const WPXString &mimeType, const WPXBinaryData binaryData)
    : m_mimeType(mimeType), m_binaryData(binaryData) {}
  ~ABWData() {}

  WPXString m_mimeType;
  WPXBinaryData m_binaryData;
};

struct ABWListElement
{
  ABWListElement()
    : m_listLevel(-1), m_minLabelWidth(0.0), m_spaceBefore(0.0), m_parentId() {}
  virtual ~ABWListElement() {}
  virtual void writeOut(WPXPropertyList &propList) const;
  virtual ABWListType getType() const = 0;

  int m_listLevel;
  double m_minLabelWidth;
  double m_spaceBefore;
  std::string m_parentId;
};

struct ABWOrderedListElement : public ABWListElement
{
  ABWOrderedListElement()
    : ABWListElement(), m_numFormat(), m_numPrefix(), m_numSuffix(), m_startValue(-1) {}
  ~ABWOrderedListElement() {}
  void writeOut(WPXPropertyList &propList) const;
  ABWListType getType() const
  {
    return ABW_ORDERED;
  }

  WPXString m_numFormat;
  WPXString m_numPrefix;
  WPXString m_numSuffix;
  int m_startValue;
};

struct ABWUnorderedListElement : public ABWListElement
{
  ABWUnorderedListElement()
    : ABWListElement(), m_bulletChar() {}
  ~ABWUnorderedListElement() {}
  void writeOut(WPXPropertyList &propList) const;
  ABWListType getType() const
  {
    return ABW_UNORDERED;
  }

  WPXString m_bulletChar;
};

class ABWCollector
{
public:
  ABWCollector() {}
  virtual ~ABWCollector() {}

  // collector functions

  virtual void collectTextStyle(const char *name, const char *basedon, const char *followedby, const char *props) = 0;
  virtual void collectParagraphProperties(const char *level, const char *listid, const char *parentid,
                                          const char *style, const char *props) = 0;
  virtual void collectSectionProperties(const char *footer, const char *footerLeft, const char *footerFirst,
                                        const char *footerLast, const char *header, const char *headerLeft,
                                        const char *headerFirst, const char *headerLast, const char *props) = 0;
  virtual void collectCharacterProperties(const char *style, const char *props) = 0;
  virtual void collectPageSize(const char *width, const char *height, const char *units, const char *pageScale) = 0;
  virtual void closeParagraphOrListElement() = 0;
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
  virtual void insertText(const WPXString &text) = 0;
  virtual void insertImage(const char *dataid, const char *props) = 0;
  virtual void collectList(const char *id, const char *listDecimal, const char *listDelim,
                           const char *parentid, const char *startValue, const char *type) = 0;

  virtual void collectData(const char *name, const char *mimeType, const WPXBinaryData &data) = 0;
  virtual void collectHeaderFooter(const char *id, const char *type) = 0;

  virtual void openTable(const char *props) = 0;
  virtual void closeTable() = 0;
  virtual void openCell(const char *props) = 0;
  virtual void closeCell() = 0;
};

} // namespace libabw

#endif /* __ABWCOLLECTOR_H__ */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
