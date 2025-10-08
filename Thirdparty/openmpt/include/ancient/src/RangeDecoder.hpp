/* Copyright (C) Teemu Suutari */

#ifndef RANGEDECODER_HPP
#define RANGEDECODER_HPP

#include <cstdint>

namespace ancient::internal
{

// used by too many compressors...
class RangeDecoder
{
public:
	class BitReader
	{
	public:
		BitReader() noexcept=default;
		virtual ~BitReader() noexcept=default;
		
		virtual uint32_t readBit()=0;
	};

	RangeDecoder(BitReader &bitReader,uint16_t initialValue);
	~RangeDecoder() noexcept=default;

	uint16_t decode(uint16_t length);
	void scale(uint16_t newLow,uint16_t newHigh,uint16_t newRange);

private:
	BitReader			&_bitReader;

	uint16_t			_low{0};
	uint16_t			_high{0xffffU};
	uint16_t			_stream;
};

}

#endif
