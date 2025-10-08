/* Copyright (C) Teemu Suutari */

#include <cstring>

#include "common/SubBuffer.hpp"
#include "CYB2Decoder.hpp"
#include "XPKMain.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool CYB2Decoder::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("CYB2");
}

std::shared_ptr<XPKDecompressor> CYB2Decoder::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<CYB2Decoder>(hdr,recursionLevel,packedData,state,verify);
}

CYB2Decoder::CYB2Decoder(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor(recursionLevel),
	_packedData(packedData)
{
	if (!detectHeaderXPK(hdr) || _packedData.size()<=10)
		throw Decompressor::InvalidFormatError();
	_blockHeader=_packedData.readBE32(0);
	// after the block header, the next 6 bytes seem to be
	// 00 64 00 00 00 00
	// Those bytes do not seem to be terribly important though...

	if (verify)
	{
		// trigger child checks...
		ConstSubBuffer blockData{_packedData,10,_packedData.size()-10};
		std::shared_ptr<XPKDecompressor::State> subState;
		auto sub=XPKMain::createDecompressor(_blockHeader,_recursionLevel+1,blockData,subState,true);
	}
}

const std::string &CYB2Decoder::getSubName() const noexcept
{
	static std::string name{"XPK-CYB2: xpkCybPrefs container"};
	return name;
}

void CYB2Decoder::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ConstSubBuffer blockData{_packedData,10,_packedData.size()-10};
	std::shared_ptr<XPKDecompressor::State> state;
	auto sub{XPKMain::createDecompressor(_blockHeader,_recursionLevel+1,blockData,state,verify)};
	sub->decompressImpl(rawData,previousData,verify);
}

}
