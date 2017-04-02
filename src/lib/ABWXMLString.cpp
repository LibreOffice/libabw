/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ABWXMLString.h"

#include <libxml/xmlmemory.h>

namespace libabw
{

ABWXMLString::ABWXMLString(xmlChar *xml)
  : m_xml(xml, xmlFree)
{
}

const xmlChar *ABWXMLString::get() const
{
  return m_xml.get();
}

ABWXMLString::operator const char *() const
{
  return reinterpret_cast<const char *>(m_xml.get());
}

}

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
