/* Copyright (C) Teemu Suutari */

#include "MASHDecompressor.hpp"
#include "HuffmanDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "VariableLengthCodeDecoder.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool MASHDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("MASH");
}

std::shared_ptr<XPKDecompressor> MASHDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<MASHDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

MASHDecompressor::MASHDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &MASHDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-MASH: LZRW-compressor"};
	return name;
}

void MASHDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,0,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1U);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};

	size_t rawSize{rawData.size()};
	ForwardOutputStream outputStream{rawData,0,rawSize};

	HuffmanDecoder<uint32_t> litDecoder
	{
		HuffmanCode{1,0b000000,0U},
		HuffmanCode{2,0b000010,1U},
		HuffmanCode{3,0b000110,2U},
		HuffmanCode{4,0b001110,3U},
		HuffmanCode{5,0b011110,4U},
		HuffmanCode{6,0b111110,5U},
		HuffmanCode{6,0b111111,6U}
	};

	VariableLengthCodeDecoder vlcDecoder{5,7,9,10,11,12,13,14};

	while (!outputStream.eof())
	{
		uint32_t litLength{litDecoder.decode(readBit)};
		if (litLength==6)
		{
			uint32_t litBits;
			for (litBits=1;litBits<=17;litBits++)
				if (!readBit()) break;
			if (litBits==17)
				throw Decompressor::DecompressionError();
			litLength=readBits(litBits)+(1<<litBits)+4;
		}
		
		for (uint32_t i=0;i<litLength;i++) outputStream.writeByte(readByte());

		uint32_t count,distance;
		if (readBit())
		{
			uint32_t countBits;
			for (countBits=1;countBits<=16;countBits++) if (!readBit()) break;
			if (countBits==16)
				throw Decompressor::DecompressionError();
			count=readBits(countBits)+(1<<countBits)+2;
			distance=vlcDecoder.decode(readBits,readBits(3));
		} else {
			if (readBit())
			{
				distance=vlcDecoder.decode(readBits,readBits(3));
				count=3;
			} else {
				distance=readBits(9);
				count=2;
			}
		}
		// hacks to make it work
		if (!distance && outputStream.eof()) break;
		// zero distance when we are at the end of the stream...
		// there seems to be almost systematic extra one byte at the end of the stream...
		count=std::min(count,uint32_t(rawSize-outputStream.getOffset()));
		outputStream.copy(distance,count);
	}
}

}
