/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ABWOutputElements.h"
#include "libabw_internal.h"

namespace libabw
{

class ABWOutputElement
{
public:
  ABWOutputElement() {}
  virtual ~ABWOutputElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface,
                     const std::map<int, std::list<ABWOutputElement *> > *footers,
                     const std::map<int, std::list<ABWOutputElement *> > *headers) const = 0;
};

class ABWCloseEndnoteElement : public ABWOutputElement
{
public:
  ABWCloseEndnoteElement() {}
  ~ABWCloseEndnoteElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
};

class ABWCloseFooterElement : public ABWOutputElement
{
public:
  ABWCloseFooterElement() {}
  ~ABWCloseFooterElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
};

class ABWCloseFootnoteElement : public ABWOutputElement
{
public:
  ABWCloseFootnoteElement() {}
  ~ABWCloseFootnoteElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
};

class ABWCloseFrameElement : public ABWOutputElement
{
public:
  ABWCloseFrameElement() {}
  ~ABWCloseFrameElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
};

class ABWCloseHeaderElement : public ABWOutputElement
{
public:
  ABWCloseHeaderElement() {}
  ~ABWCloseHeaderElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
};

class ABWCloseLinkElement : public ABWOutputElement
{
public:
  ABWCloseLinkElement() {}
  ~ABWCloseLinkElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWCloseLinkElement();
  }
};

class ABWCloseListElementElement : public ABWOutputElement
{
public:
  ABWCloseListElementElement() {}
  ~ABWCloseListElementElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWCloseListElementElement();
  }
};

class ABWCloseOrderedListLevelElement : public ABWOutputElement
{
public:
  ABWCloseOrderedListLevelElement() {}
  ~ABWCloseOrderedListLevelElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWCloseOrderedListLevelElement();
  }
};

class ABWClosePageSpanElement : public ABWOutputElement
{
public:
  ABWClosePageSpanElement() {}
  ~ABWClosePageSpanElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWClosePageSpanElement();
  }
};

class ABWCloseParagraphElement : public ABWOutputElement
{
public:
  ABWCloseParagraphElement() {}
  ~ABWCloseParagraphElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWCloseParagraphElement();
  }
};

class ABWCloseSectionElement : public ABWOutputElement
{
public:
  ABWCloseSectionElement() {}
  ~ABWCloseSectionElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWCloseSectionElement();
  }
};

class ABWCloseSpanElement : public ABWOutputElement
{
public:
  ABWCloseSpanElement() {}
  ~ABWCloseSpanElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWCloseSpanElement();
  }
};

class ABWCloseTableElement : public ABWOutputElement
{
public:
  ABWCloseTableElement() {}
  ~ABWCloseTableElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWCloseTableElement();
  }
};

class ABWCloseTableCellElement : public ABWOutputElement
{
public:
  ABWCloseTableCellElement() {}
  ~ABWCloseTableCellElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWCloseTableCellElement();
  }
};

class ABWCloseTableRowElement : public ABWOutputElement
{
public:
  ABWCloseTableRowElement() {}
  ~ABWCloseTableRowElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWCloseTableRowElement();
  }
};

class ABWCloseUnorderedListLevelElement : public ABWOutputElement
{
public:
  ABWCloseUnorderedListLevelElement() {}
  ~ABWCloseUnorderedListLevelElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWCloseUnorderedListLevelElement();
  }
};

class ABWInsertBinaryObjectElement : public ABWOutputElement
{
public:
  ABWInsertBinaryObjectElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWInsertBinaryObjectElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWInsertBinaryObjectElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWInsertCoveredTableCellElement : public ABWOutputElement
{
public:
  ABWInsertCoveredTableCellElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWInsertCoveredTableCellElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWInsertCoveredTableCellElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWInsertLineBreakElement : public ABWOutputElement
{
public:
  ABWInsertLineBreakElement() {}
  ~ABWInsertLineBreakElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWInsertLineBreakElement();
  }
};

class ABWInsertSpaceElement : public ABWOutputElement
{
public:
  ABWInsertSpaceElement() {}
  ~ABWInsertSpaceElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWInsertSpaceElement();
  }
};

class ABWInsertTabElement : public ABWOutputElement
{
public:
  ABWInsertTabElement() {}
  ~ABWInsertTabElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWInsertTabElement();
  }
};

class ABWInsertTextElement : public ABWOutputElement
{
public:
  ABWInsertTextElement(const librevenge::RVNGString &text) :
    m_text(text) {}
  ~ABWInsertTextElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWInsertTextElement(m_text);
  }
