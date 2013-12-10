/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ABWOutputElements.h"

namespace libabw
{

class ABWOutputElement
{
public:
  ABWOutputElement() {}
  virtual ~ABWOutputElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface) = 0;
  virtual ABWOutputElement *clone() = 0;
};

class ABWCloseEndnoteElement : public ABWOutputElement
{
public:
  ABWCloseEndnoteElement();
  virtual ~ABWCloseEndnoteElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWCloseEndnoteElement();
  }
};

class ABWCloseFootnoteElement : public ABWOutputElement
{
public:
  ABWCloseFootnoteElement();
  virtual ~ABWCloseFootnoteElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWCloseFootnoteElement();
  }
};

class ABWCloseFrameElement : public ABWOutputElement
{
public:
  ABWCloseFrameElement();
  virtual ~ABWCloseFrameElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWCloseFrameElement();
  }
};

class ABWCloseLinkElement : public ABWOutputElement
{
public:
  ABWCloseLinkElement();
  virtual ~ABWCloseLinkElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWCloseLinkElement();
  }
};

class ABWClosePageSpanElement : public ABWOutputElement
{
public:
  ABWClosePageSpanElement();
  virtual ~ABWClosePageSpanElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWClosePageSpanElement();
  }
};

class ABWCloseParagraphElement : public ABWOutputElement
{
public:
  ABWCloseParagraphElement();
  virtual ~ABWCloseParagraphElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWCloseParagraphElement();
  }
};

class ABWCloseSectionElement : public ABWOutputElement
{
public:
  ABWCloseSectionElement();
  virtual ~ABWCloseSectionElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWCloseSectionElement();
  }
};

class ABWCloseSpanElement : public ABWOutputElement
{
public:
  ABWCloseSpanElement();
  virtual ~ABWCloseSpanElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWCloseSpanElement();
  }
};

class ABWCloseTableElement : public ABWOutputElement
{
public:
  ABWCloseTableElement();
  virtual ~ABWCloseTableElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWCloseTableElement();
  }
};

class ABWCloseTableCellElement : public ABWOutputElement
{
public:
  ABWCloseTableCellElement();
  virtual ~ABWCloseTableCellElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWCloseTableCellElement();
  }
};

class ABWCloseTableRowElement : public ABWOutputElement
{
public:
  ABWCloseTableRowElement();
  virtual ~ABWCloseTableRowElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWCloseTableRowElement();
  }
};

class ABWInsertBinaryObjectElement : public ABWOutputElement
{
public:
  ABWInsertBinaryObjectElement(const librevenge::RVNGPropertyList &propList);
  virtual ~ABWInsertBinaryObjectElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWInsertBinaryObjectElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWInsertCoveredTableCellElement : public ABWOutputElement
{
public:
  ABWInsertCoveredTableCellElement(const librevenge::RVNGPropertyList &propList);
  virtual ~ABWInsertCoveredTableCellElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWInsertCoveredTableCellElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWInsertLineBreakElement : public ABWOutputElement
{
public:
  ABWInsertLineBreakElement();
  virtual ~ABWInsertLineBreakElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWInsertLineBreakElement();
  }
};

class ABWInsertSpaceElement : public ABWOutputElement
{
public:
  ABWInsertSpaceElement();
  virtual ~ABWInsertSpaceElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWInsertSpaceElement();
  }
};

class ABWInsertTabElement : public ABWOutputElement
{
public:
  ABWInsertTabElement();
  virtual ~ABWInsertTabElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWInsertTabElement();
  }
};

class ABWInsertTextElement : public ABWOutputElement
{
public:
  ABWInsertTextElement(const librevenge::RVNGString &text);
  virtual ~ABWInsertTextElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWInsertTextElement(m_text);
  }
private:
  librevenge::RVNGString m_text;
};

