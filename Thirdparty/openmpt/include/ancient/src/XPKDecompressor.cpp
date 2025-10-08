/* Copyright (C) Teemu Suutari */

#include "XPKDecompressor.hpp"
#include "XPKMain.hpp"

namespace ancient::internal
{

XPKDecompressor::XPKDecompressor(uint32_t recursionLevel) :
	_recursionLevel{recursionLevel}
{
	// nothing needed
}

}
