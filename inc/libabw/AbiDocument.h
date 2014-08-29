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

#include <librevenge/librevenge.h>

#ifdef DLL_EXPORT
#ifdef LIBABW_BUILD
#define ABWAPI __declspec(dllexport)
#else
#define ABWAPI __declspec(dllimport)
#endif
#else // !DLL_EXPORT
#ifdef LIBABW_VISIBILITY
#define ABWAPI __attribute__((visibility("default")))
#else
#define ABWAPI
#endif
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
  static ABWAPI bool isFileFormatSupported(librevenge::RVNGInputStream *input);
  static ABWAPI bool parse(librevenge::RVNGInputStream *input, librevenge::RVNGTextInterface *documentInterface);
};

} // namespace libabw

#endif /* ABIDOCUMENT_H */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
