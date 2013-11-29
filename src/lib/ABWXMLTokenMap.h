/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ABWXMLTOKENMAP_H__
#define __ABWXMLTOKENMAP_H__

#include <libxml/xmlstring.h>
#include "tokens.h"

namespace libabw
{

class ABWXMLTokenMap
{
public:
  static int getTokenId(const xmlChar *name);
};

} // namespace libabw

#endif /* __ABWXMLTOKENMAP_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
