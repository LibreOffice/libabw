/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <zlib.h>
#include "ABWZlibStream.h"
#include <string.h>  // for memcpy
#include <stdio.h>

#define BLOCK_SIZE 16384

namespace libabw
{

namespace
{

static bool getInflatedBuffer(librevenge::RVNGInputStream *input, std::vector<unsigned char> &buffer)
{
  int ret;
  z_stream strm;
  unsigned char in[BLOCK_SIZE];
  unsigned char out[BLOCK_SIZE];

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  ret = inflateInit2(&strm, 16 + MAX_WBITS);
  if (Z_OK != ret)
    return false;

  do
  {
    unsigned long numBytesRead(0);
    const unsigned char *p = input->read(BLOCK_SIZE, numBytesRead);
    strm.avail_in = numBytesRead;
    if (!strm.avail_in)
      break;
    memcpy(in, p, strm.avail_in);
    strm.next_in = in;

    do
    {
      strm.avail_out = BLOCK_SIZE;
      strm.next_out = out;
      ret = inflate(&strm, Z_NO_FLUSH);
      switch (ret)
      {
      case Z_NEED_DICT:
        ret = Z_DATA_ERROR;
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
      case Z_STREAM_ERROR:
        (void)inflateEnd(&strm);
        return false;
      }
      for (unsigned i = 0; i < BLOCK_SIZE - strm.avail_out; ++i)
        buffer.push_back(out[i]);
    }
    while (!strm.avail_out);
  }
  while (Z_STREAM_END != ret);

  (void)inflateEnd(&strm);
  input->seek(0, librevenge::RVNG_SEEK_SET);
  if (Z_STREAM_END == ret)
    return true;
  return false;
}

}

ABWZlibStream::ABWZlibStream(librevenge::RVNGInputStream *input) :
  librevenge::RVNGInputStream(),
  m_input(0),
  m_offset(0),
  m_buffer()
{
  if (!getInflatedBuffer(input, m_buffer))
  {
    if (input)
    {
      input->seek(0, librevenge::RVNG_SEEK_CUR);
      m_input = input;
    }
    else
      m_buffer.clear();
  }
}

const unsigned char *ABWZlibStream::read(unsigned long numBytes, unsigned long &numBytesRead)
{
  if (m_input)
    return m_input->read(numBytes, numBytesRead);

  numBytesRead = 0;

  if (numBytes == 0)
    return 0;

  unsigned numBytesToRead;

  if ((m_offset+numBytes) < m_buffer.size())
    numBytesToRead = numBytes;
  else
    numBytesToRead = m_buffer.size() - m_offset;

  numBytesRead = numBytesToRead; // about as paranoid as we can be..

  if (numBytesToRead == 0)
    return 0;

  long oldOffset = m_offset;
  m_offset += numBytesToRead;

  return &m_buffer[oldOffset];
}

int ABWZlibStream::seek(long offset, librevenge::RVNG_SEEK_TYPE seekType)
{
  if (m_input)
    return m_input->seek(offset, seekType);

  if (seekType == librevenge::RVNG_SEEK_CUR)
    m_offset += offset;
  else if (seekType == librevenge::RVNG_SEEK_SET)
    m_offset = offset;

  if (m_offset < 0)
  {
    m_offset = 0;
    return 1;
  }
  if ((long)m_offset > (long)m_buffer.size())
  {
    m_offset = m_buffer.size();
    return 1;
  }

  return 0;
}

long ABWZlibStream::tell()
{
  if (m_input)
    return m_input->tell();

  return m_offset;
}

bool ABWZlibStream::isEnd()
{
  if (m_input)
    return m_input->isEnd();

  if ((long)m_offset >= (long)m_buffer.size())
    return true;

  return false;
}

} // namespace libabw
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
