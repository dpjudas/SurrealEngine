/* Copyright (C) Teemu Suutari */

#include "LZCBDecompressor.hpp"
#include "RangeDecoder.hpp"
#include "InputStream.hpp"
#include "OutputStream.hpp"
#include "FrequencyTree.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

template<size_t T>
class FrequencyDecoder
{
public:
	FrequencyDecoder(RangeDecoder &decoder) :
		_decoder{decoder}
	{
		// nothing needed
	}
	~FrequencyDecoder() noexcept=default;

	template<typename F>
	uint16_t decode(F readFunc)
	{
		uint16_t freq{0};
		uint16_t symbol;
		uint16_t value{_decoder.decode(_threshold+_tree.getTotal())};
		if (value>=_threshold)
		{
			uint16_t low;
			symbol=_tree.decode(value-_threshold,low,freq);
			_decoder.scale(_threshold+low,_threshold+low+freq,_threshold+_tree.getTotal());
			if (freq==1 && _threshold>1)
				_threshold--;
		} else {
			_decoder.scale(0,_threshold,_threshold+_tree.getTotal());
			symbol=readFunc();
			// A bug in the encoder
			if (!symbol && _tree[symbol]) symbol=T;
			_threshold++;
		}
		_tree.add(symbol,1);
		if (_threshold+_tree.getTotal()>=0x3ffdU)
		{
			for (uint32_t i=0;i<T+1;i++)
				_tree.set(i,_tree[i]>>1);

			_threshold=(_threshold>>1)+1;
		}
		return symbol;
	}

private:
	RangeDecoder					&_decoder;
	FrequencyTree<uint16_t,uint16_t,T+1>		_tree;
	uint16_t					_threshold{1};
};

bool LZCBDecompressor::detectHeaderXPK(uint32_t hdr) noexcept
{
	return hdr==FourCC("LZCB");
}

std::shared_ptr<XPKDecompressor> LZCBDecompressor::create(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify)
{
	return std::make_shared<LZCBDecompressor>(hdr,recursionLevel,packedData,state,verify);
}

LZCBDecompressor::LZCBDecompressor(uint32_t hdr,uint32_t recursionLevel,const Buffer &packedData,std::shared_ptr<XPKDecompressor::State> &state,bool verify) :
	XPKDecompressor{recursionLevel},
	_packedData{packedData}
{
	if (packedData.size()<2)
		throw Decompressor::InvalidFormatError();
}

const std::string &LZCBDecompressor::getSubName() const noexcept
{
	static std::string name{"XPK-LZCB: LZ-compressor"};
	return name;
}

void LZCBDecompressor::decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)
{
	class BitReader : public RangeDecoder::BitReader
	{
	public:
		BitReader(ForwardInputStream &stream) :
			_reader(stream)
		{
			// nothing needed
		}
		~BitReader() noexcept=default;

		uint32_t readBit() final
		{
			return _reader.readBitsBE32(1);
		}

		uint32_t readBits(uint32_t bitCount)
		{
			return _reader.readBitsBE32(bitCount);
		}

	private:
		MSBBitReader<ForwardInputStream>	_reader;
	};

	ForwardInputStream inputStream{_packedData,0,_packedData.size(),7U};
	ForwardOutputStream outputStream{rawData,0,rawData.size()};
	BitReader bitReader{inputStream};

	RangeDecoder rangeDecoder{bitReader,uint16_t(bitReader.readBits(16))};

	// Ugly duplicates
	auto readByte=[&]()->uint16_t
	{
		uint16_t ret=rangeDecoder.decode(0x100U);
		rangeDecoder.scale(ret,ret+1,0x100U);
		return ret;
	};

	auto readCount=[&]()->uint16_t
	{
		uint16_t ret=rangeDecoder.decode(0x101U);
		rangeDecoder.scale(ret,ret+1,0x101U);
		return ret;
	};

	FrequencyDecoder<256> baseLiteralDecoder{rangeDecoder};
	FrequencyDecoder<257> repeatCountDecoder{rangeDecoder};
	FrequencyDecoder<257> literalCountDecoder{rangeDecoder};
	FrequencyDecoder<256> distanceDecoder{rangeDecoder};

	std::array<std::unique_ptr<FrequencyDecoder<256>>,256> literalDecoders;

	uint8_t ch{uint8_t(baseLiteralDecoder.decode(readByte))};
	outputStream.writeByte(ch);
	bool lastIsLiteral{true};
	while (!outputStream.eof())
	{
		uint32_t count{repeatCountDecoder.decode(readCount)};
		if (count)
		{
			if (count==0x100U)
			{
				uint32_t tmp;
				do
				{
					tmp=readByte();
					count+=tmp;
				} while (tmp==0xffU);
			}
			count+=lastIsLiteral?5:4;

			uint32_t distance{uint32_t(distanceDecoder.decode(readByte)<<8U)};
			distance|=readByte();

			ch=outputStream.copy(distance,count);
			lastIsLiteral=false;
		} else {
			uint16_t literalCount;
			do
			{
				literalCount=literalCountDecoder.decode(readCount);
				if (!literalCount)
					throw Decompressor::DecompressionError();

				for (uint32_t i=0;i<literalCount;i++)
				{
					auto &literalDecoder{literalDecoders[ch]};
					if (!literalDecoder) literalDecoder=std::make_unique<FrequencyDecoder<256>>(rangeDecoder);
					ch=uint8_t(literalDecoder->decode([&]()
					{
						return baseLiteralDecoder.decode(readByte);
					}));
					outputStream.writeByte(ch);
				}
			} while (literalCount==0x100U);
			lastIsLiteral=true;
		}
	}
}

}
