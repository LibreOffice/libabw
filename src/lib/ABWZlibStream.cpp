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

#define BLOCK_SIZE 16384

namespace libabw
{

ABWZlibStream::ABWZlibStream(librevenge::RVNGInputStream *input) :
  librevenge::RVNGInputStream(),
  m_input(nullptr),
  m_compressed(input),
  m_compressedStart(input ? input->tell() : 0),
  m_zstream(new z_stream()),
  m_finished(true),
  m_inBlock(BLOCK_SIZE),
  m_block(BLOCK_SIZE),
  m_blockStart(0),
  m_blockSize(0),
  m_offset(0)
{
  m_zstream->zalloc = Z_NULL;
  m_zstream->zfree = Z_NULL;
  m_zstream->opaque = Z_NULL;
  m_zstream->avail_in = 0;
  m_zstream->next_in = Z_NULL;
  if (!input || Z_OK != inflateInit2(m_zstream.get(), 16 + MAX_WBITS))
  {
    m_zstream.reset();
    m_compressed = nullptr;
    m_input = input;
    return;
  }
  // A stream whose first block does not inflate is read as it is.
  m_finished = false;
  if (!inflateNextBlock())
  {
    (void)inflateEnd(m_zstream.get());
    m_zstream.reset();
    m_compressed = nullptr;
    m_input = input;
    input->seek(m_compressedStart, librevenge::RVNG_SEEK_SET);
  }
}

ABWZlibStream::~ABWZlibStream()
{
  if (m_zstream)
    (void)inflateEnd(m_zstream.get());
}

bool ABWZlibStream::restart()
{
  if (Z_OK != inflateReset(m_zstream.get()))
  {
    m_finished = true;
    return false;
  }
  m_zstream->avail_in = 0;
  m_zstream->next_in = Z_NULL;
  m_compressed->seek(m_compressedStart, librevenge::RVNG_SEEK_SET);
  m_finished = false;
  m_blockStart = 0;
  m_blockSize = 0;
  return inflateNextBlock();
}

bool ABWZlibStream::inflateNextBlock()
{
  m_blockStart += long(m_blockSize);
  m_blockSize = 0;
  while (!m_finished && !m_blockSize)
  {
    if (!m_zstream->avail_in)
    {
      unsigned long numBytesRead(0);
      const unsigned char *p = m_compressed->read(BLOCK_SIZE, numBytesRead);
      if (!p || !numBytesRead)
      {
        m_finished = true;
        break;
      }
      memcpy(m_inBlock.data(), p, numBytesRead);
      m_zstream->next_in = m_inBlock.data();
      m_zstream->avail_in = uInt(numBytesRead);
    }
    m_zstream->next_out = m_block.data();
    m_zstream->avail_out = BLOCK_SIZE;
    const int ret = inflate(m_zstream.get(), Z_NO_FLUSH);
    if (Z_OK != ret && Z_STREAM_END != ret)
    {
      m_finished = true;
      break;
    }
    m_blockSize = BLOCK_SIZE - m_zstream->avail_out;
    if (Z_STREAM_END == ret)
      m_finished = true;
  }
  return m_blockSize != 0;
}

// Brings the block that holds m_offset into m_block, or returns false at the end of the data.
bool ABWZlibStream::reachOffset()
{
  if (m_offset < m_blockStart && !restart())
    return false;
  while (m_offset >= m_blockStart + long(m_blockSize))
  {
    if (!inflateNextBlock())
      return false;
  }
  return true;
}

const unsigned char *ABWZlibStream::read(unsigned long numBytes, unsigned long &numBytesRead)
{
  if (m_input)
    return m_input->read(numBytes, numBytesRead);

  numBytesRead = 0;

  if (numBytes == 0 || !m_zstream)
    return nullptr;

  if (!reachOffset())
    return nullptr;

  const unsigned long pos = static_cast<unsigned long>(m_offset - m_blockStart);
  const unsigned long remaining = m_blockSize - pos;
  numBytesRead = numBytes < remaining ? numBytes : remaining;
  m_offset += long(numBytesRead);
  return &m_block[pos];
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
  if (m_zstream && !reachOffset())
  {
    // The data ends before the requested offset, so stop at its end.
    m_offset = m_blockStart + long(m_blockSize);
    return m_offset == offset ? 0 : 1;
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

  return !m_zstream || !reachOffset();
}

} // namespace libabw
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