class ABWOpenEndnoteElement : public ABWOutputElement
{
public:
  ABWOpenEndnoteElement(const librevenge::RVNGPropertyList &propList);
  virtual ~ABWOpenEndnoteElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWOpenEndnoteElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenFootnoteElement : public ABWOutputElement
{
public:
  ABWOpenFootnoteElement(const librevenge::RVNGPropertyList &propList);
  virtual ~ABWOpenFootnoteElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWOpenFootnoteElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenFrameElement : public ABWOutputElement
{
public:
  ABWOpenFrameElement(const librevenge::RVNGPropertyList &propList);
  virtual ~ABWOpenFrameElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWOpenFrameElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenLinkElement : public ABWOutputElement
{
public:
  ABWOpenLinkElement(const librevenge::RVNGPropertyList &propList);
  virtual ~ABWOpenLinkElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWOpenLinkElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenPageSpanElement : public ABWOutputElement
{
public:
  ABWOpenPageSpanElement(const librevenge::RVNGPropertyList &propList);
  virtual ~ABWOpenPageSpanElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWOpenPageSpanElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenParagraphElement : public ABWOutputElement
{
public:
  ABWOpenParagraphElement(const librevenge::RVNGPropertyList &propList);
  virtual ~ABWOpenParagraphElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWOpenParagraphElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenSectionElement : public ABWOutputElement
{
public:
  ABWOpenSectionElement(const librevenge::RVNGPropertyList &propList);
  virtual ~ABWOpenSectionElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWOpenSectionElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenSpanElement : public ABWOutputElement
{
public:
  ABWOpenSpanElement(const librevenge::RVNGPropertyList &propList);
  virtual ~ABWOpenSpanElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWOpenSpanElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenTableElement : public ABWOutputElement
{
public:
  ABWOpenTableElement(const librevenge::RVNGPropertyList &propList);
  virtual ~ABWOpenTableElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWOpenTableElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenTableCellElement : public ABWOutputElement
{
public:
  ABWOpenTableCellElement(const librevenge::RVNGPropertyList &propList);
  virtual ~ABWOpenTableCellElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWOpenTableCellElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

class ABWOpenTableRowElement : public ABWOutputElement
{
public:
  ABWOpenTableRowElement(const librevenge::RVNGPropertyList &propList);
  virtual ~ABWOpenTableRowElement() {}
  virtual void write(librevenge::RVNGTextInterface *iface);
  virtual ABWOutputElement *clone()
  {
    return new ABWOpenTableRowElement(m_propList);
  }
private:
  librevenge::RVNGPropertyList m_propList;
};

} // namespace libabw

libabw::ABWCloseEndnoteElement::ABWCloseEndnoteElement() {}

void libabw::ABWCloseEndnoteElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->closeEndnote();
}

libabw::ABWCloseFootnoteElement::ABWCloseFootnoteElement() {}

void libabw::ABWCloseFootnoteElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->closeFootnote();
}

libabw::ABWCloseFrameElement::ABWCloseFrameElement() {}

void libabw::ABWCloseFrameElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->closeFrame();
}

libabw::ABWCloseLinkElement::ABWCloseLinkElement() {}

void libabw::ABWCloseLinkElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->closeLink();
}

libabw::ABWClosePageSpanElement::ABWClosePageSpanElement() {}

void libabw::ABWClosePageSpanElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->closePageSpan();
}

libabw::ABWCloseParagraphElement::ABWCloseParagraphElement() {}

void libabw::ABWCloseParagraphElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->closeParagraph();
}

libabw::ABWCloseSectionElement::ABWCloseSectionElement() {}

void libabw::ABWCloseSectionElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->closeSection();
}

libabw::ABWCloseSpanElement::ABWCloseSpanElement() {}

void libabw::ABWCloseSpanElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->closeSpan();
}

libabw::ABWCloseTableElement::ABWCloseTableElement() {}

void libabw::ABWCloseTableElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->closeTable();
}

libabw::ABWCloseTableCellElement::ABWCloseTableCellElement() {}

void libabw::ABWCloseTableCellElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->closeTableCell();
}

libabw::ABWCloseTableRowElement::ABWCloseTableRowElement() {}

void libabw::ABWCloseTableRowElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->closeTableRow();
}

