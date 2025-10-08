/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/bit.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/endian/floatingpoint.hpp"
#include "mpt/endian/integer.hpp"
#include "mpt/endian/type_traits.hpp"
#include "openmpt/base/Types.hpp"



OPENMPT_NAMESPACE_BEGIN



using int64le = mpt::packed<int64, mpt::endian::little>;
using int32le = mpt::packed<int32, mpt::endian::little>;
using int16le = mpt::packed<int16, mpt::endian::little>;
using int8le = mpt::packed<int8, mpt::endian::little>;
using uint64le = mpt::packed<uint64, mpt::endian::little>;
using uint32le = mpt::packed<uint32, mpt::endian::little>;
using uint16le = mpt::packed<uint16, mpt::endian::little>;
using uint8le = mpt::packed<uint8, mpt::endian::little>;

using int64be = mpt::packed<int64, mpt::endian::big>;
using int32be = mpt::packed<int32, mpt::endian::big>;
using int16be = mpt::packed<int16, mpt::endian::big>;
using int8be = mpt::packed<int8, mpt::endian::big>;
using uint64be = mpt::packed<uint64, mpt::endian::big>;
using uint32be = mpt::packed<uint32, mpt::endian::big>;
using uint16be = mpt::packed<uint16, mpt::endian::big>;
using uint8be = mpt::packed<uint8, mpt::endian::big>;



using IEEE754binary32LE = mpt::IEEE754binary_types<mpt::float_traits<somefloat32>::is_float32 && mpt::float_traits<somefloat32>::is_ieee754_binary && mpt::float_traits<somefloat32>::is_native_endian, mpt::endian::native>::IEEE754binary32LE;
using IEEE754binary32BE = mpt::IEEE754binary_types<mpt::float_traits<somefloat32>::is_float32 && mpt::float_traits<somefloat32>::is_ieee754_binary && mpt::float_traits<somefloat32>::is_native_endian, mpt::endian::native>::IEEE754binary32BE;
using IEEE754binary64LE = mpt::IEEE754binary_types<mpt::float_traits<somefloat64>::is_float64 && mpt::float_traits<somefloat64>::is_ieee754_binary && mpt::float_traits<somefloat64>::is_native_endian, mpt::endian::native>::IEEE754binary64LE;
using IEEE754binary64BE = mpt::IEEE754binary_types<mpt::float_traits<somefloat64>::is_float64 && mpt::float_traits<somefloat64>::is_ieee754_binary && mpt::float_traits<somefloat64>::is_native_endian, mpt::endian::native>::IEEE754binary64BE;


// unaligned

using float32le = mpt::IEEE754binary32EmulatedLE;
using float32be = mpt::IEEE754binary32EmulatedBE;
using float64le = mpt::IEEE754binary64EmulatedLE;
using float64be = mpt::IEEE754binary64EmulatedBE;


// potentially aligned

using float32le_fast = mpt::IEEE754binary32LE;
using float32be_fast = mpt::IEEE754binary32BE;
using float64le_fast = mpt::IEEE754binary64LE;
using float64be_fast = mpt::IEEE754binary64BE;



#define MPT_BINARY_STRUCT(type, size) \
	constexpr bool declare_binary_safe(const type &) \
	{ \
		return true; \
	} \
	static_assert(mpt::check_binary_size<type>(size)); \
	/**/



OPENMPT_NAMESPACE_END
