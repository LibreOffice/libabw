/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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

#ifndef PACKAGE
#define PACKAGE "libabw"
#endif
#ifndef VERSION
#define VERSION "UNKNOWN VERSION"
#endif

namespace
{

int printUsage()
{
  printf("`abw2raw' is used to test " PACKAGE ".\n");
  printf("\n");
  printf("Usage: abw2raw [OPTION] INPUT\n");
  printf("\n");
  printf("Options:\n");
  printf("\t--callgraph           display the call graph nesting level\n");
  printf("\t--help                show this help message\n");
  printf("\t--version             show version information\n");
  printf("\n");
  printf("Report bugs to <https://bugs.documentfoundation.org/>.\n");
  return -1;
}

int printVersion()
{
  printf("abw2raw %s\n", VERSION);
  return 0;
}

} // anonymous namespace

int main(int argc, char *argv[])
{
  bool printIndentLevel = false;
  char *file = nullptr;

  if (argc < 2)
    return printUsage();

  for (int i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i], "--callgraph"))
      printIndentLevel = true;
    else if (!strcmp(argv[i], "--version"))
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

  librevenge::RVNGRawTextGenerator documentGenerator(printIndentLevel);
  if (libabw::AbiDocument::parse(&input, &documentGenerator))
    return 0;
  return 1;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
