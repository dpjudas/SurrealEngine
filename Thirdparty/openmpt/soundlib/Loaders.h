/*
 * Loaders.h
 * ---------
 * Purpose: Common functions for module loaders
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "../common/misc_util.h"
#include "../common/FileReader.h"
#include "Sndfile.h"
#include "SampleIO.h"
#include "openmpt/fileformat_base/magic.hpp"

OPENMPT_NAMESPACE_BEGIN


// Read 'howMany' order items from an array.
// 'stopIndex' is treated as '---', 'ignoreIndex' is treated as '+++'. If the format doesn't support such indices, just pass uint16_max.
template<typename T, size_t arraySize>
bool ReadOrderFromArray(ModSequence &order, const T(&orders)[arraySize], size_t howMany = arraySize, uint16 stopIndex = uint16_max, uint16 ignoreIndex = uint16_max)
{
	static_assert(mpt::is_binary_safe<T>::value);
	LimitMax(howMany, arraySize);
	LimitMax(howMany, MAX_ORDERS);
	ORDERINDEX readEntries = static_cast<ORDERINDEX>(howMany);

	order.resize(readEntries);
	for(int i = 0; i < readEntries; i++)
	{
		PATTERNINDEX pat = static_cast<PATTERNINDEX>(orders[i]);
		if(pat == stopIndex) pat = PATTERNINDEX_INVALID;
		else if(pat == ignoreIndex) pat = PATTERNINDEX_SKIP;
		order.at(i) = pat;
	}
	return true;
}


// Read 'howMany' order items as integers with defined endianness from a file.
// 'stopIndex' is treated as '---', 'ignoreIndex' is treated as '+++'. If the format doesn't support such indices, just pass uint16_max.
template<typename T>
bool ReadOrderFromFile(ModSequence &order, FileReader &file, size_t howMany, uint16 stopIndex = uint16_max, uint16 ignoreIndex = uint16_max)
{
	static_assert(mpt::is_binary_safe<T>::value);
	if(!file.CanRead(howMany * sizeof(T)))
		return false;
	LimitMax(howMany, MAX_ORDERS);
	ORDERINDEX readEntries = static_cast<ORDERINDEX>(howMany);

	order.resize(readEntries);
	T patF;
	for(auto &pat : order)
	{
		file.ReadStruct(patF);
		pat = static_cast<PATTERNINDEX>(patF);
		if(pat == stopIndex) pat = PATTERNINDEX_INVALID;
		else if(pat == ignoreIndex) pat = PATTERNINDEX_SKIP;
	}
	return true;
}

OPENMPT_NAMESPACE_END
