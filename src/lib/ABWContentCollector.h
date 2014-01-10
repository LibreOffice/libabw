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

#include <vector>
#include <stack>
#include <set>
#include <librevenge/librevenge.h>
#include "ABWOutputElements.h"
#include "ABWCollector.h"

namespace libabw
{

enum ABWContext
{
  ABW_SECTION,
  ABW_HEADER,
  ABW_FOOTER
};

struct ABWStyle
{
  ABWStyle() : basedon(), followedby(), properties() {}
  ~ABWStyle() {}
  std::string basedon;
  std::string followedby;
  std::map<std::string, std::string> properties;
};

struct ABWContentTableState
{
  ABWContentTableState();
  ABWContentTableState(const ABWContentTableState &ts);
  ~ABWContentTableState();

  std::map<std::string, std::string> m_currentTableProperties;
  std::map<std::string, std::string> m_currentCellProperties;

  int m_currentTableCol;
  int m_currentTableRow;
  int m_currentTableCellNumberInRow;
  int m_currentTableId;
  bool m_isTableRowOpened;
  bool m_isTableColumnOpened;
  bool m_isTableCellOpened;
  bool m_isCellWithoutParagraph;
  bool m_isRowWithoutCell;
};

struct ABWContentParsingState
{
  ABWContentParsingState();
  ABWContentParsingState(const ABWContentParsingState &ps);
  ~ABWContentParsingState();

  bool m_isDocumentStarted;
  bool m_isPageSpanOpened;
  bool m_isSectionOpened;
  bool m_isHeaderOpened;
  bool m_isFooterOpened;

  bool m_isSpanOpened;
  bool m_isParagraphOpened;
  bool m_isListElementOpened;

  std::map<std::string, std::string> m_currentSectionStyle;
  std::map<std::string, std::string> m_currentParagraphStyle;
  std::map<std::string, std::string> m_currentCharacterStyle;

  double m_pageWidth;
  double m_pageHeight;
  double m_pageMarginTop;
  double m_pageMarginBottom;
  double m_pageMarginLeft;
  double m_pageMarginRight;
  int m_footerId;
  int m_footerLeftId;
  int m_footerFirstId;
  int m_footerLastId;
  int m_headerId;
  int m_headerLeftId;
  int m_headerFirstId;
  int m_headerLastId;
  int m_currentHeaderFooterId;
  librevenge::RVNGString m_currentHeaderFooterOccurrence;
  ABWContext m_parsingContext;

  bool m_deferredPageBreak;
  bool m_deferredColumnBreak;

  bool m_isNote;

  int m_currentListLevel;
  librevenge::RVNGString m_currentListId;

  std::stack<ABWContentTableState> m_tableStates;
  std::stack<std::pair<int, ABWListElement *> > m_listLevels;
};

class ABWContentCollector : public ABWCollector
{
public:
  ABWContentCollector(librevenge::RVNGTextInterface *iface, const std::map<int, int> &tableSizes,
                      const std::map<std::string, ABWData> &data,
                      const std::map<librevenge::RVNGString, ABWListElement *> &listElements);
  virtual ~ABWContentCollector();

  // collector functions

  void collectTextStyle(const char *name, const char *basedon, const char *followedby, const char *props);
  void collectParagraphProperties(const char *level, const char *listid, const char *parentid, const char *style, const char *props);
  void collectSectionProperties(const char *footer, const char *footerLeft, const char *footerFirst, const char *footerLast,
                                const char *header, const char *headerLeft, const char *headerFirst, const char *headerLast,
                                const char *props);
  void collectCharacterProperties(const char *style, const char *props);
  void collectPageSize(const char *width, const char *height, const char *units, const char *pageScale);
  void closeParagraphOrListElement();
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
  void collectList(const char *, const char *, const char *, const char *, const char *, const char *) {}

  void collectData(const char *name, const char *mimeType, const librevenge::RVNGBinaryData &data);
  void collectHeaderFooter(const char *id, const char *type);

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

  void _handleListChange();
  void _changeList();
  void _recurseListLevels(int oldLevel, int newLevel, const librevenge::RVNGString &listId);

  void _openSpan();
  void _closeSpan();

  void _openTable();
  void _closeTable();
  void _openTableRow();
  void _closeTableRow();
  void _openTableCell();
  void _closeTableCell();

  void _openHeader();
  void _closeHeader();
  void _openFooter();
  void _closeFooter();

  void _recurseTextProperties(const char *name, std::map<std::string, std::string> &styleProps);
  std::string _findParagraphProperty(const char *name);
  std::string _findCharacterProperty(const char *name);
  std::string _findTableProperty(const char *name);
  std::string _findCellProperty(const char *name);
  std::string _findSectionProperty(const char *name);

  void _fillParagraphProperties(librevenge::RVNGPropertyList &propList);

  ABWContentParsingState *m_ps;
  librevenge::RVNGTextInterface *m_iface;
  std::stack<ABWContentParsingState *> m_parsingStates;
  std::set<std::string> m_dontLoop;
  std::map<std::string, ABWStyle> m_textStyles;

  const std::map<std::string, ABWData> &m_data;
  const std::map<int, int> &m_tableSizes;
  int m_tableCounter;
  ABWOutputElements m_outputElements;
  const std::map<librevenge::RVNGString, ABWListElement *> &m_listElements;
};

} // namespace libabw

#endif /* __ABWCOLLECTOR_H__ */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