libabw::ABWInsertBinaryObjectElement::ABWInsertBinaryObjectElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libabw::ABWInsertBinaryObjectElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->insertBinaryObject(m_propList);
}

libabw::ABWInsertCoveredTableCellElement::ABWInsertCoveredTableCellElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libabw::ABWInsertCoveredTableCellElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->insertCoveredTableCell(m_propList);
}

libabw::ABWInsertLineBreakElement::ABWInsertLineBreakElement() {}

void libabw::ABWInsertLineBreakElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->insertLineBreak();
}

libabw::ABWInsertSpaceElement::ABWInsertSpaceElement() {}

void libabw::ABWInsertSpaceElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->insertSpace();
}

libabw::ABWInsertTabElement::ABWInsertTabElement() {}

void libabw::ABWInsertTabElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->insertTab();
}

libabw::ABWInsertTextElement::ABWInsertTextElement(const librevenge::RVNGString &text) :
  m_text(text) {}

void libabw::ABWInsertTextElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->insertText(m_text);
}

libabw::ABWOpenEndnoteElement::ABWOpenEndnoteElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libabw::ABWOpenEndnoteElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->openEndnote(m_propList);
}

libabw::ABWOpenFootnoteElement::ABWOpenFootnoteElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libabw::ABWOpenFootnoteElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->openFootnote(m_propList);
}

libabw::ABWOpenFrameElement::ABWOpenFrameElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libabw::ABWOpenFrameElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->openFrame(m_propList);
}

libabw::ABWOpenLinkElement::ABWOpenLinkElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libabw::ABWOpenLinkElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->openLink(m_propList);
}

libabw::ABWOpenPageSpanElement::ABWOpenPageSpanElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libabw::ABWOpenPageSpanElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->openPageSpan(m_propList);
}

libabw::ABWOpenParagraphElement::ABWOpenParagraphElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libabw::ABWOpenParagraphElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->openParagraph(m_propList);
}

libabw::ABWOpenSectionElement::ABWOpenSectionElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libabw::ABWOpenSectionElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->openSection(m_propList);
}

libabw::ABWOpenSpanElement::ABWOpenSpanElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libabw::ABWOpenSpanElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->openSpan(m_propList);
}

libabw::ABWOpenTableElement::ABWOpenTableElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}


void libabw::ABWOpenTableElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->openTable(m_propList);
}

libabw::ABWOpenTableCellElement::ABWOpenTableCellElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}


void libabw::ABWOpenTableCellElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->openTableCell(m_propList);
}

libabw::ABWOpenTableRowElement::ABWOpenTableRowElement(const librevenge::RVNGPropertyList &propList) :
  m_propList(propList) {}

void libabw::ABWOpenTableRowElement::write(librevenge::RVNGTextInterface *iface)
{
  if (iface)
    iface->openTableRow(m_propList);
}

libabw::ABWOutputElements::ABWOutputElements()
  : m_elements()
{
}

libabw::ABWOutputElements::ABWOutputElements(const libabw::ABWOutputElements &elements)
  : m_elements()
{
  std::vector<libabw::ABWOutputElement *>::const_iterator iter;
  for (iter = elements.m_elements.begin(); iter != elements.m_elements.end(); ++iter)
    m_elements.push_back((*iter)->clone());
}

libabw::ABWOutputElements &libabw::ABWOutputElements::operator=(const libabw::ABWOutputElements &elements)
{
  for (std::vector<ABWOutputElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); ++iter)
    delete(*iter);

  m_elements.clear();

  for (std::vector<ABWOutputElement *>::const_iterator cstiter = elements.m_elements.begin(); cstiter != elements.m_elements.end(); ++cstiter)
    m_elements.push_back((*cstiter)->clone());

  return *this;
}

void libabw::ABWOutputElements::append(const libabw::ABWOutputElements &elements)
{
  for (std::vector<ABWOutputElement *>::const_iterator cstiter = elements.m_elements.begin(); cstiter != elements.m_elements.end(); ++cstiter)
    m_elements.push_back((*cstiter)->clone());
}

