/* Copyright (C) Teemu Suutari */

#include "RLENDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool RLENDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("RLEN");
}

std::shared_ptr<XPKDecompressor> RLENDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<RLENDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

RLENDecompressor::RLENDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &RLENDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-RLEN: RLE-compressor"};
	return name;
}

void RLENDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,0,_packedData.size()};
	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	while (!outputStream.eof())
	{
		uint32_t count{inputStream.readByte()};
		if (count<128)
		{
			if (!count)
				throw Decompressor::DecompressionError();	// lets have this as error...
			for (uint32_t i=0;i<count;i++) outputStream.writeByte(inputStream.readByte());
		} else {
			// I can see from different implementations that count=0x80 is buggy...
			// lets try to have it more or less correctly here
			count=256-count;
			uint8_t ch{inputStream.readByte()};
			for (uint32_t i=0;i<count;i++)
				outputStream.writeByte(ch);
		}
	}
}

}
