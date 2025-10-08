/* Copyright (C) Teemu Suutari */

#include "LZBSDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool LZBSDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("LZBS");
}

std::shared_ptr<XPKDecompressor> LZBSDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<LZBSDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

LZBSDecompressor::LZBSDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr) || _packedData.size()<1)
		throw Decompressor::InvalidFormatError();
}

const std::string &LZBSDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-LZBS: LZBS CyberYAFA compressor"};
	return name;
}

void LZBSDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,1,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return rotateBits(bitReader.readBits8(count),count);
	};

	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	uint32_t bits{0};
	uint32_t maxBits{_packedData[0]};
	while (!outputStream.eof())
	{
		if (!readBits(1))
		{
			outputStream.writeByte(readBits(8));
		} else {
			uint32_t count{readBits(8)+2};
			if (count==2)
			{
				count=readBits(12);
				if (!count)
					throw Decompressor::DecompressionError();
				for (uint32_t i=0;i<count;i++)
					outputStream.writeByte(readBits(8));
			} else {
				while (outputStream.getOffset()>=(1ULL<<bits) && bits<maxBits) bits++;
				uint32_t distance=readBits(bits);

				outputStream.copy(distance,count);
			}
		}
	}
}

}
