/* Copyright (C) Teemu Suutari */

#include "RangeDecoder.hpp"


namespace ancient::internal
{

RangeDecoder::RangeDecoder(BitReader &bitReader,uint16_t initialValue) :
		_bitReader{bitReader},
		_stream{initialValue}
{
	// nothing needed
}

uint16_t RangeDecoder::decode(uint16_t length)
{
	return ((uint32_t(_stream-_low)+1)*length-1)/(uint32_t(_high-_low)+1);
}

void RangeDecoder::scale(uint16_t newLow,uint16_t newHigh,uint16_t newRange)
{
	uint32_t range{uint32_t(_high-_low)+1U};
	_high=(range*newHigh)/newRange+_low-1U;
	_low=(range*newLow)/newRange+_low;

	auto doubleContext=[&](uint16_t decr)
	{
		_low-=decr;
		_high-=decr;
		_stream-=decr;
		_low<<=1;
		_high=(_high<<1)|1U;
		_stream=(_stream<<1)|_bitReader.readBit();
	};

	for (;;)
	{
		if (_high<0x8000U)
		{
			doubleContext(0U);
		} else if (_low>=0x8000U) {
			doubleContext(0x8000U);
		} else if (_low>=0x4000U && _high<0xc000U) {
			doubleContext(0x4000U);
		} else break;
	}
}

}
