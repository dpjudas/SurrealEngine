/* Copyright (C) Teemu Suutari */

#include "CRMDecompressor.hpp"
#include "HuffmanDecoder.hpp"
#include "DLTADecode.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "VariableLengthCodeDecoder.hpp"
#include "common/OverflowCheck.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool CRMDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	switch (hdr)
	{
		case FourCC("CrM!"):
		[[fallthrough]];
		case FourCC("CrM2"):
		[[fallthrough]];
		case FourCC("Crm!"):
		[[fallthrough]];
		case FourCC("Crm2"):
		[[fallthrough]];
		case 0x1805'1973U:		// Fears
		[[fallthrough]];
		case FourCC("CD\xb3\xb9"):	// BiFi 2
		[[fallthrough]];
		case FourCC("DCS!"):		// Sonic Attack/DualCrew-Shining
		[[fallthrough]];
		case FourCC("Iron"):		// Sun / TRSI
		[[fallthrough]];
		case FourCC("MSS!"):		// Infection / Mystic
		[[fallthrough]];
		case FourCC("mss!"):
		return true;

		default:
		return false;
	}
}

bool CRMDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("CRM2") || hdr==FourCC("CRMS");
}

std::shared_ptr<Decompressor> CRMDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<CRMDecompressor>(packedData,0,verify);
}

std::shared_ptr<XPKDecompressor> CRMDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<CRMDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

CRMDecompressor::CRMDecompressor(const Buffer &packedData,uint32_t recursionLevel,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	uint32_t hdr{packedData.readBE32(0)};
	if (!detectHeader(hdr,0) || packedData.size()<20)
		throw Decompressor::InvalidFormatError();
	if (hdr==0x1805'1973U || hdr==FourCC("CD\xb3\xb9") ||
		hdr==FourCC("Iron") || hdr==FourCC("MSS!")) hdr=FourCC("CrM2");
	if (hdr==FourCC("mss!")) hdr=FourCC("Crm2");
	if (hdr==FourCC("DCS!")) hdr=FourCC("CrM!");

	_rawSize=packedData.readBE32(6);
	_packedSize=packedData.readBE32(10);
	if (!_rawSize || !_packedSize ||
		_rawSize>getMaxRawSize() || _packedSize>getMaxPackedSize() ||
		OverflowCheck::sum(_packedSize,14U)>packedData.size())
			throw Decompressor::InvalidFormatError();
	if (((hdr>>8)&0xff)=='m') _isSampled=true;
	if ((hdr&0xff)=='2') _isLZH=true;
}

CRMDecompressor::CRMDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	CRMDecompressor{packedData,recursionLevel,verify}
{
	_isXPKDelta=(hdr==FourCC("CRMS"));
}

const std::string &CRMDecompressor::getName() const noexcept
{
	static std::string names[4]={
		"CrM!: Crunch-Mania standard-mode",
		"Crm!: Crunch-Mania standard-mode, sampled",
		"CrM2: Crunch-Mania LZH-mode",
		"Crm2: Crunch-Mania LZH-mode, sampled"};
	return names[(_isLZH?2:0)+(_isSampled?1:0)];
}

const std::string &CRMDecompressor::getSubName() const noexcept
{
	// the XPK-id is not used in decompressing process,
	// but there is a real id inside the stream
	// This means we can have frankenstein configurations,
	// although in practice we don't
	static std::string names[2]={
		"XPK-CRM2: Crunch-Mania LZH-mode",
		"XPK-CRMS: Crunch-Mania LZH-mode, sampled"};
	return names[(_isXPKDelta?1:0)];
}

size_t CRMDecompressor::getPackedSize() const noexcept
{
	return _packedSize+14;
}

size_t CRMDecompressor::getRawSize() const noexcept
{
	return _rawSize;
}

void CRMDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	if (rawData.size()<_rawSize)
		throw Decompressor::DecompressionError();

	BackwardInputStream inputStream{_packedData,14,_packedSize+14-6};
	LSBBitReader<BackwardInputStream> bitReader{inputStream};
	{
		// There are empty bits?!? at the start of the stream. take them out
		size_t bufOffset{_packedSize+14-6};
		uint32_t originalBitsContent{_packedData.readBE32(bufOffset)};
		uint16_t originalShift{_packedData.readBE16(bufOffset+4)};
		uint8_t bufBitsLength{uint8_t(originalShift+16)};
		uint32_t bufBitsContent{originalBitsContent>>(16-originalShift)};
		bitReader.reset(bufBitsContent,bufBitsLength);
	}
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1);
	};

	BackwardOutputStream outputStream{rawData,0,_rawSize};

	if (_isLZH)
	{
		typedef HuffmanDecoder<uint32_t> CRMHuffmanDecoder;

		auto readHuffmanTable=[&](CRMHuffmanDecoder &dec,uint32_t codeLength)
		{
			uint32_t maxDepth{readBits(4)};
			if (!maxDepth)
				throw Decompressor::DecompressionError();
			std::array<uint32_t,15> lengthTable;
			for (uint32_t i=0;i<maxDepth;i++)
				lengthTable[i]=readBits(std::min(i+1,codeLength));
			uint32_t code{0};
			for (uint32_t depth=1;depth<=maxDepth;depth++)
			{
				for (uint32_t i=0;i<lengthTable[depth-1];i++)
				{
						uint32_t value{readBits(codeLength)};
						dec.insert(HuffmanCode{depth,code>>(maxDepth-depth),value});
						code+=1<<(maxDepth-depth);
				}
			}
		};


		do {
			CRMHuffmanDecoder lengthDecoder,distanceDecoder;
			readHuffmanTable(lengthDecoder,9);
			readHuffmanTable(distanceDecoder,4);

			uint32_t items{readBits(16)+1};
			for (uint32_t i=0;i<items;i++)
			{
				if (uint32_t count{lengthDecoder.decode(readBit)};count&0x100)
				{
					outputStream.writeByte(count);
				} else {
					count+=3;

					uint32_t distanceBits=distanceDecoder.decode(readBit);
					uint32_t distance;
					if (!distanceBits)
					{
						distance=readBits(1)+1;
					} else {
						distance=(readBits(distanceBits)|(1<<distanceBits))+1;
					}
					outputStream.copy(distance,count);
				}
			}
		} while (readBit());
	} else {
		HuffmanDecoder<uint8_t> lengthDecoder
		{
			HuffmanCode{1,0b000,uint8_t{0}},
			HuffmanCode{2,0b010,uint8_t{1}},
			HuffmanCode{3,0b110,uint8_t{2}},
			HuffmanCode{3,0b111,uint8_t{3}}
		};

		HuffmanDecoder<uint8_t> distanceDecoder
		{
			HuffmanCode{1,0b00,uint8_t{1}},
			HuffmanCode{2,0b10,uint8_t{0}},
			HuffmanCode{2,0b11,uint8_t{2}}
		};

		VariableLengthCodeDecoder lengthVLC{1,2,4,8};
		VariableLengthCodeDecoder distanceVLC{5,9,14};

		while (!outputStream.eof())
		{
			if (readBit())
			{
				outputStream.writeByte(readBits(8));
			} else {
				uint8_t lengthIndex=lengthDecoder.decode(readBit);
				if (uint32_t count{lengthVLC.decode(readBits,lengthIndex)+2U};count==23)
				{
					if (readBit())
					{
						count=readBits(5)+15;
					} else {
						count=readBits(14)+15;
					}
					for (uint32_t i=0;i<count;i++)
						outputStream.writeByte(readBits(8));
				} else {
					if (count>23) count--;

					uint8_t distanceIndex{distanceDecoder.decode(readBit)};
					uint32_t distance{distanceVLC.decode(readBits,distanceIndex)};
					outputStream.copy(distance,count);
				}
			}
		}
	}

	if (!outputStream.eof())
		throw Decompressor::DecompressionError();
	if (_isSampled)
		DLTADecode::decode(rawData,rawData,0,_rawSize);
}

void CRMDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	if (rawData.size()!=_rawSize)
		throw Decompressor::DecompressionError();
	return decompressImpl(rawData,verify);
}

}
