/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/float.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/memory.hpp"
#include "openmpt/base/Endian.hpp"
#include "openmpt/base/Types.hpp"

#include <algorithm>

#include <cmath>
#include <cstddef>


OPENMPT_NAMESPACE_BEGIN


// Byte offsets, from lowest significant to highest significant byte (for various functor template parameters)
#define littleEndian64 0, 1, 2, 3, 4, 5, 6, 7
#define littleEndian32 0, 1, 2, 3
#define littleEndian24 0, 1, 2
#define littleEndian16 0, 1

#define bigEndian64 7, 6, 5, 4, 3, 2, 1, 0
#define bigEndian32 3, 2, 1, 0
#define bigEndian24 2, 1, 0
#define bigEndian16 1, 0


namespace SC
{  // SC = _S_ample_C_onversion


// Every sample decoding functor has to typedef its input_t and output_t
// and has to provide a static constexpr input_inc member
// which describes by how many input_t elements inBuf has to be incremented between invocations.
// input_inc is normally 1 except when decoding e.g. bigger sample values
// from multiple std::byte values.


struct DecodeInt8
{
	using input_t = std::byte;
	using output_t = int8;
	static constexpr std::size_t input_inc = 1;
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		return mpt::byte_cast<int8>(*inBuf);
	}
};

struct DecodeUint8
{
	using input_t = std::byte;
	using output_t = int8;
	static constexpr std::size_t input_inc = 1;
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		return static_cast<int8>(static_cast<int>(mpt::byte_cast<uint8>(*inBuf)) - 128);
	}
};

struct DecodeInt8Delta
{
	using input_t = std::byte;
	using output_t = int8;
	static constexpr std::size_t input_inc = 1;
	uint8 delta;
	DecodeInt8Delta()
		: delta(0)
	{
	}
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		delta += mpt::byte_cast<uint8>(*inBuf);
		return static_cast<int8>(delta);
	}
};

struct DecodeInt16uLaw
{
	using input_t = std::byte;
	using output_t = int16;
	static constexpr std::size_t input_inc = 1;
	// clang-format off
	static constexpr std::array<int16, 256> uLawTable =
	{
		-32124,-31100,-30076,-29052,-28028,-27004,-25980,-24956,
		-23932,-22908,-21884,-20860,-19836,-18812,-17788,-16764,
		-15996,-15484,-14972,-14460,-13948,-13436,-12924,-12412,
		-11900,-11388,-10876,-10364, -9852, -9340, -8828, -8316,
		 -7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,
		 -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
		 -3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,
		 -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
		 -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,
		 -1372, -1308, -1244, -1180, -1116, -1052,  -988,  -924,
		  -876,  -844,  -812,  -780,  -748,  -716,  -684,  -652,
		  -620,  -588,  -556,  -524,  -492,  -460,  -428,  -396,
		  -372,  -356,  -340,  -324,  -308,  -292,  -276,  -260,
		  -244,  -228,  -212,  -196,  -180,  -164,  -148,  -132,
		  -120,  -112,  -104,   -96,   -88,   -80,   -72,   -64,
		   -56,   -48,   -40,   -32,   -24,   -16,    -8,    -1,
		 32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,
		 23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
		 15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,
		 11900, 11388, 10876, 10364,  9852,  9340,  8828,  8316,
		  7932,  7676,  7420,  7164,  6908,  6652,  6396,  6140,
		  5884,  5628,  5372,  5116,  4860,  4604,  4348,  4092,
		  3900,  3772,  3644,  3516,  3388,  3260,  3132,  3004,
		  2876,  2748,  2620,  2492,  2364,  2236,  2108,  1980,
		  1884,  1820,  1756,  1692,  1628,  1564,  1500,  1436,
		  1372,  1308,  1244,  1180,  1116,  1052,   988,   924,
		   876,   844,   812,   780,   748,   716,   684,   652,
		   620,   588,   556,   524,   492,   460,   428,   396,
		   372,   356,   340,   324,   308,   292,   276,   260,
		   244,   228,   212,   196,   180,   164,   148,   132,
		   120,   112,   104,    96,    88,    80,    72,    64,
		    56,    48,    40,    32,    24,    16,     8,     0
	};
	// clang-format on
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		return uLawTable[mpt::byte_cast<uint8>(*inBuf)];
	}
};

