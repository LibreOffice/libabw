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

namespace
{

typedef libabw::ABWOutputElements::OutputElements_t OutputElements_t;
typedef libabw::ABWOutputElements::OutputElementsMap_t OutputElementsMap_t;

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}

namespace libabw
{

class ABWOutputElement
{
public:
  ABWOutputElement() {}
  virtual ~ABWOutputElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface,
                     const OutputElementsMap_t *footers,
                     const OutputElementsMap_t *headers) const = 0;
};

class ABWCloseEndnoteElement : public ABWOutputElement
{
public:
  ABWCloseEndnoteElement() {}
  ~ABWCloseEndnoteElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseFooterElement : public ABWOutputElement
{
public:
  ABWCloseFooterElement() {}
  ~ABWCloseFooterElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseFootnoteElement : public ABWOutputElement
{
public:
  ABWCloseFootnoteElement() {}
  ~ABWCloseFootnoteElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseFrameElement : public ABWOutputElement
{
public:
  ABWCloseFrameElement() {}
  ~ABWCloseFrameElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseHeaderElement : public ABWOutputElement
{
public:
  ABWCloseHeaderElement() {}
  ~ABWCloseHeaderElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseLinkElement : public ABWOutputElement
{
public:
  ABWCloseLinkElement() {}
  ~ABWCloseLinkElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseListElementElement : public ABWOutputElement
{
public:
  ABWCloseListElementElement() {}
  ~ABWCloseListElementElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseOrderedListLevelElement : public ABWOutputElement
{
public:
  ABWCloseOrderedListLevelElement() {}
  ~ABWCloseOrderedListLevelElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWClosePageSpanElement : public ABWOutputElement
{
public:
  ABWClosePageSpanElement() {}
  ~ABWClosePageSpanElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseParagraphElement : public ABWOutputElement
{
public:
  ABWCloseParagraphElement() {}
  ~ABWCloseParagraphElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseSectionElement : public ABWOutputElement
{
public:
  ABWCloseSectionElement() {}
  ~ABWCloseSectionElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseSpanElement : public ABWOutputElement
{
public:
  ABWCloseSpanElement() {}
  ~ABWCloseSpanElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseTableElement : public ABWOutputElement
{
public:
  ABWCloseTableElement() {}
  ~ABWCloseTableElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseTableCellElement : public ABWOutputElement
{
public:
  ABWCloseTableCellElement() {}
  ~ABWCloseTableCellElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseTableRowElement : public ABWOutputElement
{
public:
  ABWCloseTableRowElement() {}
  ~ABWCloseTableRowElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseTextBoxElement : public ABWOutputElement
{
public:
  ABWCloseTextBoxElement() {}
  ~ABWCloseTextBoxElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWCloseUnorderedListLevelElement : public ABWOutputElement
{
public:
  ABWCloseUnorderedListLevelElement() {}
  ~ABWCloseUnorderedListLevelElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWInsertBinaryObjectElement : public ABWOutputElement
{
public:
  ABWInsertBinaryObjectElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWInsertBinaryObjectElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWInsertFieldElement : public ABWOutputElement
{
public:
  ABWInsertFieldElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWInsertFieldElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWInsertCoveredTableCellElement : public ABWOutputElement
{
public:
  ABWInsertCoveredTableCellElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWInsertCoveredTableCellElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWInsertLineBreakElement : public ABWOutputElement
{
public:
  ABWInsertLineBreakElement() {}
  ~ABWInsertLineBreakElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWInsertSpaceElement : public ABWOutputElement
{
public:
  ABWInsertSpaceElement() {}
  ~ABWInsertSpaceElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWInsertTabElement : public ABWOutputElement
{
public:
  ABWInsertTabElement() {}
  ~ABWInsertTabElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
};

class ABWInsertTextElement : public ABWOutputElement
{
public:
  ABWInsertTextElement(const librevenge::RVNGString &text) :
    m_text(text) {}
  ~ABWInsertTextElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGString m_text;
};

class ABWOpenEndnoteElement : public ABWOutputElement
{
public:
  ABWOpenEndnoteElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenEndnoteElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenFooterElement : public ABWOutputElement
{
public:
  ABWOpenFooterElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenFooterElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenFootnoteElement : public ABWOutputElement
{
public:
  ABWOpenFootnoteElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenFootnoteElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenFrameElement : public ABWOutputElement
{
public:
  ABWOpenFrameElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenFrameElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenHeaderElement : public ABWOutputElement
{
public:
  ABWOpenHeaderElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenHeaderElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenLinkElement : public ABWOutputElement
{
public:
  ABWOpenLinkElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenLinkElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenListElementElement : public ABWOutputElement
{
public:
  ABWOpenListElementElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenListElementElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenOrderedListLevelElement : public ABWOutputElement
{
public:
  ABWOpenOrderedListLevelElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenOrderedListLevelElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
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
  ~ABWOpenPageSpanElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  void _writeElements(librevenge::RVNGTextInterface *iface, int id,
                      const OutputElementsMap_t *elements) const;
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
  ~ABWOpenParagraphElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenSectionElement : public ABWOutputElement
{
public:
  ABWOpenSectionElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenSectionElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenSpanElement : public ABWOutputElement
{
public:
  ABWOpenSpanElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenSpanElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenTableElement : public ABWOutputElement
{
public:
  ABWOpenTableElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenTableElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenTableCellElement : public ABWOutputElement
{
public:
  ABWOpenTableCellElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenTableCellElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenTableRowElement : public ABWOutputElement
{
public:
  ABWOpenTableRowElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenTableRowElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenTextBoxElement : public ABWOutputElement
{
public:
  ABWOpenTextBoxElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenTextBoxElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenUnorderedListLevelElement : public ABWOutputElement
{
public:
  ABWOpenUnorderedListLevelElement(const librevenge::RVNGPropertyList &propList) :
    m_propList(propList) {}
  ~ABWOpenUnorderedListLevelElement() override {}
  void write(librevenge::RVNGTextInterface *iface,
             const OutputElementsMap_t *footers,
             const OutputElementsMap_t *headers) const override;
private:
  librevenge::RVNGPropertyList m_propList;
};

} // namespace libabw

void libabw::ABWCloseEndnoteElement::write(librevenge::RVNGTextInterface *iface,
                                           const OutputElementsMap_t *,
                                           const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeEndnote();
}

void libabw::ABWCloseFooterElement::write(librevenge::RVNGTextInterface *iface,
                                          const OutputElementsMap_t *,
                                          const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeFooter();
}

void libabw::ABWCloseFootnoteElement::write(librevenge::RVNGTextInterface *iface,
                                            const OutputElementsMap_t *,
                                            const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeFootnote();
}

void libabw::ABWCloseFrameElement::write(librevenge::RVNGTextInterface *iface,
                                         const OutputElementsMap_t *,
                                         const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeFrame();
}

void libabw::ABWCloseHeaderElement::write(librevenge::RVNGTextInterface *iface,
                                          const OutputElementsMap_t *,
                                          const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeHeader();
}

void libabw::ABWCloseLinkElement::write(librevenge::RVNGTextInterface *iface,
                                        const OutputElementsMap_t *,
                                        const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeLink();
}

void libabw::ABWCloseListElementElement::write(librevenge::RVNGTextInterface *iface,
                                               const OutputElementsMap_t *,
                                               const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeListElement();
}

void libabw::ABWCloseOrderedListLevelElement::write(librevenge::RVNGTextInterface *iface,
                                                    const OutputElementsMap_t *,
                                                    const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeOrderedListLevel();
}

void libabw::ABWClosePageSpanElement::write(librevenge::RVNGTextInterface *iface,
                                            const OutputElementsMap_t *,
                                            const OutputElementsMap_t *) const
{
  if (iface)
    iface->closePageSpan();
}

void libabw::ABWCloseParagraphElement::write(librevenge::RVNGTextInterface *iface,
                                             const OutputElementsMap_t *,
                                             const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeParagraph();
}

void libabw::ABWCloseSectionElement::write(librevenge::RVNGTextInterface *iface,
                                           const OutputElementsMap_t *,
                                           const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeSection();
}

void libabw::ABWCloseSpanElement::write(librevenge::RVNGTextInterface *iface,
                                        const OutputElementsMap_t *,
                                        const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeSpan();
}

void libabw::ABWCloseTableElement::write(librevenge::RVNGTextInterface *iface,
                                         const OutputElementsMap_t *,
                                         const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeTable();
}

void libabw::ABWCloseTableCellElement::write(librevenge::RVNGTextInterface *iface,
                                             const OutputElementsMap_t *,
                                             const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeTableCell();
}

void libabw::ABWCloseTableRowElement::write(librevenge::RVNGTextInterface *iface,
                                            const OutputElementsMap_t *,
                                            const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeTableRow();
}

void libabw::ABWCloseTextBoxElement::write(librevenge::RVNGTextInterface *iface,
                                           const OutputElementsMap_t *,
                                           const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeTextBox();
}

void libabw::ABWCloseUnorderedListLevelElement::write(librevenge::RVNGTextInterface *iface,
                                                      const OutputElementsMap_t *,
                                                      const OutputElementsMap_t *) const
{
  if (iface)
    iface->closeUnorderedListLevel();
}

void libabw::ABWInsertBinaryObjectElement::write(librevenge::RVNGTextInterface *iface,
                                                 const OutputElementsMap_t *,
                                                 const OutputElementsMap_t *) const
{
  if (iface)
    iface->insertBinaryObject(m_propList);
}

void libabw::ABWInsertFieldElement::write(librevenge::RVNGTextInterface *iface,
                                          const OutputElementsMap_t *,
                                          const OutputElementsMap_t *) const
{
  if (iface)
    iface->insertField(m_propList);
}

void libabw::ABWInsertCoveredTableCellElement::write(librevenge::RVNGTextInterface *iface,
                                                     const OutputElementsMap_t *,
                                                     const OutputElementsMap_t *) const
{
  if (iface)
    iface->insertCoveredTableCell(m_propList);
}

void libabw::ABWInsertLineBreakElement::write(librevenge::RVNGTextInterface *iface,
                                              const OutputElementsMap_t *,
                                              const OutputElementsMap_t *) const
{
  if (iface)
    iface->insertLineBreak();
}

void libabw::ABWInsertSpaceElement::write(librevenge::RVNGTextInterface *iface,
                                          const OutputElementsMap_t *,
                                          const OutputElementsMap_t *) const
{
  if (iface)
    iface->insertSpace();
}

void libabw::ABWInsertTabElement::write(librevenge::RVNGTextInterface *iface,
                                        const OutputElementsMap_t *,
                                        const OutputElementsMap_t *) const
{
  if (iface)
    iface->insertTab();
}

void libabw::ABWInsertTextElement::write(librevenge::RVNGTextInterface *iface,
                                         const OutputElementsMap_t *,
                                         const OutputElementsMap_t *) const
{
  if (iface)
    iface->insertText(m_text);
}

void libabw::ABWOpenEndnoteElement::write(librevenge::RVNGTextInterface *iface,
                                          const OutputElementsMap_t *,
                                          const OutputElementsMap_t *) const
{
  if (iface)
    iface->openEndnote(m_propList);
}

void libabw::ABWOpenFooterElement::write(librevenge::RVNGTextInterface *iface,
                                         const OutputElementsMap_t *,
                                         const OutputElementsMap_t *) const
{
  if (iface)
    iface->openFooter(m_propList);
}

void libabw::ABWOpenFootnoteElement::write(librevenge::RVNGTextInterface *iface,
                                           const OutputElementsMap_t *,
                                           const OutputElementsMap_t *) const
{
  if (iface)
    iface->openFootnote(m_propList);
}

void libabw::ABWOpenFrameElement::write(librevenge::RVNGTextInterface *iface,
                                        const OutputElementsMap_t *,
                                        const OutputElementsMap_t *) const
{
  if (iface)
    iface->openFrame(m_propList);
}

void libabw::ABWOpenHeaderElement::write(librevenge::RVNGTextInterface *iface,
                                         const OutputElementsMap_t *,
                                         const OutputElementsMap_t *) const
{
  if (iface)
    iface->openHeader(m_propList);
}

void libabw::ABWOpenListElementElement::write(librevenge::RVNGTextInterface *iface,
                                              const OutputElementsMap_t *,
                                              const OutputElementsMap_t *) const
{
  if (iface)
    iface->openListElement(m_propList);
}

void libabw::ABWOpenLinkElement::write(librevenge::RVNGTextInterface *iface,
                                       const OutputElementsMap_t *,
                                       const OutputElementsMap_t *) const
{
  if (iface)
    iface->openLink(m_propList);
}

void libabw::ABWOpenOrderedListLevelElement::write(librevenge::RVNGTextInterface *iface,
                                                   const OutputElementsMap_t *,
                                                   const OutputElementsMap_t *) const
{
  if (iface)
    iface->openOrderedListLevel(m_propList);
}

void libabw::ABWOpenPageSpanElement::write(librevenge::RVNGTextInterface *iface,
                                           const OutputElementsMap_t *footers,
                                           const OutputElementsMap_t *headers) const
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
                                                    const OutputElementsMap_t *elements) const
{
  if (!elements || id < 0 || !iface)
    return;

  OutputElementsMap_t::const_iterator iterMap = elements->find(id);
  if (iterMap == elements->end() || iterMap->second.empty())
    return;

  for (OutputElements_t::const_iterator iterVec = iterMap->second.begin();
       iterVec != iterMap->second.end(); ++iterVec)
    (*iterVec)->write(iface, 0, 0);
}

void libabw::ABWOpenParagraphElement::write(librevenge::RVNGTextInterface *iface,
                                            const OutputElementsMap_t *,
                                            const OutputElementsMap_t *) const
{
  if (iface)
    iface->openParagraph(m_propList);
}

void libabw::ABWOpenSectionElement::write(librevenge::RVNGTextInterface *iface,
                                          const OutputElementsMap_t *,
                                          const OutputElementsMap_t *) const
{
  if (iface)
    iface->openSection(m_propList);
}

void libabw::ABWOpenSpanElement::write(librevenge::RVNGTextInterface *iface,
                                       const OutputElementsMap_t *,
                                       const OutputElementsMap_t *) const
{
  if (iface)
    iface->openSpan(m_propList);
}

void libabw::ABWOpenTableElement::write(librevenge::RVNGTextInterface *iface,
                                        const OutputElementsMap_t *,
                                        const OutputElementsMap_t *) const
{
  if (iface)
    iface->openTable(m_propList);
}

void libabw::ABWOpenTableCellElement::write(librevenge::RVNGTextInterface *iface,
                                            const OutputElementsMap_t *,
                                            const OutputElementsMap_t *) const
{
  if (iface)
    iface->openTableCell(m_propList);
}

void libabw::ABWOpenTableRowElement::write(librevenge::RVNGTextInterface *iface,
                                           const OutputElementsMap_t *,
                                           const OutputElementsMap_t *) const
{
  if (iface)
    iface->openTableRow(m_propList);
}

void libabw::ABWOpenTextBoxElement::write(librevenge::RVNGTextInterface *iface,
                                          const OutputElementsMap_t *,
                                          const OutputElementsMap_t *) const
{
  if (iface)
    iface->openTextBox(m_propList);
}

void libabw::ABWOpenUnorderedListLevelElement::write(librevenge::RVNGTextInterface *iface,
                                                     const OutputElementsMap_t *,
                                                     const OutputElementsMap_t *) const
{
  if (iface)
    iface->openUnorderedListLevel(m_propList);
}

// ABWOutputElements

libabw::ABWOutputElements::ABWOutputElements()
  : m_bodyElements(), m_headerElements(), m_footerElements(), m_elements(0)
{
  m_elements = &m_bodyElements;
}

libabw::ABWOutputElements::~ABWOutputElements()
{
}

void libabw::ABWOutputElements::splice(ABWOutputElements &elements)
{
  m_bodyElements.splice(m_bodyElements.end(), elements.m_bodyElements);
}

void libabw::ABWOutputElements::write(librevenge::RVNGTextInterface *iface) const
{
  OutputElements_t::const_iterator iter;
  for (iter = m_bodyElements.begin(); iter != m_bodyElements.end(); ++iter)
    (*iter)->write(iface, &m_footerElements, &m_headerElements);
}

void libabw::ABWOutputElements::addCloseEndnote()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseEndnoteElement>());
}

void libabw::ABWOutputElements::addCloseFooter()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseFooterElement>());
  m_elements = &m_bodyElements;
}

void libabw::ABWOutputElements::addCloseFootnote()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseFootnoteElement>());
}

void libabw::ABWOutputElements::addCloseFrame()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseFrameElement>());
}

void libabw::ABWOutputElements::addCloseHeader()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseHeaderElement>());
  m_elements = &m_bodyElements;
}

void libabw::ABWOutputElements::addCloseLink()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseLinkElement>());
}

void libabw::ABWOutputElements::addCloseListElement()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseListElementElement>());
}

void libabw::ABWOutputElements::addCloseOrderedListLevel()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseOrderedListLevelElement>());
}

void libabw::ABWOutputElements::addClosePageSpan()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWClosePageSpanElement>());
}

void libabw::ABWOutputElements::addCloseParagraph()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseParagraphElement>());
}

void libabw::ABWOutputElements::addCloseSection()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseSectionElement>());
}

void libabw::ABWOutputElements::addCloseSpan()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseSpanElement>());
}

void libabw::ABWOutputElements::addCloseTable()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseTableElement>());
}

void libabw::ABWOutputElements::addCloseTableCell()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseTableCellElement>());
}

void libabw::ABWOutputElements::addCloseTableRow()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseTableRowElement>());
}

void libabw::ABWOutputElements::addCloseTextBox()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseTextBoxElement>());
}

void libabw::ABWOutputElements::addCloseUnorderedListLevel()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWCloseUnorderedListLevelElement>());
}

