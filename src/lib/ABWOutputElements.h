/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ABWOUTPUTELEMENTS_H
#define ABWOUTPUTELEMENTS_H

#include <list>
#include <map>
#include <libwpd/libwpd.h>

namespace libabw
{

class ABWOutputElement;

class ABWOutputElements
{
public:
  ABWOutputElements();
  virtual ~ABWOutputElements();
  void append(const ABWOutputElements &elements);
  void write(WPXDocumentInterface *iface) const;
  void addCloseEndnote();
  void addCloseFooter();
  void addCloseFootnote();
  void addCloseFrame();
  void addCloseHeader();
  void addCloseLink();
  void addCloseListElement();
  void addCloseOrderedListLevel();
  void addClosePageSpan();
  void addCloseParagraph();
  void addCloseSection();
  void addCloseSpan();
  void addCloseTable();
  void addCloseTableCell();
  void addCloseTableRow();
  void addCloseUnorderedListLevel();
  void addInsertBinaryObject(const WPXPropertyList &propList, const WPXBinaryData &data);
  void addInsertCoveredTableCell(const WPXPropertyList &propList);
  void addInsertLineBreak();
  void addInsertSpace();
  void addInsertTab();
  void addInsertText(const WPXString &text);
  void addOpenEndnote(const WPXPropertyList &propList);
  void addOpenFooter(const WPXPropertyList &propList, int id);
  void addOpenFootnote(const WPXPropertyList &propList);
  void addOpenFrame(const WPXPropertyList &propList);
  void addOpenHeader(const WPXPropertyList &propList, int id);
  void addOpenLink(const WPXPropertyList &propList);
  void addOpenListElement(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops);
  void addOpenOrderedListLevel(const WPXPropertyList &propList);
  void addOpenPageSpan(const WPXPropertyList &propList,
                       int footer, int footerLeft, int footerFirst, int footerLast,
                       int header, int headerLeft, int headerFirst, int headerLast);
  void addOpenParagraph(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops);
  void addOpenSection(const WPXPropertyList &propList, const WPXPropertyListVector &columns);
  void addOpenSpan(const WPXPropertyList &propList);
  void addOpenTable(const WPXPropertyList &propList, const WPXPropertyListVector &columns);
  void addOpenTableCell(const WPXPropertyList &propList);
  void addOpenTableRow(const WPXPropertyList &propList);
  void addOpenUnorderedListLevel(const WPXPropertyList &propList);
  void addStartDocument(const WPXPropertyList &propList);
  bool empty() const
  {
    return m_bodyElements.empty();
  }
private:
  ABWOutputElements(const ABWOutputElements &);
  ABWOutputElements &operator=(const ABWOutputElements &);
  std::list<ABWOutputElement *> m_bodyElements;
  std::map<int, std::list<ABWOutputElement *> > m_headerElements;
  std::map<int, std::list<ABWOutputElement *> > m_footerElements;
  std::list<ABWOutputElement *> *m_elements;
};


} // namespace libabw

#endif /* ABWOUTPUTELEMENTS_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
