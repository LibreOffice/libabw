/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ABWCOLLECTOR_H__
#define __ABWCOLLECTOR_H__

#include <map>
#include <vector>
#include <string>
#include <librevenge/librevenge.h>

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

class ABWParsingState
{
public:
  ABWParsingState();
  ~ABWParsingState();

  bool m_isDocumentStarted;
  bool m_isPageSpanOpened;
  bool m_isSectionOpened;

  bool m_isSpanOpened;
  bool m_isParagraphOpened;

  std::map<std::string, std::string> m_currentParagraphStyle;
  std::map<std::string, std::string> m_currentCharacterStyle;

  std::map<std::string, ABWStyle> m_paragraphStyles;
  std::map<std::string, ABWStyle> m_characterStyles;

  double m_pageWidth;
  double m_pageHeight;

private:
  ABWParsingState(const ABWParsingState &);
  ABWParsingState &operator=(const ABWParsingState &);
};

class ABWCollector
{
public:
  ABWCollector(::librevenge::RVNGTextInterface *iface);
  virtual ~ABWCollector();

  // collector functions

  void collectParagraphStyle(const char *name, const char *basedon, const char *followedby, const char *props);
  void collectCharacterStyle(const char *name, const char *basedon, const char *followedby, const char *props);
  void collectParagraphProperties(const char *style, const char *props);
  void collectSectionProperties(const char *props);
  void collectCharacterProperties(const char *style, const char *props);
  void collectPageSize(const char *width, const char *height, const char *units, const char *pageScale);
  void closeParagraph();
  void closeSpan();
  void endSection();
  void startDocument();
  void endDocument();
  void insertLineBreak();
  void insertColumnBreak();
  void insertPageBreak();
  void insertText(const librevenge::RVNGString &text);

private:
  ABWCollector(const ABWCollector &);
  ABWCollector &operator=(const ABWCollector &);

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

  ABWParsingState *m_ps;
  librevenge::RVNGTextInterface *m_iface;

};

} // namespace libabw

#endif /* __ABWCOLLECTOR_H__ */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
