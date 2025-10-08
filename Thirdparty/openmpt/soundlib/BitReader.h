/*
 * BitReader.h
 * -----------
 * Purpose: An extended FileReader to read bit-oriented rather than byte-oriented streams.
 * Notes  : The current implementation can only read bit widths up to 32 bits, and it always
 *          reads bits starting from the least significant bit, as this is all that is
 *          required by the class users at the moment.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "../common/FileReader.h"
#include <stdexcept>
#include "mpt/io/base.hpp"


OPENMPT_NAMESPACE_BEGIN


class BitReader : private FileReader
{
protected:
	pos_type m_bufPos = 0;
	pos_type m_bufSize = 0;
	uint32 bitBuf = 0; // Current bit buffer
	int m_bitNum = 0;  // Currently available number of bits
	std::byte buffer[mpt::IO::BUFFERSIZE_TINY]{};

public:

	class eof : public std::range_error
	{
	public:
		eof() : std::range_error("Truncated bit buffer") { }
	};

	BitReader() : FileReader() { }
	BitReader(mpt::span<const std::byte> bytedata) : FileReader(bytedata) { }
	BitReader(const FileCursor &other) : FileReader(other) { }
	BitReader(FileCursor &&other) : FileReader(std::move(other)) { }

	pos_type GetLength() const
	{
		return FileReader::GetLength();
	}

	pos_type GetPosition() const
	{
		return FileReader::GetPosition() - m_bufSize + m_bufPos;
	}

	uint32 ReadBits(int numBits)
	{
		while(m_bitNum < numBits)
		{
			// Fetch more bits
			if(m_bufPos >= m_bufSize)
			{
				m_bufSize = ReadRaw(mpt::as_span(buffer)).size();
				m_bufPos = 0;
				if(!m_bufSize)
				{
					throw eof();
				}
			}
			bitBuf |= (static_cast<uint32>(buffer[m_bufPos++]) << m_bitNum);
			m_bitNum += 8;
		}

		uint32 v = bitBuf & ((1 << numBits) - 1);
		bitBuf >>= numBits;
		m_bitNum -= numBits;
		return v;
	}
};


OPENMPT_NAMESPACE_END
