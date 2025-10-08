/* Copyright (C) Teemu Suutari */

#include "IMPDecompressor.hpp"
#include "HuffmanDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"
#include "common/OverflowCheck.hpp"

#include <array>

namespace ancient::internal
{

static bool readIMPHeader(uint32_t hdr,uint32_t &addition) noexcept
{
	switch (hdr)
	{
		case FourCC("ATN!"):	// Team 17 games
		[[fallthrough]];
		case FourCC("EDAM"):	// Indy Heat
		[[fallthrough]];
		case FourCC("IMP!"):	// Original!!
		[[fallthrough]];
		case FourCC("M.H."):	// Georg Glaxo
		addition=7;
		return true;

		case FourCC("BDPI"):	// Dizzy's excellent adventures
		addition=0x6e8;
		return true;

		case FourCC("CHFI"):	// Dizzy's excellent adventures, Bubble and Squeak
		addition=0xfe4;
		return true;

		case FourCC("RDC9"):	// Telekommando 2 (Files do not contain checksum)
		[[fallthrough]];
		// I haven't got these files to be sure what is the addition
		case FourCC("Dupa"):
		[[fallthrough]];
		case FourCC("FLT!"):
		[[fallthrough]];
		case FourCC("PARA"):
		addition=0; 		// disable checksum for now
		return true;

		default:
		return false;
	}
}

bool IMPDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	uint32_t dummy;
	return readIMPHeader(hdr,dummy);
}

bool IMPDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("IMPL");
}

std::shared_ptr<Decompressor> IMPDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<IMPDecompressor>(packedData,verify);
}

std::shared_ptr<XPKDecompressor> IMPDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<IMPDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

IMPDecompressor::IMPDecompressor(const Buffer &packedData,bool verify) :
	_packedData{packedData}
{
	uint32_t hdr{packedData.readBE32(0)};
	uint32_t checksumAddition;
	if (!readIMPHeader(hdr,checksumAddition) || packedData.size()<0x32)
		throw InvalidFormatError();

	_rawSize=packedData.readBE32(4);
	_endOffset=packedData.readBE32(8);
	if ((_endOffset&1) || _endOffset<0xc || _endOffset+0x32>packedData.size() ||
		!_rawSize || !_endOffset || _rawSize>getMaxRawSize() || _endOffset>getMaxPackedSize())
		throw InvalidFormatError();
	uint32_t checksum{packedData.readBE32(_endOffset+0x2e)};
	if (verify && checksumAddition)
	{
		// size is divisible by 2
		uint32_t sum{checksumAddition};
		for (uint32_t i=0;i<_endOffset+0x2e;i+=2)
			sum+=uint32_t(_packedData.readBE16(i));
		if (checksum!=sum)
			throw InvalidFormatError();
	}
}

IMPDecompressor::IMPDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr) || packedData.size()<0x2e)
		throw InvalidFormatError();

	_rawSize=packedData.readBE32(4);
	_endOffset=packedData.readBE32(8);
	if ((_endOffset&1) || _endOffset<0xc || OverflowCheck::sum(_endOffset,0x2eU)>packedData.size())
		throw InvalidFormatError();
	_isXPK=true;
}

const std::string &IMPDecompressor::getName() const noexcept
{
	static std::string name{"IMP: File Imploder"};
	return name;
}

const std::string &IMPDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-IMPL: File Imploder"};
	return name;
}

size_t IMPDecompressor::getPackedSize() const noexcept
{
	return _endOffset+0x32;
}

size_t IMPDecompressor::getRawSize() const noexcept
{
	return _rawSize;
}

void IMPDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	if (rawData.size()<_rawSize)
		throw DecompressionError();

	class IMPInputStream
	{
	public:
		IMPInputStream(const Buffer &buffer,size_t startOffset,size_t endOffset) :
			_buffer{buffer},
			_currentOffset{endOffset},
			_endOffset{startOffset},
			_refOffset{endOffset}
		{
			if (_currentOffset<_endOffset || _currentOffset>buffer.size() || _endOffset>buffer.size())
				throw Decompressor::DecompressionError();
			uint8_t markerByte{buffer.read8(_currentOffset+16)};
			if (!(markerByte&0x80))
			{
				if (_currentOffset==_endOffset)
					throw Decompressor::DecompressionError();
				_currentOffset--;
			}
		}
		~IMPInputStream() noexcept=default;

		uint8_t readByte()
		{
			// streamreader with funny ordering
			auto sourceOffset=[&](size_t i)->size_t
			{
				if (i>=12)
				{
					return i;
				} else {
					if (i<4)
					{
						return i+_refOffset+8;
					} else if (i<8) {
						return i+_refOffset;
					} else {
						return i+_refOffset-8;
					}
				}
			};
			if (_currentOffset<=_endOffset)
				throw Decompressor::DecompressionError();
			return _buffer[sourceOffset(--_currentOffset)];
		}

		bool eof() const { return _currentOffset==_endOffset; }

	private:
		const Buffer		&_buffer;
		size_t			_currentOffset;
		size_t			_endOffset;
		size_t			_refOffset;
	};

	IMPInputStream inputStream{_packedData,0,_endOffset};
	MSBBitReader<IMPInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};
	// the anchor-bit does not seem always to be at the correct place
	{
		uint8_t halfByte{_packedData.read8(_endOffset+17)};
		for (uint32_t i=0;i<7;i++)
			if (halfByte&(1<<i))
			{
				bitReader.reset(halfByte>>(i+1),7-i);
				break;
			}
	}

	BackwardOutputStream outputStream{rawData,0,_rawSize};

	// tables
	std::array<std::array<uint16_t,4>,2> distanceValues;
	for (uint32_t i=0;i<8;i++)
		distanceValues[i>>2][i&3]=_packedData.readBE16(_endOffset+18+i*2);
	std::array<std::array<uint8_t,4>,3> distanceBits;
	for (uint32_t i=0;i<12;i++)
		distanceBits[i>>2][i&3]=_packedData.read8(_endOffset+34+i);

	// length, distance & literal counts are all intertwined
	HuffmanDecoder<uint8_t> lldDecoder
	{
		HuffmanCode{1,0b00000,uint8_t{0}},
		HuffmanCode{2,0b00010,uint8_t{1}},
		HuffmanCode{3,0b00110,uint8_t{2}},
		HuffmanCode{4,0b01110,uint8_t{3}},
		HuffmanCode{5,0b11110,uint8_t{4}},
		HuffmanCode{5,0b11111,uint8_t{5}}
	};

	HuffmanDecoder<uint8_t> lldDecoder2
	{
		HuffmanCode{1,0b00,uint8_t{0}},
		HuffmanCode{2,0b10,uint8_t{1}},
		HuffmanCode{2,0b11,uint8_t{2}}
	};

	const std::array<uint8_t,4> literalLengths{6,10,10,18};
	const std::array<std::array<uint8_t,4>,3> literalBits{{
		{1,1,1,1},
		{2,3,3,4},
		{4,5,7,14}}};

	uint32_t litLength{_packedData.readBE32(_endOffset+12)};
	for (;;)
	{
		for (uint32_t i=0;i<litLength;i++) outputStream.writeByte(readByte());

		if (outputStream.eof()) break;

		// now the intertwined Huffman table reads.
		uint32_t i0{lldDecoder.decode(readBit)};
		uint32_t selector{(i0<4)?i0:3};
		uint32_t count{i0+2};
		if (count==6)
		{
			count+=readBits(3);
		} else if (count==7) {
			count=readByte();
			// why this is error? (Well, it just is)
			if (!count)
				throw DecompressionError();
		}

		uint32_t i1{lldDecoder2.decode(readBit)};
		litLength=i1+i1;
		if (litLength==4)
		{
			litLength=literalLengths[selector];
		}
		litLength+=readBits(literalBits[i1][selector]);

		uint32_t i2{lldDecoder2.decode(readBit)};
		uint32_t distance{1+((i2)?distanceValues[i2-1][selector]:0)+readBits(distanceBits[i2][selector])};

		outputStream.copy(distance,count);
	}
}

void IMPDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	if (_rawSize!=rawData.size())
		throw DecompressionError();
	return decompressImpl(rawData,verify);
}

}
