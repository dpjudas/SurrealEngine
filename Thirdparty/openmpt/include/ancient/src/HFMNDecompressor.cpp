/* Copyright (C) Teemu Suutari */

#include "HFMNDecompressor.hpp"
#include "HuffmanDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "common/OverflowCheck.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

bool HFMNDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("HFMN");
}

std::shared_ptr<XPKDecompressor> HFMNDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<HFMNDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

HFMNDecompressor::HFMNDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (!detectHeaderXPK(hdr) || packedData.size()<4)
		throw Decompressor::InvalidFormatError();
	uint16_t tmp{packedData.readBE16(0)};
	if (tmp&3U)
		throw Decompressor::InvalidFormatError();	// header is being written in 4 byte chunks
	_headerSize=tmp&0x1ffU;					// the top 7 bits are flags. No definition what they are and they are ignored in decoder...
	if (OverflowCheck::sum(_headerSize,4U)>packedData.size())
		throw Decompressor::InvalidFormatError();
	_rawSize=packedData.readBE16(_headerSize+2U);
	if (!_rawSize)
		throw Decompressor::InvalidFormatError();
	_headerSize+=4;
}

const std::string &HFMNDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-HFMN: Huffman compressor"};
	return name;
}

void HFMNDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	ForwardOutputStream outputStream{rawData,0,rawData.size()};
	HuffmanDecoder<uint32_t> decoder;

	if (rawData.size()!=_rawSize)
		throw Decompressor::DecompressionError();
	{
		ForwardInputStream inputStream{_packedData,2,_headerSize};
		MSBBitReader<ForwardInputStream> bitReader{inputStream};
		auto readBit=[&]()->uint32_t
		{
			return bitReader.readBits8(1);
		};

		uint32_t code{1};
		uint32_t codeBits{1};
		for (;;)
		{
			if (!readBit())
			{
				uint32_t lit=rotateBits(bitReader.readBits8(8),8);
				decoder.insert(HuffmanCode{codeBits,code,lit});
				while (!(code&1) && codeBits)
				{
					codeBits--;
					code>>=1;
				}
				if (!codeBits) break;
				code--;
			} else {
				code=(code<<1)+1;
				codeBits++;
			}
		}
	}

	ForwardInputStream inputStream{_packedData,_headerSize,_packedData.size()};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1);
	};

	while (!outputStream.eof())
		outputStream.writeByte(decoder.decode(readBit));
}

}
