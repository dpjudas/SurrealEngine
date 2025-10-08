/* Copyright (C) Teemu Suutari */

#include "LOBDecompressor.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "HuffmanDecoder.hpp"
#include "LZWDecoder.hpp"
#include "common/Common.hpp"
#include "common/SubBuffer.hpp"
#include "common/MemoryBuffer.hpp"
#include "common/OverflowCheck.hpp"


namespace ancient::internal
{

bool LOBDecompressor::detectHeader(uint32_t hdr,uint32_t footer) noexcept
{
	return hdr==FourCC("\001LOB")||hdr==FourCC("\002LOB")||hdr==FourCC("\003LOB");
}

std::shared_ptr<Decompressor> LOBDecompressor::create(const Buffer &packedData,bool exactSizeKnown,bool verify)
{
	return std::make_shared<LOBDecompressor>(packedData,verify);
}

LOBDecompressor::LOBDecompressor(const Buffer &packedData,bool verify) :
	_packedData{packedData}
{
	uint32_t hdr{packedData.readBE32(0)};
	if (!detectHeader(hdr,0) || packedData.size()<12U)
		throw InvalidFormatError();
	_methodCount=hdr>>24U;

	uint8_t method{packedData.read8(4U)};
	if (method<1U || method>6U)
		throw InvalidFormatError();
	if (_methodCount==1U)
	{
		_rawSize=packedData.readBE32(4U)&0xff'ffffU;
		if (!_rawSize || _rawSize>getMaxRawSize())
			throw InvalidFormatError();
	}
	_packedSize=OverflowCheck::sum(packedData.readBE32(8U),12U);
	// now parse the huffman tables to get the correct size
	if (method==2U)
	{
		uint32_t count{2U};
		for (uint32_t i=0;i<count;i+=2U)
		{
			if (_packedSize+count>packedData.size())
				throw InvalidFormatError();
			uint32_t t1{_packedData.read8(_packedSize+i)};
			uint32_t t2{_packedData.read8(_packedSize+i+1U)};
			if (t1!=t2)
			{
				t1=std::max(t1,t2);
				count=std::max(t1+i+4U,count);
				if (count>1024U)
					throw InvalidFormatError();
			}
		}
		_packedSize+=count;
	}
	if (_packedSize>packedData.size())
		throw InvalidFormatError();
}

const std::string &LOBDecompressor::getName() const noexcept
{
	static std::string names[3]={
		"LOB: LOB's File Compressor",
		"LOB: LOB's File Compressor (double compressed)",
		"LOB: LOB's File Compressor (triple compressed)"};
	return names[_methodCount-1U];
}

size_t LOBDecompressor::getPackedSize() const noexcept
{
	return _packedSize;
}

size_t LOBDecompressor::getRawSize() const noexcept
{
	// not known for recursive compressions
	return _rawSize;
}

void LOBDecompressor::decompressRound(Buffer &rawData,const Buffer &packedData)
{
	uint8_t method{packedData.read8(0)};
	if (method<1U || method>6U)
		throw DecompressionError();
	uint32_t rawSize{packedData.readBE32(0)&0xff'ffffU};
	uint32_t packedSize{OverflowCheck::sum(packedData.readBE32(4U),8U)};

	ForwardInputStream inputStream{packedData,8U,packedSize};
	MSBBitReader<ForwardInputStream> bitReader{inputStream};
	auto readBit=[&]()->uint32_t
	{
		return bitReader.readBits8(1U);
	};
	auto readBits=[&](uint32_t bits)->uint32_t
	{
		return bitReader.readBits8(bits);
	};
	auto readByte=[&]()->uint8_t
	{
		return inputStream.readByte();
	};

	ForwardOutputStream outputStream{rawData,0,rawSize};
	auto writeByte=[&](uint8_t value)
	{
		outputStream.writeByte(value);
	};

	switch (method)
	{
		// BMC (RLE compressor)
		case 1U:
		while (!outputStream.eof())
		{
			uint32_t count{readByte()};
			if (count<0x80U)
			{
				count++;
				for (uint32_t i=0;i<count;i++)
					outputStream.writeByte(readByte());
			} else {
				outputStream.writeByte(readByte());
				outputStream.copy(1U,0x100U-count);
			}
		}
		break;

		// HUF (huffman coding)
		case 2U:
		{
			HuffmanDecoder<uint8_t> decoder;
			std::array<uint16_t,1024> tree;

			uint32_t count=2U;
			for (uint32_t i=0;i<count;i+=2U)
			{
				uint32_t t1{packedData.read8(packedSize+i)};
				uint32_t t2{packedData.read8(packedSize+i+1U)};
				if (t1!=t2)
				{
					t1=t1*2U+2U;
					t2=t2*2U+2U;
					tree[i]=t2;
					tree[i+1]=t1;
					count=std::max(t1+i+2U,count);
					count=std::max(t2+i+2U,count);
					if (count>1024U)
						throw DecompressionError();
				} else {
					tree[i]=0;
					tree[i+1]=t1;
				}
			}

			auto branch=[&](uint32_t node,uint32_t length,uint32_t bits,auto branch)->void
			{
				if (node>=count)
					throw DecompressionError();
				if (tree[node])
				{
					length++;
					bits<<=1U;
					if (length>=32U)
						throw DecompressionError();
					branch(node+tree[node],length,bits,branch);
					if (node+1>=count)
						throw DecompressionError();
					branch(node+tree[node+1],length,bits|1U,branch);
				} else {
					if (!length)
						throw DecompressionError();
					decoder.insert(HuffmanCode{length,bits,uint8_t(tree[node+1])});
				}
			};
			branch(0,0,0,branch);

			while (!outputStream.eof())
				outputStream.writeByte(decoder.decode(readBit));
		}
		break;

		// LZW (12-bit fixed code LZW)
		case 3U:
		{
			uint32_t firstCode{readBits(12U)};
			LZWDecoder decoder{4096U,256U,65536U,firstCode};
			decoder.write(firstCode,false,writeByte);
			while (!outputStream.eof())
			{
				uint32_t code{readBits(12U)};
				if (code==0xfffU)
				{
					firstCode=readBits(12U);
					decoder.reset(firstCode);
					decoder.write(firstCode,false,writeByte);
				} else {
					decoder.write(code,!decoder.isLiteral(code),writeByte);
					decoder.add(code);
				}
			}
		}
		break;

		// LZB (9 to 12-bit LZW)
		case 4U:
		{
			uint32_t codeBits{9U};
			uint32_t firstCode{readBits(codeBits)};
			if (!firstCode--)
				throw DecompressionError();
			LZWDecoder decoder{4096U,256U,65536U,firstCode};
			decoder.write(firstCode,false,writeByte);
			while (!outputStream.eof())
			{
				if (codeBits!=12U && decoder.getCurrentIndex()+1U>=(1U<<codeBits))
					codeBits++;
				uint32_t code{readBits(codeBits)};
				if (!code--)
				{
					codeBits=9U;
					firstCode=readBits(codeBits);
					if (!firstCode--)
						throw DecompressionError();
					decoder.reset(firstCode);
					decoder.write(firstCode,false,writeByte);
				} else {
					decoder.write(code,!decoder.isLiteral(code),writeByte);
					decoder.add(code);
				}
			}
		}
		break;

		// MSP (something lz)
		case 5U:
		{
			HuffmanDecoder<uint8_t> decoder
			{
				HuffmanCode{2,0b000,uint8_t{0}},
				HuffmanCode{2,0b001,uint8_t{1}},
				HuffmanCode{3,0b100,uint8_t{2}},
				HuffmanCode{3,0b101,uint8_t{3}},
				HuffmanCode{3,0b110,uint8_t{4}},
				HuffmanCode{3,0b111,uint8_t{5}}
			};
			// will fail if size<2
			outputStream.writeByte(readByte());
			outputStream.writeByte(readByte());
			while (!outputStream.eof())
			{
				uint32_t count,distance;
				switch (uint32_t value=decoder.decode(readBit))
				{
					case 0:
					count=readBits(3U)+1U;
					for (uint32_t i=0;i<count;i++)
						outputStream.writeByte(readByte());
					break;

					case 1U:
					[[fallthrough]];
					case 2U:
					[[fallthrough]];
					case 3U:
					distance=readBits(value+7U)+1U;
					outputStream.copy(distance,value+1U);
					break;

					case 4U:
					count=readBits(8U)+4U;
					distance=readBits(12U)+1U;
					outputStream.copy(distance,count);
					break;

					case 5U:
					count=readBits(8U)+9U;
					for (uint32_t i=0;i<count;i++)
						outputStream.writeByte(readByte());
					break;

					default:
					throw DecompressionError();
					break;
				}
			}
		}
		break;

		// MSS (lzss style packer)
		case 6U:
		while (!outputStream.eof())
		{
			if (readBit())
			{
				outputStream.writeByte(readByte());
			} else {
				uint8_t byte1=readByte();
				uint8_t byte2=readByte();
				uint32_t distance=(uint32_t(byte1&0xf0)<<4)|byte2;
				uint32_t count=uint32_t(byte1&0xf)+3;

				outputStream.copy(distance,count);
			}
		}
		break;

		default:
		throw DecompressionError();
		break;
	}
}

void LOBDecompressor::decompressImpl(Buffer &rawData,bool verify)
{
	switch (_methodCount)
	{
		case 1U:
		decompressRound(rawData,ConstSubBuffer(_packedData,4U,_packedData.size()-4U));
		break;

		case 2U:
		{
			uint32_t rawSize{_packedData.readBE32(4U)&0xff'ffffU};
			if (!rawSize || rawSize>getMaxRawSize())
				throw DecompressionError();
			MemoryBuffer tmpBuffer{rawSize};
			decompressRound(tmpBuffer,ConstSubBuffer(_packedData,4U,_packedData.size()-4U));
			rawSize=tmpBuffer.readBE32(0U)&0xff'ffffU;
			if (!rawSize || rawSize>getMaxRawSize())
				throw DecompressionError();
			_rawSize=rawSize;
			if (rawData.size()<_rawSize)
				rawData.resize(rawSize);
			decompressRound(rawData,tmpBuffer);
		}
		break;

		case 3U:
		{
			uint32_t rawSize{_packedData.readBE32(4U)&0xff'ffffU};
			if (!rawSize || rawSize>getMaxRawSize())
				throw DecompressionError();
			MemoryBuffer tmpBuffer{rawSize};
			decompressRound(tmpBuffer,ConstSubBuffer(_packedData,4U,_packedData.size()-4U));
			rawSize=tmpBuffer.readBE32(0U)&0xff'ffffU;
			if (!rawSize || rawSize>getMaxRawSize())
				throw DecompressionError();
			MemoryBuffer tmpBuffer2{rawSize};
			decompressRound(tmpBuffer2,tmpBuffer);
			rawSize=tmpBuffer2.readBE32(0U)&0xff'ffffU;
			if (!rawSize || rawSize>getMaxRawSize())
				throw DecompressionError();
			_rawSize=rawSize;
			if (rawData.size()<_rawSize)
				rawData.resize(rawSize);
			decompressRound(rawData,tmpBuffer2);
		}
		break;

		default:
		throw DecompressionError();
		break;
	}
}

}
