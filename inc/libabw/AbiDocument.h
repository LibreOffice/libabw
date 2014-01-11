/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ABIDOCUMENT_H
#define ABIDOCUMENT_H

#include <libwpd/libwpd.h>

#ifdef DLL_EXPORT
#ifdef LIBABW_BUILD
#define ABWAPI __declspec(dllexport)
#else
#define ABWAPI __declspec(dllimport)
#endif
#else
#define ABWAPI
#endif

namespace libabw
{

/**
This class provides all the functions an application would need to parse
AbiWord documents.
*/

class AbiDocument
{
public:
  static ABWAPI bool isFileFormatSupported(WPXInputStream *input);
  static ABWAPI bool parse(WPXInputStream *input, WPXDocumentInterface *documentInterface);
};

} // namespace libabw

#endif /* ABIDOCUMENT_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
