/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

  if (!libabw::AbiDocument::isFileFormatSupported(&input))
  {
    fprintf(stderr, "ERROR: Unsupported file format!\n");
    return 1;
  }

  librevenge::RVNGString document;
  librevenge::RVNGHTMLTextGenerator documentGenerator(document);
  if (!libabw::AbiDocument::parse(&input, &documentGenerator))
    return 1;

  printf("%s", document.cstr());

  return 0;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
