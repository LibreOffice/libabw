/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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

/**
This class provides all the functions an application would need to parse
AbiWord documents.
*/

class AbiDocument
{
public:
  static bool isFileFormatSupported(librevenge::RVNGInputStream *input);
  static bool parse(librevenge::RVNGInputStream *input, librevenge::RVNGTextInterface *documentInterface);
};

#endif /* ABIDOCUMENT_H */
/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */
