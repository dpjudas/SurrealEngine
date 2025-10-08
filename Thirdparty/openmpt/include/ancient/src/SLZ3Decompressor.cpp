/* Copyright (C) Teemu Suutari */

#include "SLZ3Decompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool SLZ3Decompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("SLZ3");
}

std::shared_ptr<XPKDecompressor> SLZ3Decompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<SLZ3Decompressor>(hdr,recursionLevel,packedData,state,verify);
}

SLZ3Decompressor::SLZ3Decompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &SLZ3Decompressor::getSubName() const noexcept
{
	static std::string name{"XPK-SLZ3: SLZ3 CyberYAFA compressor"};
	return name;
}

void SLZ3Decompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,0,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};

	ForwardOutputStream outputStream(rawData,0,rawData.size());

	while (!outputStream.eof())
	{
		if (!readBit())
		{
			outputStream.writeByte(readByte());
		} else {
			uint8_t tmp{readByte()};
			if (!tmp)
				throw Decompressor::DecompressionError();
			uint32_t distance={uint32_t(tmp&0xf0U)<<4U};
			distance|=readByte();
			uint32_t count{uint32_t(tmp&0xfU)+2U};
			outputStream.copy(distance,count);
		}
	}
}

}
