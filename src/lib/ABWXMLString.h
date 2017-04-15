/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ABWXMLSTRING_H__
#define __ABWXMLSTRING_H__

#include <memory>

#include <libxml/xmlstring.h>

namespace libabw
{

// An exception-safe wrapper around xmlChar *
class ABWXMLString
{
public:
  ABWXMLString(xmlChar *xml);

  const xmlChar *get() const;

  operator const char *() const;

private:
  std::shared_ptr<xmlChar> m_xml;
};

}

#endif /* __ABWXMLSTRING_H__ */

/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
