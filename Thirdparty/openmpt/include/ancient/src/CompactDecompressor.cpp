/* Copyright (C) Teemu Suutari */

#include "CompactDecompressor.hpp"
#include "DynamicHuffmanDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"

#include <array>

namespace ancient::internal
{

bool CompactDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	// If a compact file has signature of 0x1fff it means the its endianess is wrongly configured
	// (This is easy since it only knows of vax and pdp-11)
	// Files with a wrong endianess are almost salvageable - they just wrongly encode first and last byte
	// Thus supporting them would be in vain
	return (hdr>>16)==0xff1fU;
}

std::shared_ptr<Decompressor> CompactDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<CompactDecompressor>(packedData,exactSizeKnown,verify);
}

CompactDecompressor::CompactDecompressor(const Buffer &packedData,bool exactSizeKnown,bool verify) :
	_packedData{packedData},
	_exactSizeKnown{exactSizeKnown}
{
	if (_packedData.size()<2U)
		throw InvalidFormatError();
	uint32_t hdr{_packedData.readBE16(0)};
	if (!detectHeader(hdr<<16,0))
		throw InvalidFormatError();
	if (exactSizeKnown)
		_packedSize=packedData.size();
}

const std::string &CompactDecompressor::getName() const noexcept
{
	static std::string name{"C: Compact"};
	return name;
}

size_t CompactDecompressor::getPackedSize() const noexcept
{
	// no way to know before decompressing
	return _packedSize;
}


size_t CompactDecompressor::getRawSize() const noexcept
{
	// same thing, decompression needed first
	return _rawSize;
}

void CompactDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	ForwardInputStream inputStream{_packedData,2U,_packedSize?_packedSize:_packedData.size()};

	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};
	auto readBit=[&]()->uint32_t
	{
		// left is right and right is left
		return bitReader.readBits8(1U)^1U;
	};

	AutoExpandingForwardOutputStream outputStream{rawData};
	DynamicHuffmanDecoder<258U> decoder{3U};
	uint32_t codeCount{0};
	std::array<uint16_t,258> mapper;

	// Magic!
	mapper[codeCount++]=256U;
	mapper[codeCount++]=257U;
	mapper[codeCount++]=readBits(8U);
	outputStream.writeByte(uint8_t(mapper[2]));

	for(;;)
	{
		uint32_t code{decoder.decode(readBit)};
		decoder.update(code);
		code=mapper[code];
		if (code==257U)
		{
			if (codeCount==258U)
				throw DecompressionError();
			decoder.addCode();
			decoder.update(codeCount);
			code=mapper[codeCount++]=readBits(8U);
		} else if (code==256U) break;
		outputStream.writeByte(code);
	}
	_rawSize=outputStream.getOffset();
	if (_exactSizeKnown && inputStream.getOffset()!=_packedSize)
		throw DecompressionError();
	_packedSize=inputStream.getOffset();
}

}