struct DecodeInt16ALaw
{
	using input_t = std::byte;
	using output_t = int16;
	static constexpr std::size_t input_inc = 1;
	// clang-format off
	static constexpr std::array<int16, 256> ALawTable =
	{
		 -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736,
		 -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784,
		 -2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368,
		 -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392,
		-22016,-20992,-24064,-23040,-17920,-16896,-19968,-18944,
		-30208,-29184,-32256,-31232,-26112,-25088,-28160,-27136,
		-11008,-10496,-12032,-11520, -8960, -8448, -9984, -9472,
		-15104,-14592,-16128,-15616,-13056,-12544,-14080,-13568,
		  -344,  -328,  -376,  -360,  -280,  -264,  -312,  -296,
		  -472,  -456,  -504,  -488,  -408,  -392,  -440,  -424,
		   -88,   -72,  -120,  -104,   -24,   -8,    -56,   -40,
		  -216,  -200,  -248,  -232,  -152,  -136,  -184,  -168,
		 -1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184,
		 -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696,
		  -688,  -656,  -752,  -720,  -560,  -528,  -624,  -592,
		  -944,  -912,  -1008, -976,  -816,  -784,  -880,  -848,
		  5504,  5248,  6016,  5760,  4480,  4224,  4992,  4736,
		  7552,  7296,  8064,  7808,  6528,  6272,  7040,  6784,
		  2752,  2624,  3008,  2880,  2240,  2112,  2496,  2368,
		  3776,  3648,  4032,  3904,  3264,  3136,  3520,  3392,
		 22016, 20992, 24064, 23040, 17920, 16896, 19968, 18944,
		 30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136,
		 11008, 10496, 12032, 11520,  8960,  8448,  9984,  9472,
		 15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568,
		   344,   328,   376,   360,   280,   264,   312,   296,
		   472,   456,   504,   488,   408,   392,   440,   424,
		    88,    72,   120,   104,    24,     8,    56,    40,
		   216,   200,   248,   232,   152,   136,   184,   168,
		  1376,  1312,  1504,  1440,  1120,  1056,  1248,  1184,
		  1888,  1824,  2016,  1952,  1632,  1568,  1760,  1696,
		   688,   656,   752,   720,   560,   528,   624,   592,
		   944,   912,  1008,   976,   816,   784,   880,   848
	};
	// clang-format on
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		return ALawTable[mpt::byte_cast<uint8>(*inBuf)];
	}
};

template <uint16 offset, std::size_t loByteIndex, std::size_t hiByteIndex>
struct DecodeInt16
{
	using input_t = std::byte;
	using output_t = int16;
	static constexpr std::size_t input_inc = 2;
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		return static_cast<int16>((mpt::byte_cast<uint8>(inBuf[loByteIndex]) | (mpt::byte_cast<uint8>(inBuf[hiByteIndex]) << 8)) - offset);
	}
};

template <std::size_t loByteIndex, std::size_t hiByteIndex>
struct DecodeInt16Delta
{
	using input_t = std::byte;
	using output_t = int16;
	static constexpr std::size_t input_inc = 2;
	uint16 delta;
	DecodeInt16Delta()
		: delta(0)
	{
	}
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		delta += static_cast<uint16>(static_cast<uint16>(mpt::byte_cast<uint8>(inBuf[loByteIndex])) | static_cast<uint16>(mpt::byte_cast<uint8>(inBuf[hiByteIndex]) << 8));
		return static_cast<int16>(delta);
	}
};

struct DecodeInt16Delta8
{
	using input_t = std::byte;
	using output_t = int16;
	static constexpr std::size_t input_inc = 2;
	uint16 delta;
	DecodeInt16Delta8()
		: delta(0)
	{
	}
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		delta += mpt::byte_cast<uint8>(inBuf[0]);
		int16 result = delta & 0xFF;
		delta += mpt::byte_cast<uint8>(inBuf[1]);
		result |= static_cast<uint16>(delta << 8);
		return result;
	}
};

template <uint32 offset, std::size_t loByteIndex, std::size_t midByteIndex, std::size_t hiByteIndex>
struct DecodeInt24
{
	using input_t = std::byte;
	using output_t = int32;
	static constexpr std::size_t input_inc = 3;
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		const uint32 tmp = (uint32(0)
							| (static_cast<uint32>(mpt::byte_cast<uint8>(inBuf[loByteIndex])) << 8)
							| (static_cast<uint32>(mpt::byte_cast<uint8>(inBuf[midByteIndex])) << 16)
							| (static_cast<uint32>(mpt::byte_cast<uint8>(inBuf[hiByteIndex])) << 24))
						 - offset;
		return static_cast<int32>(tmp);
	}
};

template <uint32 offset, std::size_t loLoByteIndex, std::size_t loHiByteIndex, std::size_t hiLoByteIndex, std::size_t hiHiByteIndex>
struct DecodeInt32
{
	using input_t = std::byte;
	using output_t = int32;
	static constexpr std::size_t input_inc = 4;
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		const uint32 tmp = (uint32(0)
							| (static_cast<uint32>(mpt::byte_cast<uint8>(inBuf[loLoByteIndex])) << 0)
							| (static_cast<uint32>(mpt::byte_cast<uint8>(inBuf[loHiByteIndex])) << 8)
							| (static_cast<uint32>(mpt::byte_cast<uint8>(inBuf[hiLoByteIndex])) << 16)
							| (static_cast<uint32>(mpt::byte_cast<uint8>(inBuf[hiHiByteIndex])) << 24))
						 - offset;
		return static_cast<int32>(tmp);
	}
};

