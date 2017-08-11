/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ABWSTYLESCOLLECTOR_H__
#define __ABWSTYLESCOLLECTOR_H__

#include <memory>
#include <stack>
#include <librevenge/librevenge.h>
#include "ABWCollector.h"

namespace libabw
{

struct ABWStylesTableState
{
  ABWStylesTableState();
  ABWStylesTableState(const ABWStylesTableState &ts);
  ~ABWStylesTableState();

  ABWPropertyMap m_currentCellProperties;

  int m_currentTableWidth;
  int m_currentTableRow;
  int m_currentTableId;
};

struct ABWStylesParsingState
{
  ABWStylesParsingState();
  ABWStylesParsingState(const ABWStylesParsingState &ps);
  ~ABWStylesParsingState();

  std::stack<ABWStylesTableState> m_tableStates;
};

class ABWStylesCollector : public ABWCollector
{
public:
  ABWStylesCollector(std::map<int, int> &tableSizes,
                     std::map<std::string, ABWData> &data,
                     std::map<int, std::shared_ptr<ABWListElement>> &listElements);
  ~ABWStylesCollector() override;

  // collector functions

  void collectTextStyle(const char *, const char *, const char *, const char *) override {}
  void collectDocumentProperties(const char *) override {}
  void collectParagraphProperties(const char *level, const char *listid, const char *parentid,
                                  const char *style, const char *props) override;
  void collectSectionProperties(const char *, const char *, const char *, const char *,
                                const char *, const char *, const char *, const char *,
                                const char *) override {}
  void collectCharacterProperties(const char *, const char *) override {}
  void collectPageSize(const char *, const char *, const char *, const char *) override {}
  void closeParagraphOrListElement() override {}
  void closeSpan() override {}
  void openLink(const char *) override {}
  void closeLink() override {}
  void openFoot(const char *) override {}
  void closeFoot() override {}
  void openEndnote(const char *) override {}
  void closeEndnote() override {}
  void openField(const char *, const char *) override {}
  void closeField() override {}
  void endSection() override {}
  void startDocument() override {}
  void endDocument() override {}
  void insertLineBreak() override {}
  void insertColumnBreak() override {}
  void insertPageBreak() override {}
  void insertText(const char *) override {}
  void insertImage(const char *, const char *) override {}

  void collectData(const char *name, const char *mimeType, const librevenge::RVNGBinaryData &data) override;
  void collectHeaderFooter(const char *, const char *) override {}
  void collectList(const char *id, const char *listDecimal, const char *listDelim,
                   const char *parentid, const char *startValue, const char *type) override;

  void openTable(const char *props) override;
  void closeTable() override;
  void openCell(const char *props) override;
  void closeCell() override;

  void openFrame(const char *, const char *, const char *, const char *) override {}
  void closeFrame(ABWOutputElements *(&elements), bool &) override
  {
    elements=nullptr;
  }
  void addFrameElements(ABWOutputElements &, bool) override {}

  void addMetadataEntry(const char *, const char *) override {}

private:
  ABWStylesCollector(const ABWStylesCollector &);
  ABWStylesCollector &operator=(const ABWStylesCollector &);

  std::string _findCellProperty(const char *name);
  void _processList(int id, const char *listDelim, int parentid, int startValue, int type);

  std::unique_ptr<ABWStylesParsingState> m_ps;
  std::map<int, int> &m_tableSizes;
  std::map<std::string, ABWData> &m_data;
  int m_tableCounter;
  std::map<int, std::shared_ptr<ABWListElement>> &m_listElements;
};

} // namespace libabw

#endif /* __ABWSTYLESCOLLECTOR_H__ */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
