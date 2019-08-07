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
#include <librevenge/librevenge.h>

namespace libabw
{
class ABWOutputElements;

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

typedef std::map<std::string, std::string> ABWPropertyMap;

bool findInt(const std::string &str, int &res);
bool findDouble(const std::string &str, double &res, ABWUnit &unit);
void parsePropString(const std::string &str, ABWPropertyMap &props);

struct ABWData
{
  ABWData()
    : m_mimeType(), m_binaryData() {}
  ABWData(const ABWData &data)
    : m_mimeType(data.m_mimeType), m_binaryData(data.m_binaryData) {}
  ABWData(const librevenge::RVNGString &mimeType, const librevenge::RVNGBinaryData binaryData)
    : m_mimeType(mimeType), m_binaryData(binaryData) {}
  ~ABWData() {}
  ABWData &operator=(const ABWData &data) = default;

  librevenge::RVNGString m_mimeType;
  librevenge::RVNGBinaryData m_binaryData;
};

struct ABWListElement
{
  ABWListElement()
    : m_listLevel(-1), m_minLabelWidth(0.0), m_spaceBefore(0.0), m_parentId(), m_listId(0) {}
  virtual ~ABWListElement() {}
  virtual void writeOut(librevenge::RVNGPropertyList &propList) const;
  virtual ABWListType getType() const = 0;

  int m_listLevel;
  double m_minLabelWidth;
  double m_spaceBefore;
  int m_parentId;
  //! the list id
  int m_listId;
};

struct ABWOrderedListElement : public ABWListElement
{
  ABWOrderedListElement()
    : ABWListElement(), m_numFormat(), m_numPrefix(), m_numSuffix(), m_startValue(-1) {}
  ~ABWOrderedListElement() override {}
  void writeOut(librevenge::RVNGPropertyList &propList) const override;
  ABWListType getType() const override
  {
    return ABW_ORDERED;
  }

  librevenge::RVNGString m_numFormat;
  librevenge::RVNGString m_numPrefix;
  librevenge::RVNGString m_numSuffix;
  int m_startValue;
};

struct ABWUnorderedListElement : public ABWListElement
{
  ABWUnorderedListElement()
    : ABWListElement(), m_bulletChar() {}
  ~ABWUnorderedListElement() override {}
  void writeOut(librevenge::RVNGPropertyList &propList) const override;
  ABWListType getType() const override
  {
    return ABW_UNORDERED;
  }

  librevenge::RVNGString m_bulletChar;
};

class ABWCollector
{
public:
  ABWCollector() {}
  virtual ~ABWCollector() {}

  // collector functions

  virtual void collectTextStyle(const char *name, const char *basedon, const char *followedby, const char *props) = 0;
  virtual void collectDocumentProperties(const char *props) = 0;
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
  virtual void openField(const char *type, const char *id)= 0;
  virtual void closeField()= 0;
  virtual void endSection() = 0;
  virtual void startDocument() = 0;
  virtual void endDocument() = 0;
  virtual void insertLineBreak() = 0;
  virtual void insertColumnBreak() = 0;
  virtual void insertPageBreak() = 0;
  virtual void insertText(const char *text) = 0;
  virtual void insertImage(const char *dataid, const char *props) = 0;
  virtual void collectList(const char *id, const char *listDecimal, const char *listDelim,
                           const char *parentid, const char *startValue, const char *type) = 0;

  virtual void collectData(const char *name, const char *mimeType, const librevenge::RVNGBinaryData &data) = 0;
  virtual void collectHeaderFooter(const char *id, const char *type) = 0;

  virtual void openTable(const char *props) = 0;
  virtual void closeTable() = 0;
  virtual void openCell(const char *props) = 0;
  virtual void closeCell() = 0;

  virtual void openFrame(const char *props, const char *imageId, const char *title, const char *alt) = 0;
  virtual void closeFrame(ABWOutputElements *(&elements), bool &pageFrame) = 0;
  virtual void addFrameElements(ABWOutputElements &elements, bool pageFrame) = 0;

  virtual void addMetadataEntry(const char *name, const char *value) = 0;
};

} // namespace libabw

#endif /* __ABWCOLLECTOR_H__ */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
