/* Copyright (C) Teemu Suutari */

#include "ACCADecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"

#include <array>

namespace ancient::internal
{

bool ACCADecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("ACCA");
}

std::shared_ptr<XPKDecompressor> ACCADecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<ACCADecompressor>(hdr,recursionLevel,packedData,state,verify);
}

ACCADecompressor::ACCADecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &ACCADecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-ACCA: Andre's code compression algorithm"};
	return name;
}

void ACCADecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
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
			const std::array<uint8_t,16> staticBytes{
				0x00,0x01,0x02,0x03,0x04,0x08,0x10,0x20,
				0x40,0x55,0x60,0x80,0xaa,0xc0,0xe0,0xff};

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
				[[fallthrough]];
				case 14:
				count+=3;
				doRLE=true;
				break;

				case 1:
				count=(count|(uint32_t{readByte()}<<4))+19;
				repeatChar=readByte();
				doRLE=true;
				break;

				case 2:
				repeatChar=staticBytes[count];
				count=2;
				doRLE=true;
				break;

				case 15:
				distance=(count|(uint32_t(readByte())<<4))+3;
				count=uint32_t{readByte()}+14;
				break;

				default: /* 3 to 13 */
				distance=(count|(uint32_t{readByte()}<<4))+3;
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
