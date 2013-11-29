/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ABWXMLHELPER_H__
#define __ABWXMLHELPER_H__

#include <librevenge-stream/librevenge-stream.h>
#include <libxml/xmlreader.h>

namespace libabw
{

// create an xmlTextReader pointer from a librevenge::RVNGInputStream pointer
// needs to be freed using xmlTextReaderFree function.

xmlTextReaderPtr xmlReaderForStream(librevenge::RVNGInputStream *input,
                                    const char *URL,
                                    const char *encoding,
                                    int options);

} // namespace libabw

#endif // __ABWXMLHELPER_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
