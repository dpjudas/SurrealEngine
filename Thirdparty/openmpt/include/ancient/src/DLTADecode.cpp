/* Copyright (C) Teemu Suutari */

#include "DLTADecode.hpp"

#include "common/OverflowCheck.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool DLTADecode::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("DLTA");
}

std::shared_ptr<XPKDecompressor> DLTADecode::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<DLTADecode>(hdr,recursionLevel,packedData,state,verify);
}

DLTADecode::DLTADecode(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr))
		throw Decompressor::InvalidFormatError();
}

const std::string &DLTADecode::getSubName() const noexcept
{
	static std::string name{"XPK-DLTA: Delta encoding"};
	return name;
}

void DLTADecode::decode(Buffer &bufferDest,const Buffer &bufferSrc,size_t offset,size_t size)
{
	uint8_t ctr{0};
	for (size_t i=0;i<size;i++)
	{
		ctr+=bufferSrc[offset+i];
		bufferDest[offset+i]=ctr;
	}
}


void DLTADecode::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	if (rawData.size()<_packedData.size())
		throw Decompressor::DecompressionError();
	decode(rawData,_packedData,0,_packedData.size());
}

}
