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
                     std::map<int, ABWListElement *> &listElements);
  virtual ~ABWStylesCollector();

  // collector functions

  void collectTextStyle(const char *, const char *, const char *, const char *) {}
  void collectDocumentProperties(const char *) {}
  void collectParagraphProperties(const char *level, const char *listid, const char *parentid,
                                  const char *style, const char *props);
  void collectSectionProperties(const char *, const char *, const char *, const char *,
                                const char *, const char *, const char *, const char *,
                                const char *) {}
  void collectCharacterProperties(const char *, const char *) {}
  void collectPageSize(const char *, const char *, const char *, const char *) {}
  void closeParagraphOrListElement() {}
  void closeSpan() {}
  void openLink(const char *) {}
  void closeLink() {}
  void openFoot(const char *) {}
  void closeFoot() {}
  void openEndnote(const char *) {}
  void closeEndnote() {}
  void endSection() {}
  void startDocument() {}
  void endDocument() {}
  void insertLineBreak() {}
  void insertColumnBreak() {}
  void insertPageBreak() {}
  void insertText(const char *) {}
  void insertImage(const char *, const char *) {}

  void collectData(const char *name, const char *mimeType, const librevenge::RVNGBinaryData &data);
  void collectHeaderFooter(const char *, const char *) {}
  void collectList(const char *id, const char *listDecimal, const char *listDelim,
                   const char *parentid, const char *startValue, const char *type);

  void openTable(const char *props);
  void closeTable();
  void openCell(const char *props);
  void closeCell();

  void addMetadataEntry(const char *, const char *) {}

private:
  ABWStylesCollector(const ABWStylesCollector &);
  ABWStylesCollector &operator=(const ABWStylesCollector &);

  std::string _findCellProperty(const char *name);
  void _processList(int id, const char *listDelim, int parentid, int startValue, int type);

  ABWStylesParsingState *m_ps;
  std::map<int, int> &m_tableSizes;
  std::map<std::string, ABWData> &m_data;
  int m_tableCounter;
  std::map<int, ABWListElement *> &m_listElements;
};

} // namespace libabw

#endif /* __ABWSTYLESCOLLECTOR_H__ */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
