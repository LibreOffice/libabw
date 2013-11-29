/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include <stdio.h>
#include <string.h>
#include <librevenge-stream/librevenge-stream.h>
#include <librevenge-generators/librevenge-generators.h>
#include <libabw/libabw.h>

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
  printf("Usage: abw2text [OPTION] <AbiWord Document>\n");
  printf("\n");
  printf("Options:\n");
  printf("--info                Display document metadata instead of the text\n");
  printf("--help                Shows this help message\n");
  printf("--version             Output abw2text version\n");
  return -1;
}

int printVersion()
{
  printf("abw2text %s\n", VERSION);
  return 0;
}

} // anonymous namespace

int main(int argc, char *argv[])
{
  if (argc < 2)
    return printUsage();

  char *szInputFile = 0;
  bool isInfo = false;

  for (int i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i], "--info"))
      isInfo = true;
    else if (!strcmp(argv[i], "--version"))
      return printVersion();
    else if (!szInputFile && strncmp(argv[i], "--", 2))
      szInputFile = argv[i];
    else
      return printUsage();
  }

  if (!szInputFile)
    return printUsage();

  librevenge::RVNGFileStream input(szInputFile);

  if (!AbiDocument::isFileFormatSupported(&input))
  {
    fprintf(stderr, "ERROR: Unsupported file format!\n");
    return 1;
  }

  librevenge::RVNGString document;
  librevenge::RVNGTextTextGenerator documentGenerator(document, isInfo);
  if (!AbiDocument::parse(&input, &documentGenerator))
    return 1;

  printf("%s", document.cstr());

  return 0;
}
/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */
