/* Copyright (C) Teemu Suutari */

#include "FASTDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool FASTDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("FAST");
}

std::shared_ptr<XPKDecompressor> FASTDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<FASTDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

FASTDecompressor::FASTDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &FASTDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-FAST: Fast LZ77 compressor"};
	return name;
}

void FASTDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream forwardInputStream{_packedData,0,_packedData.size()};
	BackwardInputStream backwardInputStream{_packedData,0,_packedData.size()};
	forwardInputStream.link(backwardInputStream);
	backwardInputStream.link(forwardInputStream);
	MSBBitReader<BackwardInputStream> bitReader{backwardInputStream};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBitsBE16(1);
	};
	auto readByte=[&]()->uint8_t
	{
		return forwardInputStream.readByte();
	};
	auto readShort=[&]()->uint16_t
	{
		return backwardInputStream.readBE16();
	};

	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	while (!outputStream.eof())
	{
		if (!readBit())
		{
			outputStream.writeByte(readByte());
		} else {
			uint16_t ld{readShort()};
			uint32_t count{std::min(18U-(ld&0xf),uint32_t(outputStream.getEndOffset()-outputStream.getOffset()))};
			uint32_t distance{uint32_t(ld>>4U)};
			outputStream.copy(distance,count);
		}
	}
}

}
