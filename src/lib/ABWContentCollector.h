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

#include <memory>
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
  ABW_FOOTER,
  ABW_FRAME_IMAGE,
  ABW_FRAME_TEXTBOX,
  ABW_FRAME_UNKNOWN
};

struct ABWStyle
{
  ABWStyle() : basedon(), followedby(), properties() {}
  ~ABWStyle() {}
  std::string basedon;
  std::string followedby;
  ABWPropertyMap properties;
};

struct ABWContentTableState
{
  ABWContentTableState();
  ABWContentTableState(const ABWContentTableState &ts);
  ~ABWContentTableState();

  ABWPropertyMap m_currentTableProperties;
  ABWPropertyMap m_currentCellProperties;

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

  bool m_isPageFrame;

  bool m_isSpanOpened;
  bool m_isParagraphOpened;
  bool m_isListElementOpened;
  bool m_inParagraphOrListElement;

  ABWPropertyMap m_currentSectionStyle;
  ABWPropertyMap m_currentParagraphStyle;
  ABWPropertyMap m_currentCharacterStyle;

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
  int m_currentListId;
  bool m_isFirstTextInListElement;

  std::stack<ABWContentTableState> m_tableStates;
  std::stack<std::pair<int, std::shared_ptr<ABWListElement>>> m_listLevels;
};

class ABWContentCollector : public ABWCollector
{
public:
  ABWContentCollector(librevenge::RVNGTextInterface *iface, const std::map<int, int> &tableSizes,
                      const std::map<std::string, ABWData> &data,
                      const std::map<int, std::shared_ptr<ABWListElement>> &listElements);
  ~ABWContentCollector() override;

  // collector functions

  void collectTextStyle(const char *name, const char *basedon, const char *followedby, const char *props) override;
  void collectDocumentProperties(const char *props) override;
  void collectParagraphProperties(const char *level, const char *listid, const char *parentid, const char *style, const char *props) override;
  void collectSectionProperties(const char *footer, const char *footerLeft, const char *footerFirst, const char *footerLast,
                                const char *header, const char *headerLeft, const char *headerFirst, const char *headerLast,
                                const char *props) override;
  void collectCharacterProperties(const char *style, const char *props) override;
  void collectPageSize(const char *width, const char *height, const char *units, const char *pageScale) override;
  void closeParagraphOrListElement() override;
  void closeSpan() override;
  void openLink(const char *href) override;
  void closeLink() override;
  void openFoot(const char *id) override;
  void closeFoot() override;
  void openEndnote(const char *id) override;
  void closeEndnote() override;
  void openField(const char *type, const char *id) override;
  void closeField() override;
  void endSection() override;
  void startDocument() override;
  void endDocument() override;
  void insertLineBreak() override;
  void insertColumnBreak() override;
  void insertPageBreak() override;
  void insertText(const char *text) override;
  void insertImage(const char *dataid, const char *props) override;
  void collectList(const char *, const char *, const char *, const char *, const char *, const char *) override {}

  void collectData(const char *name, const char *mimeType, const librevenge::RVNGBinaryData &data) override;
  void collectHeaderFooter(const char *id, const char *type) override;

  void openTable(const char *props) override;
  void closeTable() override;
  void openCell(const char *props) override;
  void closeCell() override;

  void openFrame(const char *props, const char *imageId, const char *title, const char *alt) override;
  void closeFrame(ABWOutputElements *(&elements), bool &pageFrame) override;
  void addFrameElements(ABWOutputElements &elements, bool pageFrame) override;

  void addMetadataEntry(const char *name, const char *value) override;

private:
  ABWContentCollector(const ABWContentCollector &);
  ABWContentCollector &operator=(const ABWContentCollector &);

  void _setMetadata();

  void _addBorderProperties(const std::map<std::string, std::string> &map, librevenge::RVNGPropertyList &propList, const std::string &defaultUndefBorderProp="");

  void _openPageSpan();
  void _closePageSpan();

  void _openSection();
  void _closeSection();

  //! open a paragraph or a list element (depend on m_currentListLevel)
  void _openBlock();
  //! close the current paragraph or list element
  void _closeBlock();

  void _openParagraph();
  void _closeParagraph();

  void _openListElement();
  void _closeListElement();

  void _handleListChange();
  void _changeList();
  void _recurseListLevels(int oldLevel, int newLevel, int listId);
  void _writeOutDummyListLevels(int oldLevel, int newLevel);

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

  void _recurseTextProperties(const char *name, ABWPropertyMap &styleProps);
  std::string _findDocumentProperty(const char *name);
  std::string _findParagraphProperty(const char *name);
  std::string _findCharacterProperty(const char *name);
  std::string _findTableProperty(const char *name);
  std::string _findCellProperty(const char *name);
  std::string _findSectionProperty(const char *name);
  std::string _findMetadataEntry(const char *name);

  void _fillParagraphProperties(librevenge::RVNGPropertyList &propList, bool isListElement);
  bool _convertFieldDTFormat(std::string const &dtFormat, librevenge::RVNGPropertyListVector &propVect);

  int getCellPos(const char *startProp, const char *endProp, int defStart);

  std::shared_ptr<ABWContentParsingState> m_ps;
  librevenge::RVNGTextInterface *m_iface;
  std::stack<std::shared_ptr<ABWContentParsingState> > m_parsingStates;
  std::set<std::string> m_dontLoop;
  std::map<std::string, ABWStyle> m_textStyles;

  ABWPropertyMap m_documentStyle;
  ABWPropertyMap m_metadata;

  const std::map<std::string, ABWData> &m_data;
  const std::map<int, int> &m_tableSizes;
  int m_tableCounter;
  ABWOutputElements m_outputElements;
  ABWOutputElements m_pageOutputElements;
  const std::map<int, std::shared_ptr<ABWListElement>> &m_listElements;
  std::vector<std::shared_ptr<ABWListElement>> m_dummyListElements;
};

} // namespace libabw

#endif /* __ABWCOLLECTOR_H__ */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
