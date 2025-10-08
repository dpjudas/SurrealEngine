/* Copyright (C) Teemu Suutari */

#include "RDCNDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool RDCNDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("RDCN");
}

std::shared_ptr<XPKDecompressor> RDCNDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<RDCNDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

RDCNDecompressor::RDCNDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &RDCNDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-RDCN: Ross data compression"};
	return name;
}

void RDCNDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,0,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBitsBE16(1);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};

	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	while (!outputStream.eof())
	{
		if (!readBit())
		{
			outputStream.writeByte(readByte());
		} else {
			uint8_t tmp{readByte()};
			uint32_t count{tmp&0xfU};
			uint32_t code{uint32_t(tmp>>4U)};
			uint32_t distance{0};
			uint8_t repeatChar{0};
			bool doRLE{false};
			switch (code)
			{
				case 0:
				repeatChar=readByte();
				count+=3;
				doRLE=true;
				break;

				case 1:
				count=(count|(uint32_t(readByte())<<4U))+19U;
				repeatChar=readByte();
				doRLE=true;
				break;

				case 2:
				distance=(count|(uint32_t(readByte())<<4U))+3U;
				count=uint32_t(readByte())+16U;
				break;

				default: /* 3 to 15 */
				distance=(count|(uint32_t(readByte())<<4U))+3U;
				count=code;
				break;
			}
			if (doRLE)
			{
				for (uint32_t i=0;i<count;i++) outputStream.writeByte(repeatChar);
			} else {
				outputStream.copy(distance,count);
			}
		}
	}
}

}