private:
  librevenge::RVNGString m_text;
};

class ABWOpenEndnoteElement : public ABWOutputElement
{
public:
  ABWOpenEndnoteElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenEndnoteElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWOpenEndnoteElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenFooterElement : public ABWOutputElement
{
public:
  ABWOpenFooterElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenFooterElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWOpenFooterElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenFootnoteElement : public ABWOutputElement
{
public:
  ABWOpenFootnoteElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenFootnoteElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWOpenFootnoteElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenFrameElement : public ABWOutputElement
{
public:
  ABWOpenFrameElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenFrameElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWOpenFrameElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenHeaderElement : public ABWOutputElement
{
public:
  ABWOpenHeaderElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenHeaderElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
  ABWOutputElement *clone()
  {
    return new ABWOpenHeaderElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenLinkElement : public ABWOutputElement
{
public:
  ABWOpenLinkElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenLinkElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenPageSpanElement : public ABWOutputElement
{
public:
  ABWOpenPageSpanElement(const librevenge::RVNGPropertyList &propList,
                         int footer, int footerLeft, int footerFirst, int footerLast,
                         int header, int headerLeft, int headerFirst, int headerLast) :
    m_propList(propList),
    m_footer(footer),
    m_footerLeft(footerLeft),
    m_footerFirst(footerFirst),
    m_footerLast(footerLast),
    m_header(header),
    m_headerLeft(headerLeft),
    m_headerFirst(headerFirst),
    m_headerLast(headerLast) {}
  ~ABWOpenPageSpanElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
private:
  void _writeElements(librevenge::RVNGTextInterface *iface, int id,
                      const std::map<int, std::list<ABWOutputElement *> > *elements) const;
  librevenge::RVNGPropertyList m_propList;
  int m_footer;
  int m_footerLeft;
  int m_footerFirst;
  int m_footerLast;
  int m_header;
  int m_headerLeft;
  int m_headerFirst;
  int m_headerLast;
};

class ABWOpenParagraphElement : public ABWOutputElement
{
public:
  ABWOpenParagraphElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenParagraphElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenSectionElement : public ABWOutputElement
{
public:
  ABWOpenSectionElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenSectionElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenSpanElement : public ABWOutputElement
{
public:
  ABWOpenSpanElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenSpanElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenTableElement : public ABWOutputElement
{
public:
  ABWOpenTableElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenTableElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenTableCellElement : public ABWOutputElement
{
public:
  ABWOpenTableCellElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenTableCellElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenTableRowElement : public ABWOutputElement
{
public:
  ABWOpenTableRowElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenTableRowElement() {}
  void write(librevenge::RVNGTextInterface *iface,
             const std::map<int, std::list<ABWOutputElement *> > *footers,
             const std::map<int, std::list<ABWOutputElement *> > *headers) const;
private:
  librevenge::RVNGPropertyList m_propList;
};

} // namespace libabw

void libabw::ABWCloseEndnoteElement::write(librevenge::RVNGTextInterface *iface,
                                           const std::map<int, std::list<ABWOutputElement *> > *,
                                           const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeEndnote();
}

void libabw::ABWCloseFooterElement::write(librevenge::RVNGTextInterface *iface,
                                          const std::map<int, std::list<ABWOutputElement *> > *,
                                          const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeFooter();
}

void libabw::ABWCloseFootnoteElement::write(librevenge::RVNGTextInterface *iface,
                                            const std::map<int, std::list<ABWOutputElement *> > *,
                                            const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeFootnote();
}

void libabw::ABWCloseFrameElement::write(librevenge::RVNGTextInterface *iface,
                                         const std::map<int, std::list<ABWOutputElement *> > *,
                                         const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeFrame();
}

void libabw::ABWCloseHeaderElement::write(librevenge::RVNGTextInterface *iface,
                                          const std::map<int, std::list<ABWOutputElement *> > *,
                                          const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeHeader();
}

void libabw::ABWCloseLinkElement::write(librevenge::RVNGTextInterface *iface,
                                        const std::map<int, std::list<ABWOutputElement *> > *,
                                        const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeLink();
}

void libabw::ABWCloseListElementElement::write(librevenge::RVNGTextInterface *iface,
                                               const std::map<int, std::list<ABWOutputElement *> > *,
                                               const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeLink();
}

void libabw::ABWCloseOrderedListLevelElement::write(librevenge::RVNGTextInterface *iface,
                                                    const std::map<int, std::list<ABWOutputElement *> > *,
                                                    const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeOrderedListLevel();
}

void libabw::ABWClosePageSpanElement::write(librevenge::RVNGTextInterface *iface,
                                            const std::map<int, std::list<ABWOutputElement *> > *,
                                            const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closePageSpan();
}

void libabw::ABWCloseParagraphElement::write(librevenge::RVNGTextInterface *iface,
                                             const std::map<int, std::list<ABWOutputElement *> > *,
                                             const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeParagraph();
}

void libabw::ABWCloseSectionElement::write(librevenge::RVNGTextInterface *iface,
                                           const std::map<int, std::list<ABWOutputElement *> > *,
                                           const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeSection();
}

void libabw::ABWCloseSpanElement::write(librevenge::RVNGTextInterface *iface,
                                        const std::map<int, std::list<ABWOutputElement *> > *,
                                        const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeSpan();
}

void libabw::ABWCloseTableElement::write(librevenge::RVNGTextInterface *iface,
                                         const std::map<int, std::list<ABWOutputElement *> > *,
                                         const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeTable();
}

void libabw::ABWCloseTableCellElement::write(librevenge::RVNGTextInterface *iface,
                                             const std::map<int, std::list<ABWOutputElement *> > *,
                                             const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeTableCell();
}

void libabw::ABWCloseTableRowElement::write(librevenge::RVNGTextInterface *iface,
                                            const std::map<int, std::list<ABWOutputElement *> > *,
                                            const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeTableRow();
}

void libabw::ABWCloseUnorderedListLevelElement::write(librevenge::RVNGTextInterface *iface,
                                                      const std::map<int, std::list<ABWOutputElement *> > *,
                                                      const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->closeUnorderedListLevel();
}

void libabw::ABWInsertBinaryObjectElement::write(librevenge::RVNGTextInterface *iface,
                                                 const std::map<int, std::list<ABWOutputElement *> > *,
                                                 const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->insertBinaryObject(m_propList);
}

void libabw::ABWInsertCoveredTableCellElement::write(librevenge::RVNGTextInterface *iface,
                                                     const std::map<int, std::list<ABWOutputElement *> > *,
                                                     const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->insertCoveredTableCell(m_propList);
}

void libabw::ABWInsertLineBreakElement::write(librevenge::RVNGTextInterface *iface,
                                              const std::map<int, std::list<ABWOutputElement *> > *,
                                              const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->insertLineBreak();
}

void libabw::ABWInsertSpaceElement::write(librevenge::RVNGTextInterface *iface,
                                          const std::map<int, std::list<ABWOutputElement *> > *,
                                          const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->insertSpace();
}

void libabw::ABWInsertTabElement::write(librevenge::RVNGTextInterface *iface,
                                        const std::map<int, std::list<ABWOutputElement *> > *,
                                        const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->insertTab();
}

void libabw::ABWInsertTextElement::write(librevenge::RVNGTextInterface *iface,
                                         const std::map<int, std::list<ABWOutputElement *> > *,
                                         const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->insertText(m_text);
}

void libabw::ABWOpenEndnoteElement::write(librevenge::RVNGTextInterface *iface,
                                          const std::map<int, std::list<ABWOutputElement *> > *,
                                          const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->openEndnote(m_propList);
}

void libabw::ABWOpenFooterElement::write(librevenge::RVNGTextInterface *iface,
                                         const std::map<int, std::list<ABWOutputElement *> > *,
                                         const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->openFooter(m_propList);
}

void libabw::ABWOpenFootnoteElement::write(librevenge::RVNGTextInterface *iface,
                                           const std::map<int, std::list<ABWOutputElement *> > *,
                                           const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->openFootnote(m_propList);
}

void libabw::ABWOpenFrameElement::write(librevenge::RVNGTextInterface *iface,
                                        const std::map<int, std::list<ABWOutputElement *> > *,
                                        const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->openFrame(m_propList);
}

void libabw::ABWOpenHeaderElement::write(librevenge::RVNGTextInterface *iface,
                                         const std::map<int, std::list<ABWOutputElement *> > *,
                                         const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->openHeader(m_propList);
}

void libabw::ABWOpenLinkElement::write(librevenge::RVNGTextInterface *iface,
                                       const std::map<int, std::list<ABWOutputElement *> > *,
                                       const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->openLink(m_propList);
}

void libabw::ABWOpenPageSpanElement::write(librevenge::RVNGTextInterface *iface,
                                           const std::map<int, std::list<ABWOutputElement *> > *footers,
                                           const std::map<int, std::list<ABWOutputElement *> > *headers) const
{
  if (iface)
  {
    // open the page span
    iface->openPageSpan(m_propList);
    // write out the footers
    _writeElements(iface, m_footer, footers);
    _writeElements(iface, m_footerLeft, footers);
    _writeElements(iface, m_footerFirst, footers);
    _writeElements(iface, m_footerLast, footers);
    // write out the headers
    _writeElements(iface, m_header, headers);
    _writeElements(iface, m_headerLeft, headers);
    _writeElements(iface, m_headerFirst, headers);
    _writeElements(iface, m_headerLast, headers);
    // and continue with writing out the other stuff
  }

}

void libabw::ABWOpenPageSpanElement::_writeElements(librevenge::RVNGTextInterface *iface, int id,
                                                    const std::map<int, std::list<ABWOutputElement *> > *elements) const
{
  if (!elements || id < 0 || !iface)
    return;

  std::map<int, std::list<ABWOutputElement *> >::const_iterator iterMap = elements->find(id);
  if (iterMap == elements->end() || iterMap->second.empty())
    return;

  for (std::list<ABWOutputElement *>::const_iterator iterVec = iterMap->second.begin();
       iterVec != iterMap->second.end(); ++iterVec)
    (*iterVec)->write(iface, 0, 0);
}

void libabw::ABWOpenParagraphElement::write(librevenge::RVNGTextInterface *iface,
                                            const std::map<int, std::list<ABWOutputElement *> > *,
                                            const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->openParagraph(m_propList);
}

void libabw::ABWOpenSectionElement::write(librevenge::RVNGTextInterface *iface,
                                          const std::map<int, std::list<ABWOutputElement *> > *,
                                          const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->openSection(m_propList);
}

void libabw::ABWOpenSpanElement::write(librevenge::RVNGTextInterface *iface,
                                       const std::map<int, std::list<ABWOutputElement *> > *,
                                       const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->openSpan(m_propList);
}

void libabw::ABWOpenTableElement::write(librevenge::RVNGTextInterface *iface,
                                        const std::map<int, std::list<ABWOutputElement *> > *,
                                        const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->openTable(m_propList);
}

void libabw::ABWOpenTableCellElement::write(librevenge::RVNGTextInterface *iface,
                                            const std::map<int, std::list<ABWOutputElement *> > *,
                                            const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->openTableCell(m_propList);
}

void libabw::ABWOpenTableRowElement::write(librevenge::RVNGTextInterface *iface,
                                           const std::map<int, std::list<ABWOutputElement *> > *,
                                           const std::map<int, std::list<ABWOutputElement *> > *) const
{
  if (iface)
    iface->openTableRow(m_propList);
}


// ABWOutputElements

libabw::ABWOutputElements::ABWOutputElements()
  : m_bodyElements(), m_headerElements(), m_footerElements(), m_elements(0)
{
  m_elements = &m_bodyElements;
}

libabw::ABWOutputElements::~ABWOutputElements()
{
  m_elements = 0;
  std::list<ABWOutputElement *>::iterator iterVec;
  for (iterVec = m_bodyElements.begin(); iterVec != m_bodyElements.end(); ++iterVec)
    delete(*iterVec);
  std::map<int, std::list<ABWOutputElement *> >::iterator iterMap;
  for (iterMap = m_headerElements.begin(); iterMap != m_headerElements.end(); ++iterMap)
  {
    for (iterVec = iterMap->second.begin(); iterVec != iterMap->second.end(); ++iterVec)
      delete(*iterVec);
  }
  for (iterMap = m_footerElements.begin(); iterMap != m_footerElements.end(); ++iterMap)
  {
    for (iterVec = iterMap->second.begin(); iterVec != iterMap->second.end(); ++iterVec)
      delete(*iterVec);
  }
}

void libabw::ABWOutputElements::write(librevenge::RVNGTextInterface *iface) const
{
  std::list<ABWOutputElement *>::const_iterator iter;
  for (iter = m_bodyElements.begin(); iter != m_bodyElements.end(); ++iter)
    (*iter)->write(iface, &m_footerElements, &m_headerElements);
}

void libabw::ABWOutputElements::addCloseEndnote()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseEndnoteElement());
}

void libabw::ABWOutputElements::addCloseFooter()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseFooterElement());
  m_elements = &m_bodyElements;
}

void libabw::ABWOutputElements::addCloseFootnote()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseFootnoteElement());
}

void libabw::ABWOutputElements::addCloseFrame()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseFrameElement());
}

void libabw::ABWOutputElements::addCloseHeader()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseHeaderElement());
  m_elements = &m_bodyElements;
}

void libabw::ABWOutputElements::addCloseLink()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseLinkElement());
}

