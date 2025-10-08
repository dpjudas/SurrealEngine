/* Copyright (C) Teemu Suutari */

#include "Common.hpp"


namespace ancient::internal
{

uint32_t rotateBits(uint32_t value,uint32_t count) noexcept
{
	static const uint8_t rotateNibble[16]={
		0x0,0x8,0x4,0xc,
		0x2,0xa,0x6,0xe,
		0x1,0x9,0x5,0xd,
		0x3,0xb,0x7,0xf
	};

	uint32_t ret=0;
	for (uint32_t i=0;i<count;i+=4)
	{
		ret=(ret<<4)|rotateNibble[value&0xf];
		value>>=4;
	}
	ret>>=(4-count)&3;
	return ret;
}

}
