/* Copyright (C) Teemu Suutari */

#include "LZWDecoder.hpp"
#include "common/Common.hpp"


namespace ancient::internal
{

LZWDecoder::LZWDecoder(uint32_t maxCode,uint32_t literalCodes,uint32_t stackLength,uint32_t firstCode) :
	_maxCode{maxCode},
	_literalCodes{literalCodes},
	_stackLength{stackLength},
	_freeIndex{literalCodes},
	_prevCode{firstCode},
	_prefix{std::make_unique<uint32_t[]>(maxCode-literalCodes)},
	_suffix{std::make_unique<uint8_t[]>(maxCode-literalCodes)},
	_stack{std::make_unique<uint8_t[]>(stackLength)}
{
	// nothing needed
}

void LZWDecoder::reset(uint32_t firstCode)
{
	_freeIndex=_literalCodes;
	_prevCode=firstCode;
}

void LZWDecoder::add(uint32_t code)
{
	if (_freeIndex<_maxCode)
	{
		_suffix[_freeIndex-_literalCodes]=_newCode;
		_prefix[_freeIndex-_literalCodes]=_prevCode;
		_freeIndex++;
	}
	_prevCode=code;
}

}