void libabw::ABWOutputElements::addCloseListElement()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseListElementElement());
}

void libabw::ABWOutputElements::addCloseOrderedListLevel()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseOrderedListLevelElement());
}

void libabw::ABWOutputElements::addClosePageSpan()
{
  if (m_elements)
    m_elements->push_back(new ABWClosePageSpanElement());
}

void libabw::ABWOutputElements::addCloseParagraph()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseParagraphElement());
}

void libabw::ABWOutputElements::addCloseSection()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseSectionElement());
}

void libabw::ABWOutputElements::addCloseSpan()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseSpanElement());
}

void libabw::ABWOutputElements::addCloseTable()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseTableElement());
}

void libabw::ABWOutputElements::addCloseTableCell()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseTableCellElement());
}

void libabw::ABWOutputElements::addCloseTableRow()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseTableRowElement());
}

void libabw::ABWOutputElements::addCloseUnorderedListLevel()
{
  if (m_elements)
    m_elements->push_back(new ABWCloseUnorderedListLevelElement());
}

void libabw::ABWOutputElements::addInsertBinaryObject(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(new ABWInsertBinaryObjectElement(propList));
}

void libabw::ABWOutputElements::addInsertCoveredTableCell(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(new ABWInsertCoveredTableCellElement(propList));
}

