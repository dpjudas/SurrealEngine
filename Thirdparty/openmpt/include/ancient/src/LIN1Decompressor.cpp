/* Copyright (C) Teemu Suutari */

#include "LIN1Decompressor.hpp"

#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool LIN1Decompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("LIN1") || hdr==FourCC("LIN3");
}

std::shared_ptr<XPKDecompressor> LIN1Decompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<LIN1Decompressor>(hdr,recursionLevel,packedData,state,verify);
}

LIN1Decompressor::LIN1Decompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
	_ver=(hdr==FourCC("LIN1"))?1:3;
	if (packedData.size()<5)
		throw Decompressor::InvalidFormatError();

	uint32_t tmp{packedData.readBE32(0)};
	if (tmp)
		throw Decompressor::InvalidFormatError();	// password set
}

const std::string &LIN1Decompressor::getSubName() const noexcept
{
	static std::string name1{"XPK-LIN1: LIN1 LINO packer"};
	static std::string name3{"XPK-LIN3: LIN3 LINO packer"};
	return (_ver==1)?name1:name3;
}

void LIN1Decompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,5U,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};

	size_t rawSize{rawData.size()};
	ForwardOutputStream outputStream{rawData,0,rawSize};

	while (!outputStream.eof())
	{
		if (!readBits(1U))
		{
			outputStream.writeByte(readByte()^0x55U);
		} else {
			uint32_t count{3U};
			if (readBits(1U))
			{
				count=readBits(2U);
				if (count==3U)
				{
					count=readBits(3U);
					if (count==7U)
					{
						count=readBits(4U);
						if (count==15U)
						{
							count=readByte();
							if (count==0xffU)
								throw Decompressor::DecompressionError();
							count+=3U;
						} else count+=14U;
					} else count+=7U;
				} else count+=4U;
			}
			uint32_t distance{0};
			switch (readBits(2))
			{
				case 0:
				distance=readByte()+1;
				break;

				case 1:
				distance=uint32_t(readBits(2U))<<8U;
				distance|=readByte();
				distance+=0x101U;
				break;

				case 2:
				distance=uint32_t(readBits(4U))<<8U;
				distance|=readByte();
				distance+=0x501U;
				break;

				case 3:
				distance=uint32_t(readBits(6U))<<8U;
				distance|=readByte();
				distance+=0x1501U;
				break;
			}

			// buggy compressors
			count=std::min(count,uint32_t(rawSize-outputStream.getOffset()));
			if (!count)
				throw Decompressor::DecompressionError();

			outputStream.copy(distance,count);
		}
	}
}

}
