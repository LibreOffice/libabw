/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ABWCONTENTCOLLECTOR_H__
#define __ABWCONTENTCOLLECTOR_H__

#include <map>
#include <vector>
#include <stack>
#include <set>
#include <string>
#include <librevenge/librevenge.h>
#include "ABWCollector.h"

namespace libabw
{

struct ABWStyle
{
  ABWStyle() : basedon(), followedby(), properties() {}
  ~ABWStyle() {}
  std::string basedon;
  std::string followedby;
  std::map<std::string, std::string> properties;
};

struct ABWTableState
{
  ABWTableState();
  ABWTableState(const ABWTableState &ps);
  ~ABWTableState();

  std::map<std::string, std::string> m_currentTableProperties;
  std::map<std::string, std::string> m_currentCellProperties;

  int m_currentTableCol;
  int m_currentTableRow;
  int m_currentTableCellNumberInRow;
  bool m_isTableRowOpened;
  bool m_isTableColumnOpened;
  bool m_isTableCellOpened;
  bool m_isCellWithoutParagraph;
  bool m_isRowWithoutCell;
};

struct ABWParsingState
{
  ABWParsingState();
  ABWParsingState(const ABWParsingState &ps);
  ~ABWParsingState();

  bool m_isDocumentStarted;
  bool m_isPageSpanOpened;
  bool m_isSectionOpened;

  bool m_isSpanOpened;
  bool m_isParagraphOpened;

  std::map<std::string, std::string> m_currentSectionStyle;
  std::map<std::string, std::string> m_currentParagraphStyle;
  std::map<std::string, std::string> m_currentCharacterStyle;

  double m_pageWidth;
  double m_pageHeight;
  double m_pageMarginTop;
  double m_pageMarginBottom;
  double m_pageMarginLeft;
  double m_pageMarginRight;

  bool m_deferredPageBreak;
  bool m_deferredColumnBreak;

  bool m_isNote;

  std::stack<ABWTableState> m_tableStates;
};

class ABWContentCollector : public ABWCollector
{
public:
  ABWContentCollector(::librevenge::RVNGTextInterface *iface);
  virtual ~ABWContentCollector();

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

  void openTable(const char *props);
  void closeTable();
  void openCell(const char *props);
  void closeCell();


private:
  ABWContentCollector(const ABWContentCollector &);
  ABWContentCollector &operator=(const ABWContentCollector &);

  void _openPageSpan();
  void _closePageSpan();

  void _openSection();
  void _closeSection();

  void _openParagraph();
  void _closeParagraph();

  void _openListElement();
  void _closeListElement();

  void _openSpan();
  void _closeSpan();

  void _openTable();
  void _closeTable();
  void _openTableRow();
  void _closeTableRow();
  void _openTableCell();
  void _closeTableCell();

  void _recurseTextProperties(const char *name, std::map<std::string, std::string> &styleProps);
  std::string _findParagraphProperty(const char *name);
  std::string _findCharacterProperty(const char *name);
  std::string _findTableProperty(const char *name);
  std::string _findCellProperty(const char *name);

  ABWParsingState *m_ps;
  librevenge::RVNGTextInterface *m_iface;
  std::stack<ABWParsingState *> m_parsingStates;
  std::set<std::string> m_dontLoop;
  std::map<std::string, ABWStyle> m_textStyles;
};

} // namespace libabw

#endif /* __ABWCOLLECTOR_H__ */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
