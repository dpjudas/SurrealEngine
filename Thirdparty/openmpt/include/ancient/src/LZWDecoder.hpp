/* Copyright (C) Teemu Suutari */

#ifndef LZWDECODER_HPP
#define LZWDECODER_HPP

#include <memory>

#include "Decompressor.hpp"
#include "OutputStream.hpp"

namespace ancient::internal
{

class LZWDecoder
{
public:
	LZWDecoder(uint32_t maxCode,uint32_t literalCodes,uint32_t stackLength,uint32_t firstCode);
	~LZWDecoder() noexcept=default;

	void reset(uint32_t firstCode);
	void add(uint32_t code);

	template<typename F>
	void write(uint32_t code,bool addNew,F func)
	{
		auto suffixLookup=[&](uint32_t value)->uint32_t
		{
			if (value>=_freeIndex)
				throw Decompressor::DecompressionError();
			return (value<_literalCodes)?value:_suffix[value-_literalCodes];
		};

		uint32_t stackPos{0};

		uint32_t tmp{_newCode};
		if (addNew) code=_prevCode;

		_newCode=suffixLookup(code);
		while (code>=_literalCodes)
		{
			if (stackPos+1>=_stackLength)
				throw Decompressor::DecompressionError();
			_stack[stackPos++]=_newCode;
			code=_prefix[code-_literalCodes];
			_newCode=suffixLookup(code);
		}
		_stack[stackPos++]=_newCode;
		while (stackPos) func(_stack[--stackPos]);
		if (addNew) func(tmp);
	}

	bool isLiteral(uint32_t code) { return code<_freeIndex; }

	bool isFull() { return _freeIndex==_maxCode; }

	uint32_t getCurrentIndex() { return _freeIndex; }

private:
	uint32_t	_maxCode;
	uint32_t	_literalCodes;
	uint32_t	_stackLength;
	uint32_t	_freeIndex;

	uint32_t	_prevCode;
	uint32_t	_newCode{0};

	std::unique_ptr<uint32_t[]> _prefix;
	std::unique_ptr<uint8_t[]> _suffix;
	std::unique_ptr<uint8_t[]> _stack;
};

}

#endif