libabw::ABWOutputElements::~ABWOutputElements()
{
  for (std::vector<ABWOutputElement *>::iterator iter = m_elements.begin(); iter != m_elements.end(); ++iter)
    delete(*iter);
  m_elements.clear();
}

void libabw::ABWOutputElements::write(librevenge::RVNGTextInterface *iface) const
{
  for (std::vector<ABWOutputElement *>::const_iterator iter = m_elements.begin(); iter != m_elements.end(); ++iter)
    (*iter)->write(iface);
}

void libabw::ABWOutputElements::addCloseEndnote()
{
  m_elements.push_back(new ABWCloseEndnoteElement());
}

void libabw::ABWOutputElements::addCloseFootnote()
{
  m_elements.push_back(new ABWCloseFootnoteElement());
}

void libabw::ABWOutputElements::addCloseFrame()
{
  m_elements.push_back(new ABWCloseFrameElement());
}

void libabw::ABWOutputElements::addCloseLink()
{
  m_elements.push_back(new ABWCloseLinkElement());
}

void libabw::ABWOutputElements::addClosePageSpan()
{
  m_elements.push_back(new ABWClosePageSpanElement());
}

void libabw::ABWOutputElements::addCloseParagraph()
{
  m_elements.push_back(new ABWCloseParagraphElement());
}

void libabw::ABWOutputElements::addCloseSection()
{
  m_elements.push_back(new ABWCloseSectionElement());
}

void libabw::ABWOutputElements::addCloseSpan()
{
  m_elements.push_back(new ABWCloseSpanElement());
}

void libabw::ABWOutputElements::addCloseTable()
{
  m_elements.push_back(new ABWCloseTableElement());
}

void libabw::ABWOutputElements::addCloseTableCell()
{
  m_elements.push_back(new ABWCloseTableCellElement());
}

void libabw::ABWOutputElements::addCloseTableRow()
{
  m_elements.push_back(new ABWCloseTableRowElement());
}

void libabw::ABWOutputElements::addInsertBinaryObject(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(new ABWInsertBinaryObjectElement(propList));
}

void libabw::ABWOutputElements::addInsertCoveredTableCell(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(new ABWInsertCoveredTableCellElement(propList));
}

void libabw::ABWOutputElements::addInsertLineBreak()
{
  m_elements.push_back(new ABWInsertLineBreakElement());
}

void libabw::ABWOutputElements::addInsertSpace()
{
  m_elements.push_back(new ABWInsertSpaceElement());
}

void libabw::ABWOutputElements::addInsertTab()
{
  m_elements.push_back(new ABWInsertTabElement());
}

void libabw::ABWOutputElements::addInsertText(const librevenge::RVNGString &text)
{
  m_elements.push_back(new ABWInsertTextElement(text));
}

void libabw::ABWOutputElements::addOpenEndnote(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(new ABWOpenEndnoteElement(propList));
}

void libabw::ABWOutputElements::addOpenFootnote(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(new ABWOpenFootnoteElement(propList));
}

void libabw::ABWOutputElements::addOpenFrame(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(new ABWOpenFrameElement(propList));
}

void libabw::ABWOutputElements::addOpenLink(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(new ABWOpenLinkElement(propList));
}

void libabw::ABWOutputElements::addOpenPageSpan(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(new ABWOpenPageSpanElement(propList));
}

void libabw::ABWOutputElements::addOpenParagraph(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(new ABWOpenParagraphElement(propList));
}

void libabw::ABWOutputElements::addOpenSection(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(new ABWOpenSectionElement(propList));
}

void libabw::ABWOutputElements::addOpenSpan(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(new ABWOpenSpanElement(propList));
}

void libabw::ABWOutputElements::addOpenTable(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(new ABWOpenTableElement(propList));
}

void libabw::ABWOutputElements::addOpenTableCell(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(new ABWOpenTableCellElement(propList));
}

void libabw::ABWOutputElements::addOpenTableRow(const librevenge::RVNGPropertyList &propList)
{
  m_elements.push_back(new ABWOpenTableRowElement(propList));
}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
