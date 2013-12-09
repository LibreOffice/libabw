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

#include <map>
#include <stack>
#include <string>
#include <librevenge/librevenge.h>
#include "ABWCollector.h"

namespace libabw
{

struct ABWStylesTableState
{
  ABWStylesTableState();
  ABWStylesTableState(const ABWStylesTableState &ts);
  ~ABWStylesTableState();

  std::map<std::string, std::string> m_currentCellProperties;

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
  ABWStylesCollector(std::map<int, int> &tableSizes, std::map<std::string, ABWData> &data);
  virtual ~ABWStylesCollector();

  // collector functions

  void collectTextStyle(const char *name, const char *basedon, const char *followedby, const char *props);
  void collectParagraphProperties(const char *style, const char *props);
  void collectSectionProperties(const char *props);
  void collectCharacterProperties(const char *style, const char *props);
  void collectPageSize(const char *width, const char *height, const char *units, const char *pageScale);
  void closeParagraph();
  void closeSpan();
  void openLink(const char *href);
  void closeLink();
  void openFoot(const char *id);
  void closeFoot();
  void openEndnote(const char *id);
  void closeEndnote();
  void endSection();
  void startDocument();
  void endDocument();
  void insertLineBreak();
  void insertColumnBreak();
  void insertPageBreak();
  void insertText(const librevenge::RVNGString &text);
  void insertImage(const char *dataid, const char *props);

  void collectData(const char *name, const char *mimeType, const librevenge::RVNGBinaryData &data);

  void openTable(const char *props);
  void closeTable();
  void openCell(const char *props);
  void closeCell();


private:
  ABWStylesCollector(const ABWStylesCollector &);
  ABWStylesCollector &operator=(const ABWStylesCollector &);

  std::string _findCellProperty(const char *name);

  ABWStylesParsingState *m_ps;
  std::map<int, int> &m_tableSizes;
  std::map<std::string, ABWData> &m_data;
  int m_tableCounter;
};

} // namespace libabw

#endif /* __ABWSTYLESCOLLECTOR_H__ */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
