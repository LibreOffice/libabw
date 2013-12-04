/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ABWSTYLES_H__
#define __ABWSTYLES_H__

#include <string>

namespace libabw
{

struct ABWColor
{
  ABWColor(unsigned char red, unsigned char green, unsigned char blue)
    : r(red), g(green), b(blue) {}
  ABWColor() : r(0), g(0), b(0) {}
  ABWColor(const ABWColor &color) : r(color.r), g(color.g), b(color.b) {}
  ABWColor &operator=(const ABWColor &color)
  {
    r = color.r;
    b = color.b;
    g = color.g;
    return *this;
  }
  ~ABWColor() {}

  unsigned char r;
  unsigned char g;
  unsigned char b;
};

struct ABWCharacterStyle
{
  ABWCharacterStyle()
    : font(), color(), size(12.0/72.0), bold(false), italic(false), underline(false),
      doubleunderline(false), strikeout(false), doublestrikeout(false), allcaps(false), initcaps(false),
      smallcaps(false), superscript(false), subscript(false) {}
  ABWCharacterStyle(const std::string &ft, const ABWColor &c, double s, bool b, bool i, bool u, bool du,
                    bool so, bool dso, bool ac, bool ic, bool sc, bool super, bool sub) :
    font(ft), color(c), size(s), bold(b), italic(i), underline(u), doubleunderline(du), strikeout(so),
    doublestrikeout(dso), allcaps(ac), initcaps(ic), smallcaps(sc), superscript(super), subscript(sub) {}
  ABWCharacterStyle(const ABWCharacterStyle &style) :
    font(style.font), color(style.color), size(style.size), bold(style.bold),
    italic(style.italic), underline(style.underline), doubleunderline(style.doubleunderline), strikeout(style.strikeout),
    doublestrikeout(style.doublestrikeout), allcaps(style.allcaps), initcaps(style.initcaps), smallcaps(style.smallcaps),
    superscript(style.superscript), subscript(style.subscript) {}
  ~ABWCharacterStyle() {}

  std::string font;
  ABWColor color;
  double size;
  bool bold;
  bool italic;
  bool underline;
  bool doubleunderline;
  bool strikeout;
  bool doublestrikeout;
  bool allcaps;
  bool initcaps;
  bool smallcaps;
  bool superscript;
  bool subscript;
};

struct ABWParagraphStyle
{
  ABWParagraphStyle() :
    indFirst(0.0), indLeft(0.0), indRight(0.0), spLine(1.0), spBefore(0.0), spAfter(0.0), align(1) {}
  ABWParagraphStyle(double ifst, double il, double ir, double sl, double sb,
                    double sa, unsigned char a) :
    indFirst(ifst), indLeft(il), indRight(ir), spLine(sl), spBefore(sb), spAfter(sa), align(a) {}
  ABWParagraphStyle(const ABWParagraphStyle &style) :
    indFirst(style.indFirst), indLeft(style.indLeft), indRight(style.indRight), spLine(style.spLine),
    spBefore(style.spBefore), spAfter(style.spAfter), align(style.align) {}
  ~ABWParagraphStyle() {}

  double indFirst;
  double indLeft;
  double indRight;
  double spLine;
  double spBefore;
  double spAfter;
  unsigned char align;
};

} // namespace libabw

#endif // __ABWSTYLES_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
