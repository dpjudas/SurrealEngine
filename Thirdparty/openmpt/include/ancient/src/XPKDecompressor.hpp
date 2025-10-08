/* Copyright (C) Teemu Suutari */

#ifndef XPKDECOMPRESSOR_HPP
#define XPKDECOMPRESSOR_HPP

#include <cstddef>
#include <cstdint>

#include <string>

#include "Decompressor.hpp"

namespace ancient::internal
{

class XPKDecompressor
{
public:
	class State
	{
	public:
		State(const State&)=delete;
		State& operator=(const State&)=delete;

		State() noexcept=default;
		virtual ~State() noexcept=default;
	};

	XPKDecompressor(const XPKDecompressor&)=delete;
	XPKDecompressor& operator=(const XPKDecompressor&)=delete;

	XPKDecompressor(uint32_t recursionLevel=0);
	virtual ~XPKDecompressor() noexcept=default;

	virtual const std::string &getSubName() const noexcept=0;

	// Actual decompression
	virtual void decompressImpl(Buffer &rawData,const Buffer &previousData,bool verify)=0;

protected:
	uint32_t	_recursionLevel;
};

}

#endif