void libabw::ABWOutputElements::addInsertBinaryObject(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWInsertBinaryObjectElement>(propList));
}

void libabw::ABWOutputElements::addInsertField(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWInsertFieldElement>(propList));
}

void libabw::ABWOutputElements::addInsertCoveredTableCell(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWInsertCoveredTableCellElement>(propList));
}

void libabw::ABWOutputElements::addInsertLineBreak()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWInsertLineBreakElement>());
}

void libabw::ABWOutputElements::addInsertSpace()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWInsertSpaceElement>());
}

void libabw::ABWOutputElements::addInsertTab()
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWInsertTabElement>());
}

void libabw::ABWOutputElements::addInsertText(const librevenge::RVNGString &text)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWInsertTextElement>(text));
}

void libabw::ABWOutputElements::addOpenEndnote(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenEndnoteElement>(propList));
}

void libabw::ABWOutputElements::addOpenFooter(const librevenge::RVNGPropertyList &propList, int id)
{
  // if the corresponding element of the map does not exist, this will default-construct it.
  // In that case we will get an empty list to fill with the footer content. If the element
  // already exists, this might be a footer with different occurrence and we will add it to
  // the existing one.
  m_elements = &m_footerElements[id];
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenFooterElement>(propList));
}

void libabw::ABWOutputElements::addOpenFootnote(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenFootnoteElement>(propList));
}

