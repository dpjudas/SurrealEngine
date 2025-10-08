/* Copyright (C) Teemu Suutari */

#include <cstring>

#include "common/SubBuffer.hpp"
#include "SDHCDecompressor.hpp"
#include "XPKMain.hpp"
#include "DLTADecode.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool SDHCDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("SDHC");
}

std::shared_ptr<XPKDecompressor> SDHCDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<SDHCDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

SDHCDecompressor::SDHCDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr) || _packedData.size()<2)
		throw Decompressor::InvalidFormatError();
	_mode=_packedData.readBE16(0);
	if (verify && (_mode&0x8000U))
	{
		ConstSubBuffer src{_packedData,2U,_packedData.size()-2U};
		XPKMain::createDecompressor(_recursionLevel+1,src,true);
	}
}

const std::string &SDHCDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-SDHC: Sample delta huffman compressor"};
	return name;
}

void SDHCDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ConstSubBuffer src{_packedData,2U,_packedData.size()-2U};
	if (_mode&0x8000U)
	{
		auto main=XPKMain::createDecompressor(_recursionLevel+1,src,verify);
		main->decompress(rawData,verify);
	} else {
		if (src.size()!=rawData.size())
			throw Decompressor::DecompressionError();
		std::memcpy(rawData.data(),src.data(),src.size());
	}

	size_t length{rawData.size()&~3U};

	auto deltaDecodeMono=[&]()
	{
		uint16_t ctr{0};
		for (size_t i=0;i<length;i+=2)
		{
			uint16_t tmp;
			tmp=(uint16_t(rawData[i])<<8U)|rawData[i+1U];
			ctr+=tmp;
			rawData[i]=ctr>>8U;
			rawData[i+1U]=ctr&0xffU;
		}
	};

	auto deltaDecodeStereo=[&]()
	{
		uint16_t ctr1{0};
		uint16_t ctr2{0};
		for (size_t i=0;i<length;i+=4)
		{
			uint16_t tmp{uint16_t((uint16_t(rawData[i])<<8U)|rawData[i+1U])};
			ctr1+=tmp;
			tmp=(uint16_t(rawData[i+2U])<<8U)|rawData[i+3U];
			ctr2+=tmp;
			rawData[i]=ctr1>>8U;
			rawData[i+1U]=ctr1&0xffU;
			rawData[i+2U]=ctr2>>8U;
			rawData[i+3U]=ctr2&0xffU;
		}
	};

	switch (_mode&15)
	{
		case 1:
		DLTADecode::decode(rawData,rawData,0,length);
		[[fallthrough]];
		case 0:
		DLTADecode::decode(rawData,rawData,0,length);
		break;
		
		case 3:
		deltaDecodeMono();
		[[fallthrough]];
		case 2:
		deltaDecodeMono();
		break;

		case 11:
		deltaDecodeStereo();
		[[fallthrough]];
		case 10:
		deltaDecodeStereo();
		break;

		default:
		throw Decompressor::DecompressionError();
	}
}

}