template <uint64 offset, std::size_t b0, std::size_t b1, std::size_t b2, std::size_t b3, std::size_t b4, std::size_t b5, std::size_t b6, std::size_t b7>
struct DecodeInt64
{
	using input_t = std::byte;
	using output_t = int64;
	static constexpr std::size_t input_inc = 8;
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		const uint64 tmp = (uint64(0)
							| (static_cast<uint64>(mpt::byte_cast<uint8>(inBuf[b0])) << 0)
							| (static_cast<uint64>(mpt::byte_cast<uint8>(inBuf[b1])) << 8)
							| (static_cast<uint64>(mpt::byte_cast<uint8>(inBuf[b2])) << 16)
							| (static_cast<uint64>(mpt::byte_cast<uint8>(inBuf[b3])) << 24)
							| (static_cast<uint64>(mpt::byte_cast<uint8>(inBuf[b4])) << 32)
							| (static_cast<uint64>(mpt::byte_cast<uint8>(inBuf[b5])) << 40)
							| (static_cast<uint64>(mpt::byte_cast<uint8>(inBuf[b6])) << 48)
							| (static_cast<uint64>(mpt::byte_cast<uint8>(inBuf[b7])) << 56))
						 - offset;
		return static_cast<int64>(tmp);
	}
};

template <std::size_t loLoByteIndex, std::size_t loHiByteIndex, std::size_t hiLoByteIndex, std::size_t hiHiByteIndex>
struct DecodeFloat32
{
	using input_t = std::byte;
	using output_t = somefloat32;
	static constexpr std::size_t input_inc = 4;
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		somefloat32 val = IEEE754binary32LE(inBuf[loLoByteIndex], inBuf[loHiByteIndex], inBuf[hiLoByteIndex], inBuf[hiHiByteIndex]);
		val = mpt::sanitize_nan(val);
		if(std::isinf(val))
		{
			if(val >= 0.0f)
			{
				val = 1.0f;
			} else
			{
				val = -1.0f;
			}
		}
		return val;
	}
};

template <std::size_t loLoByteIndex, std::size_t loHiByteIndex, std::size_t hiLoByteIndex, std::size_t hiHiByteIndex>
struct DecodeScaledFloat32
{
	using input_t = std::byte;
	using output_t = somefloat32;
	static constexpr std::size_t input_inc = 4;
	float factor;
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		somefloat32 val = IEEE754binary32LE(inBuf[loLoByteIndex], inBuf[loHiByteIndex], inBuf[hiLoByteIndex], inBuf[hiHiByteIndex]);
		val = mpt::sanitize_nan(val);
		if(std::isinf(val))
		{
			if(val >= 0.0f)
			{
				val = 1.0f;
			} else
			{
				val = -1.0f;
			}
		}
		return factor * val;
	}
	MPT_FORCEINLINE DecodeScaledFloat32(float scaleFactor)
		: factor(scaleFactor)
	{
		return;
	}
};

template <std::size_t b0, std::size_t b1, std::size_t b2, std::size_t b3, std::size_t b4, std::size_t b5, std::size_t b6, std::size_t b7>
struct DecodeFloat64
{
	using input_t = std::byte;
	using output_t = somefloat64;
	static constexpr std::size_t input_inc = 8;
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		somefloat64 val = IEEE754binary64LE(inBuf[b0], inBuf[b1], inBuf[b2], inBuf[b3], inBuf[b4], inBuf[b5], inBuf[b6], inBuf[b7]);
		val = mpt::sanitize_nan(val);
		if(std::isinf(val))
		{
			if(val >= 0.0)
			{
				val = 1.0;
			} else
			{
				val = -1.0;
			}
		}
		return val;
	}
};

template <typename Tsample>
struct DecodeIdentity
{
	using input_t = Tsample;
	using output_t = Tsample;
	static constexpr std::size_t input_inc = 1;
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		return *inBuf;
	}
};


// Reads sample data with Func and passes it directly to Func2.
// Func1::output_t and Func2::input_t must be identical
template <typename Func2, typename Func1>
struct ConversionChain
{
	using input_t = typename Func1::input_t;
	using output_t = typename Func2::output_t;
	static constexpr std::size_t input_inc = Func1::input_inc;
	Func1 func1;
	Func2 func2;
	MPT_FORCEINLINE output_t operator()(const input_t *inBuf)
	{
		return func2(func1(inBuf));
	}
	MPT_FORCEINLINE ConversionChain(Func2 f2 = Func2(), Func1 f1 = Func1())
		: func1(f1)
		, func2(f2)
	{
		return;
	}
};


}  // namespace SC


OPENMPT_NAMESPACE_END
