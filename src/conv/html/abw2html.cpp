/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* libabw
 * Version: MPL 2.0 / LGPLv2.1+
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Major Contributor(s):
 * Copyright (C) 2002-2003 William Lachance (wrlach@gmail.com)
 * Copyright (C) 2002-2004 Marc Maurer (uwog@uwog.net)
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Lesser General Public License Version 2.1 or later
 * (LGPLv2.1+), in which case the provisions of the LGPLv2.1+ are
 * applicable instead of those above.
 *
 * For further information visit http://libabw.sourceforge.net
 */

#include <stdio.h>
#include <librevenge-stream/librevenge-stream.h>
#include <librevenge-generators/librevenge-generators.h>
#include <libabw/libabw.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef VERSION
#define VERSION "UNKNOWN VERSION"
#endif

namespace
{

int printUsage()
{
  printf("Usage: abw2html [OPTION] <AbiWord Document>\n");
  printf("\n");
  printf("Options:\n");
  printf("--help                Shows this help message\n");
  printf("--version             Output abw2html version \n");
  return -1;
}

int printVersion()
{
  printf("abw2html %s\n", VERSION);
  return 0;
}

} // anonymous namespace

int main(int argc, char *argv[])
{
  if (argc < 2)
    return printUsage();

  char *file = 0;

  for (int i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i], "--version"))
      return printVersion();
    else if (!file && strncmp(argv[i], "--", 2))
      file = argv[i];
    else
      return printUsage();
  }

  if (!file)
    return printUsage();

  librevenge::RVNGFileStream input(file);

  if (!AbiDocument::isFileFormatSupported(&input))
  {
    fprintf(stderr, "ERROR: Unsupported file format!\n");
    return 1;
  }

  librevenge::RVNGString document;
  librevenge::RVNGHTMLTextGenerator documentGenerator(document);
  if (!AbiDocument::parse(&input, &documentGenerator))
    return 1;

  printf("%s", document.cstr());

  return 0;
}
/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */
