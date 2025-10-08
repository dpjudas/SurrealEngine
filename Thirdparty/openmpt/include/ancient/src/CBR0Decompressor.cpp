/* Copyright (C) Teemu Suutari */

#include "CBR0Decompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool CBR0Decompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	// CBR1 is practical joke: it is the same as CBR0 but with ID changed
	return hdr==FourCC("CBR0") || hdr==FourCC("CBR1");
}

std::shared_ptr<XPKDecompressor> CBR0Decompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<CBR0Decompressor>(hdr,recursionLevel,packedData,state,verify);
}

CBR0Decompressor::CBR0Decompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData},
	_isCBR0{hdr==FourCC("CBR0")}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &CBR0Decompressor::getSubName() const noexcept
{
	static std::string nameCBR0{"XPK-CBR0: RLE-compressor"};
	static std::string nameCBR1{"XPK-CBR1: RLE-compressor"};
	return (_isCBR0)?nameCBR0:nameCBR1;
}

void CBR0Decompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,0,_packedData.size()};
	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	// barely different than RLEN, however the count is well defined here.
	while (!outputStream.eof())
	{
		if (uint32_t count{inputStream.readByte()};count<128)
		{
			count++;
			for (uint32_t i=0;i<count;i++) outputStream.writeByte(inputStream.readByte());
		} else {
			count=257-count;
			uint8_t ch=inputStream.readByte();
			for (uint32_t i=0;i<count;i++) outputStream.writeByte(ch);
		}
	}
}

}
