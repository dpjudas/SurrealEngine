/* Copyright (C) Teemu Suutari */

#include "FBR2Decompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool FBR2Decompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("FBR2");
}

std::shared_ptr<XPKDecompressor> FBR2Decompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<FBR2Decompressor>(hdr,recursionLevel,packedData,state,verify);
}

FBR2Decompressor::FBR2Decompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();;
}

const std::string &FBR2Decompressor::getSubName() const noexcept
{
	static std::string name{"XPK-FBR2: FBR2 CyberYAFA compressor"};
	return name;
}

void FBR2Decompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,0,_packedData.size()};

	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	uint8_t mode=inputStream.readByte();
	while (!outputStream.eof())
	{
		bool doCopy{false};
		uint32_t count{0};
		switch (mode)
		{
			case 33:
			count=uint32_t(inputStream.readByte())<<24;
			count|=uint32_t(inputStream.readByte())<<16;
			count|=uint32_t(inputStream.readByte())<<8;
			count|=uint32_t(inputStream.readByte());
			if (count>=0x8000'0000)
			{
				doCopy=true;
				count=0-count;
			}
			break;

			case 67:
			count=uint32_t(inputStream.readByte())<<8;
			count|=uint32_t(inputStream.readByte());
			if (count>=0x8000)
			{
				doCopy=true;
				count=0x10000-count;
			}
			break;

			case 100:
			count=uint32_t(inputStream.readByte());
			if (count>=0x80)
			{
				doCopy=true;
				count=0x100-count;
			}
			break;

			default:
			throw Decompressor::DecompressionError();
		}

		count++;
		if (doCopy) {
			for (uint32_t i=0;i<count;i++) outputStream.writeByte(inputStream.readByte());
		} else {
			uint8_t repeatChar{inputStream.readByte()};
			for (uint32_t i=0;i<count;i++) outputStream.writeByte(repeatChar);
		}
	}
}

}
