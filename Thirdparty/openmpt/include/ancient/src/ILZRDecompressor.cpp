/* Copyright (C) Teemu Suutari */

#include "ILZRDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool ILZRDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("ILZR");
}

std::shared_ptr<XPKDecompressor> ILZRDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<ILZRDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

ILZRDecompressor::ILZRDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr) || packedData.size()<2)
		throw Decompressor::InvalidFormatError();
	_rawSize=_packedData.readBE16(0);
	if (!_rawSize)
		throw Decompressor::InvalidFormatError();
}

const std::string &ILZRDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-ILZR: Incremental Lempel-Ziv-Renau compressor"};
	return name;
}

void ILZRDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	if (rawData.size()!=_rawSize)
		throw Decompressor::DecompressionError();

	ForwardInputStream inputStream{_packedData,2,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};

	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	uint32_t bits{8};
	while (!outputStream.eof())
	{
		if (readBits(1))
		{
			outputStream.writeByte(readBits(8));
		} else {
			while (outputStream.getOffset()>(1ULL<<bits)) bits++;
			uint32_t position{readBits(bits)};
			uint32_t count{readBits(4)+3};

			if (position>=outputStream.getOffset())
				throw Decompressor::DecompressionError();
			outputStream.copy(outputStream.getOffset()-position,count);
		}
	}
}

}
