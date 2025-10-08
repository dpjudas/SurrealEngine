/* Copyright (C) Teemu Suutari */

#include <cstring>

#include "NONEDecompressor.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool NONEDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("NONE");
}

std::shared_ptr<XPKDecompressor> NONEDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<NONEDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

NONEDecompressor::NONEDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &NONEDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-NONE: Null compressor"};
	return name;
}

void NONEDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	if (rawData.size()!=_packedData.size())
		throw Decompressor::DecompressionError();

	std::memcpy(rawData.data(),_packedData.data(),_packedData.size());
}

}
