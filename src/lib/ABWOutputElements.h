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
#include <memory>

#include <librevenge/librevenge.h>

namespace libabw
{

class ABWOutputElement;

class ABWOutputElements
{
public:
  typedef std::list<std::unique_ptr<ABWOutputElement>> OutputElements_t;
  typedef std::map<int, OutputElements_t> OutputElementsMap_t;

  ABWOutputElements();
  virtual ~ABWOutputElements();
  void append(const ABWOutputElements &elements);
  void write(librevenge::RVNGTextInterface *iface) const;
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
  void addInsertBinaryObject(const librevenge::RVNGPropertyList &propList);
  void addInsertCoveredTableCell(const librevenge::RVNGPropertyList &propList);
  void addInsertField(const librevenge::RVNGPropertyList &propList);
  void addInsertLineBreak();
  void addInsertSpace();
  void addInsertTab();
  void addInsertText(const librevenge::RVNGString &text);
  void addOpenEndnote(const librevenge::RVNGPropertyList &propList);
  void addOpenFooter(const librevenge::RVNGPropertyList &propList, int id);
  void addOpenFootnote(const librevenge::RVNGPropertyList &propList);
  void addOpenFrame(const librevenge::RVNGPropertyList &propList);
  void addOpenHeader(const librevenge::RVNGPropertyList &propList, int id);
  void addOpenLink(const librevenge::RVNGPropertyList &propList);
  void addOpenListElement(const librevenge::RVNGPropertyList &propList);
  void addOpenOrderedListLevel(const librevenge::RVNGPropertyList &propList);
  void addOpenPageSpan(const librevenge::RVNGPropertyList &propList,
                       int footer, int footerLeft, int footerFirst, int footerLast,
                       int header, int headerLeft, int headerFirst, int headerLast);
  void addOpenParagraph(const librevenge::RVNGPropertyList &propList);
  void addOpenSection(const librevenge::RVNGPropertyList &propList);
  void addOpenSpan(const librevenge::RVNGPropertyList &propList);
  void addOpenTable(const librevenge::RVNGPropertyList &propList);
  void addOpenTableCell(const librevenge::RVNGPropertyList &propList);
  void addOpenTableRow(const librevenge::RVNGPropertyList &propList);
  void addOpenUnorderedListLevel(const librevenge::RVNGPropertyList &propList);
  void addStartDocument(const librevenge::RVNGPropertyList &propList);
  bool empty() const
  {
    return m_bodyElements.empty();
  }
private:
  ABWOutputElements(const ABWOutputElements &);
  ABWOutputElements &operator=(const ABWOutputElements &);
  OutputElements_t m_bodyElements;
  std::map<int, OutputElements_t > m_headerElements;
  std::map<int, OutputElements_t > m_footerElements;
  OutputElements_t *m_elements;
};


} // namespace libabw

#endif /* ABWOUTPUTELEMENTS_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
