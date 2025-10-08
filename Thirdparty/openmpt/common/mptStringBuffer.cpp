/*
 * mptStringBuffer.cpp
 * -------------------
 * Purpose: Various functions for "fixing" char array strings for writing to or
 *          reading from module files, or for securing char arrays in general.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#include "mptStringBuffer.h"


OPENMPT_NAMESPACE_BEGIN



namespace mpt
{

namespace String
{

namespace detail
{

std::string ReadStringBuffer(String::ReadWriteMode mode, const char *srcBuffer, std::size_t srcSize)
{

	std::string dest;
	const char *src = srcBuffer;

	if(mode == nullTerminated || mode == spacePaddedNull)
	{
		// We assume that the last character of the source buffer is null.
		if(srcSize > 0)
		{
			srcSize -= 1;
		}
	}

	if(mode == nullTerminated || mode == maybeNullTerminated)
	{

		// Copy null-terminated string, stopping at null.
		dest.assign(src, std::find(src, src + srcSize, '\0'));

	} else if(mode == spacePadded || mode == spacePaddedNull)
	{

		// Copy string over.
		dest.assign(src, src + srcSize);

		// Convert null characters to spaces.
		std::transform(dest.begin(), dest.end(), dest.begin(), [] (char c) -> char { return (c != '\0') ? c : ' '; });

		// Trim trailing spaces.
		dest = mpt::trim_right(dest, std::string(" "));

	}

	return dest;

}

void WriteStringBuffer(String::ReadWriteMode mode, char *destBuffer, const std::size_t destSize, const char *srcBuffer, const std::size_t srcSize)
{

	MPT_ASSERT(destSize > 0);

	const size_t maxSize = std::min(destSize, srcSize);
	char *dst = destBuffer;
	const char *src = srcBuffer;

	// First, copy over null-terminated string.
	size_t pos = maxSize;
	while(pos > 0)
	{
		if((*dst = *src) == '\0')
		{
			break;
		}
		pos--;
		dst++;
		src++;
	}

	if(mode == nullTerminated || mode == maybeNullTerminated)
	{
		// Fill rest of string with nulls.
		std::fill(dst, dst + destSize - maxSize + pos, '\0');
	} else if(mode == spacePadded || mode == spacePaddedNull)
	{
		// Fill the rest of the destination string with spaces.
		std::fill(dst, dst + destSize - maxSize + pos, ' ');
	}

	if(mode == nullTerminated || mode == spacePaddedNull)
	{
		// Make sure that destination is really null-terminated.
		SetNullTerminator(destBuffer, destSize);
	}

}

} // namespace detail

} // namespace String

} // namespace mpt



OPENMPT_NAMESPACE_END
