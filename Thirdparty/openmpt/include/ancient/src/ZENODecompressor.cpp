/* Copyright (C) Teemu Suutari */

#include "ZENODecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/Common.hpp"

#include <vector>

namespace ancient::internal
{

bool ZENODecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("ZENO");
}

std::shared_ptr<XPKDecompressor> ZENODecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<ZENODecompressor>(hdr,recursionLevel,packedData,state,verify);
}

ZENODecompressor::ZENODecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr) || _packedData.size()<6)
		throw Decompressor::InvalidFormatError();
	// first 4 bytes is checksum for password. It needs to be zero
	if (_packedData.readBE32(0))
		throw Decompressor::InvalidFormatError();
	_maxBits=_packedData.read8(4U);
	if (_maxBits<9U || _maxBits>20U)
		throw Decompressor::InvalidFormatError();
	_startOffset=uint32_t(_packedData.read8(5U))+6U;
	if (_startOffset>=_packedData.size())
		throw Decompressor::InvalidFormatError();
}

const std::string &ZENODecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-ZENO: LZW-compressor"};
	return name;
}

void ZENODecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardInputStream inputStream{_packedData,_startOffset,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBits=[&](uint32_t count)->uint32_t
	{
		return bitReader.readBits8(count);
	};

	ForwardOutputStream outputStream{rawData,0,rawData.size()};

	uint32_t maxCode{1U<<_maxBits};
	uint32_t stackLength{5000U};				// magic constant
	std::vector<uint32_t> prefix(maxCode-258U);
	std::vector<uint8_t> suffix(maxCode-258U);
	std::vector<uint8_t> stack(stackLength);

	uint32_t freeIndex,codeBits;

	auto init=[&]()
	{
		codeBits=9U;
		freeIndex=258U;
	};

	init();
	uint32_t prevCode{readBits(9U)};
	uint32_t newCode{prevCode};
	suffix[freeIndex-258]=0;
	prefix[freeIndex-258]=0;
	freeIndex++;
	outputStream.writeByte(newCode);
	
	while (!outputStream.eof())
	{
		if (freeIndex+3U>=(1U<<codeBits) && codeBits<_maxBits) codeBits++;
		switch (uint32_t code{readBits(codeBits)};code)
		{
			case 256U:
			throw Decompressor::DecompressionError();
			break;

			case 257U:
			init();
			break;

			default:
			{
				uint32_t stackPos{0};
				uint32_t tmp{code};
				if (tmp==freeIndex)
				{
					stack[stackPos++]=newCode;
					tmp=prevCode;
				}
				if (tmp>=258U)
				{
					do {
						if (stackPos+1>=stackLength || tmp>=freeIndex)
							throw Decompressor::DecompressionError();
						stack[stackPos++]=suffix[tmp-258U];
						tmp=prefix[tmp-258U];
					} while (tmp>=258U);
					stack[stackPos++]=newCode=tmp;
					while (stackPos) outputStream.writeByte(stack[--stackPos]);
				} else {
					newCode=tmp;
					outputStream.writeByte(tmp);
					if (stackPos) outputStream.writeByte(stack[0]);
				}
			}
			if (freeIndex<maxCode)
			{
				suffix[freeIndex-258U]=newCode;
				prefix[freeIndex-258U]=prevCode;
				freeIndex++;
			}
			prevCode=code;
			break;
		}
	}
}

}
