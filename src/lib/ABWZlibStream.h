/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libabw project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ABWZLIBSTREAM_H__
#define __ABWZLIBSTREAM_H__

#include <memory>
#include <vector>
#include <librevenge-stream/librevenge-stream.h>

struct z_stream_s;

namespace libabw
{

class ABWZlibStream : public librevenge::RVNGInputStream
{
public:
  ABWZlibStream(librevenge::RVNGInputStream *input);
  ~ABWZlibStream() override;

  bool isStructured() override
  {
    return false;
  }
  unsigned subStreamCount() override
  {
    return 0;
  }
  const char *subStreamName(unsigned) override
  {
    return nullptr;
  }
  bool existsSubStream(const char *) override
  {
    return false;
  }
  librevenge::RVNGInputStream *getSubStreamByName(const char *) override
  {
    return nullptr;
  }
  librevenge::RVNGInputStream *getSubStreamById(unsigned) override
  {
    return nullptr;
  }
  const unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead) override;
  int seek(long offset, librevenge::RVNG_SEEK_TYPE seekType) override;
  long tell() override;
  bool isEnd() override;
private:
  bool restart();
  bool inflateNextBlock();
  bool reachOffset();

  //! the stream itself when it is not gzip compressed, otherwise null
  librevenge::RVNGInputStream *m_input;
  //! the gzip compressed stream, otherwise null
  librevenge::RVNGInputStream *m_compressed;
  long m_compressedStart;
  std::unique_ptr<z_stream_s> m_zstream;
  bool m_finished;
  std::vector<unsigned char> m_inBlock;
  //! the most recently inflated block, which starts at m_blockStart in the uncompressed data
  std::vector<unsigned char> m_block;
  long m_blockStart;
  unsigned long m_blockSize;
  long m_offset;
  ABWZlibStream(const ABWZlibStream &);
  ABWZlibStream &operator=(const ABWZlibStream &);
};

} // namespace libabw

#endif // __ABWZLIBSTREAM_H__
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
