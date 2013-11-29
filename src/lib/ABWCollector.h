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

namespace libabw
{

class ABWCollector
{
public:
  ABWCollector(::librevenge::RVNGTextInterface *iface);
  virtual ~ABWCollector();

  // collector functions

private:
  ABWCollector(const ABWCollector &);
  ABWCollector &operator=(const ABWCollector &);

  librevenge::RVNGTextInterface *m_iface;
};

} // namespace libabw

#endif /* __ABWCOLLECTOR_H__ */
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