void libabw::ABWOutputElements::addInsertLineBreak()
{
  if (m_elements)
    m_elements->push_back(new ABWInsertLineBreakElement());
}

void libabw::ABWOutputElements::addInsertSpace()
{
  if (m_elements)
    m_elements->push_back(new ABWInsertSpaceElement());
}

void libabw::ABWOutputElements::addInsertTab()
{
  if (m_elements)
    m_elements->push_back(new ABWInsertTabElement());
}

void libabw::ABWOutputElements::addInsertText(const librevenge::RVNGString &text)
{
  if (m_elements)
    m_elements->push_back(new ABWInsertTextElement(text));
}

void libabw::ABWOutputElements::addOpenEndnote(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(new ABWOpenEndnoteElement(propList));
}

void libabw::ABWOutputElements::addOpenFooter(const librevenge::RVNGPropertyList &propList, int id)
{
  // if the corresponding element of the map does not exist, this will default-construct it.
  // In that case we will get an empty list to fill with the footer content. If the element
  // already exists, this might be a footer with different occurrence and we will add it to
  // the existing one.
  m_elements = &m_footerElements[id];
  if (m_elements)
    m_elements->push_back(new ABWOpenFooterElement(propList));
}

void libabw::ABWOutputElements::addOpenFootnote(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(new ABWOpenFootnoteElement(propList));
}