void libabw::ABWOutputElements::addOpenFrame(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenFrameElement>(propList));
}

void libabw::ABWOutputElements::addOpenHeader(const librevenge::RVNGPropertyList &propList, int id)
{
  // Check the comment in addOpenFooter to see what happens here
  m_elements = &m_headerElements[id];
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenHeaderElement>(propList));
}

void libabw::ABWOutputElements::addOpenListElement(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenListElementElement>(propList));
}

void libabw::ABWOutputElements::addOpenLink(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenLinkElement>(propList));
}

void libabw::ABWOutputElements::addOpenOrderedListLevel(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenOrderedListLevelElement>(propList));
}

void libabw::ABWOutputElements::addOpenPageSpan(const librevenge::RVNGPropertyList &propList,
                                                int footer, int footerLeft, int footerFirst, int footerLast,
                                                int header, int headerLeft, int headerFirst, int headerLast)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenPageSpanElement>(propList, footer, footerLeft, footerFirst, footerLast,
                                                              header, headerLeft, headerFirst, headerLast));
}

void libabw::ABWOutputElements::addOpenParagraph(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenParagraphElement>(propList));
}

void libabw::ABWOutputElements::addOpenSection(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenSectionElement>(propList));
}

void libabw::ABWOutputElements::addOpenSpan(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenSpanElement>(propList));
}

void libabw::ABWOutputElements::addOpenTable(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenTableElement>(propList));
}

void libabw::ABWOutputElements::addOpenTableCell(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenTableCellElement>(propList));
}

void libabw::ABWOutputElements::addOpenTableRow(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenTableRowElement>(propList));
}

void libabw::ABWOutputElements::addOpenTextBox(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenTextBoxElement>(propList));
}

void libabw::ABWOutputElements::addOpenUnorderedListLevel(const librevenge::RVNGPropertyList &propList)
{
  if (m_elements)
    m_elements->push_back(make_unique<ABWOpenUnorderedListLevelElement>(propList));
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
