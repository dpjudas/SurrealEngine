/* Copyright (C) Teemu Suutari */

#include "FRLEDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool FRLEDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("FRLE");
}

std::shared_ptr<XPKDecompressor> FRLEDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<FRLEDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

FRLEDecompressor::FRLEDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &FRLEDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-FRLE: RLE-compressor"};
	return name;
}

void FRLEDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,0,_packedData.size()};

	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	while (!outputStream.eof())
	{
		auto countMod=[](uint32_t count)->uint32_t
		{
			return (32-(count&0x1f))+(count&0x60);
		};

		if (uint32_t count{inputStream.readByte()};count<128)
		{
			count=countMod(count);
			for (uint32_t i=0;i<count;i++) outputStream.writeByte(inputStream.readByte());
		} else {
			count=countMod(count)+1;
			uint8_t ch{inputStream.readByte()};
			for (uint32_t i=0;i<count;i++) outputStream.writeByte(ch);
		}
	}
}

}