void libabw::ABWOutputElements::addOpenFrame(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(new ABWOpenFrameElement(propList));
}

void libabw::ABWOutputElements::addOpenHeader(const librevenge::RVNGPropertyList &propList, int id)
{
  // Check the comment in addOpenFooter to see what happens here
  m_elements = &m_headerElements[id];
  if (m_elements)
    m_elements->push_back(new ABWOpenHeaderElement(propList));
}

void libabw::ABWOutputElements::addOpenLink(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(new ABWOpenLinkElement(propList));
}

void libabw::ABWOutputElements::addOpenPageSpan(const librevenge::RVNGPropertyList &propList,
                                                int footer, int footerLeft, int footerFirst, int footerLast,
                                                int header, int headerLeft, int headerFirst, int headerLast)
{
  if (m_elements)
    m_elements->push_back(new ABWOpenPageSpanElement(propList, footer, footerLeft, footerFirst, footerLast,
                                                     header, headerLeft, headerFirst, headerLast));
}

void libabw::ABWOutputElements::addOpenParagraph(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(new ABWOpenParagraphElement(propList));
}

void libabw::ABWOutputElements::addOpenSection(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(new ABWOpenSectionElement(propList));
}

void libabw::ABWOutputElements::addOpenSpan(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(new ABWOpenSpanElement(propList));
}

void libabw::ABWOutputElements::addOpenTable(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(new ABWOpenTableElement(propList));
}

void libabw::ABWOutputElements::addOpenTableCell(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(new ABWOpenTableCellElement(propList));
}

void libabw::ABWOutputElements::addOpenTableRow(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(new ABWOpenTableRowElement(propList));
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
