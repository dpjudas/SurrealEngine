/*
 * GzipWriter.h
 * ------------
 * Purpose: Simple wrapper around zlib's Gzip writer
 * Notes  : miniz doesn't implement Gzip writing, so this is only compatible with zlib.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"

#include "mptString.h"

#ifdef MPT_WITH_ZLIB

#include <ctime>

#include <zlib.h>

OPENMPT_NAMESPACE_BEGIN

inline void WriteGzip(std::ostream &output, std::string &outData, const mpt::ustring &fileName)
{
	z_stream strm{};
	int zlib_errc = Z_OK;
	strm.avail_in = static_cast<uInt>(outData.size());
	strm.next_in = reinterpret_cast<Bytef *>(outData.data());
	zlib_errc = deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, 15 | 16, 9, Z_DEFAULT_STRATEGY);
	if(zlib_errc == Z_MEM_ERROR)
	{
		mpt::throw_out_of_memory();
	} else if(zlib_errc < Z_OK)
	{
		throw std::runtime_error{"zlib: deflateInit2() failed"};
	}
	gz_header gzHeader{};
	gzHeader.time = static_cast<uLong>(std::time(nullptr));
	std::string filenameISO = mpt::ToCharset(mpt::Charset::ISO8859_1, fileName);
	gzHeader.name = reinterpret_cast<Bytef *>(filenameISO.data());
	zlib_errc = deflateSetHeader(&strm, &gzHeader);
	if(zlib_errc == Z_MEM_ERROR)
	{
		deflateEnd(&strm);
		mpt::throw_out_of_memory();
	} else if(zlib_errc < Z_OK)
	{
		deflateEnd(&strm);
		throw std::runtime_error{"zlib: deflateSetHeader() failed"};
	}
	try
	{
		do
		{
			std::array<Bytef, mpt::IO::BUFFERSIZE_TINY> buffer;
			strm.avail_out = static_cast<uInt>(buffer.size());
			strm.next_out = buffer.data();
			zlib_errc = deflate(&strm, Z_FINISH);
			if(zlib_errc == Z_BUF_ERROR)
			{
				// expected
			} else if(zlib_errc == Z_MEM_ERROR)
			{
				mpt::throw_out_of_memory();
			} else if(zlib_errc < Z_OK)
			{
				throw std::runtime_error{"zlib: deflate() failed"};
			}
			mpt::IO::WritePartial(output, buffer, buffer.size() - strm.avail_out);
		} while(strm.avail_out == 0);
	} catch(mpt::out_of_memory e)
	{
		deflateEnd(&strm);
		mpt::rethrow_out_of_memory(e);
	} catch(const std::exception &)
	{
		deflateEnd(&strm);
		throw;
	} catch(...)
	{
		deflateEnd(&strm);
		throw;
	}
	deflateEnd(&strm);
}

OPENMPT_NAMESPACE_END

#endif
