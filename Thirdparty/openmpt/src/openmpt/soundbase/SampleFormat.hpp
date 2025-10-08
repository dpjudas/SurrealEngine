/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/macros.hpp"
#include "mpt/base/utility.hpp"
#include "openmpt/base/Int24.hpp"
#include "openmpt/base/Types.hpp"

#include <type_traits>


OPENMPT_NAMESPACE_BEGIN


class SampleFormat
{
public:
	enum class Enum : uint8
	{
		Unsigned8 = 9,       // do not change value (for compatibility with old configuration settings)
		Int8 = 8,            // do not change value (for compatibility with old configuration settings)
		Int16 = 16,          // do not change value (for compatibility with old configuration settings)
		Int24 = 24,          // do not change value (for compatibility with old configuration settings)
		Int32 = 32,          // do not change value (for compatibility with old configuration settings)
		Float32 = 32 + 128,  // do not change value (for compatibility with old configuration settings)
		Float64 = 64 + 128,  // do not change value (for compatibility with old configuration settings)
		Default = Float32
	};
	static constexpr SampleFormat::Enum Unsigned8 = SampleFormat::Enum::Unsigned8;
	static constexpr SampleFormat::Enum Int8 = SampleFormat::Enum::Int8;
	static constexpr SampleFormat::Enum Int16 = SampleFormat::Enum::Int16;
	static constexpr SampleFormat::Enum Int24 = SampleFormat::Enum::Int24;
	static constexpr SampleFormat::Enum Int32 = SampleFormat::Enum::Int32;
	static constexpr SampleFormat::Enum Float32 = SampleFormat::Enum::Float32;
	static constexpr SampleFormat::Enum Float64 = SampleFormat::Enum::Float64;
	static constexpr SampleFormat::Enum Default = SampleFormat::Enum::Default;

private:
	SampleFormat::Enum value;

	template <typename T>
	static MPT_CONSTEXPRINLINE SampleFormat::Enum Sanitize(T x) noexcept
	{
		using uT = typename std::make_unsigned<T>::type;
		uT val = static_cast<uT>(x);
		if(!val)
		{
			return SampleFormat::Enum::Default;
		}
		if(val == static_cast<uT>(-8))
		{
			val = 8 + 1;
		}
		// float|64|32|16|8|?|?|unsigned
		val &= 0b1'1111'00'1;
		auto is_float = [](uT val) -> bool
		{
			return (val & 0b1'0000'00'0) ? true : false;
		};
		auto is_unsigned = [](uT val) -> bool
		{
			return (val & 0b0'0000'00'1) ? true : false;
		};
		auto has_size = [](uT val) -> bool
		{
			return (val & 0b0'1111'00'0) ? true : false;
		};
		auto unique_size = [](uT val) -> bool
		{
			val &= 0b0'1111'00'0;
			if(val == 0b0'0001'00'0)
			{
				return true;
			} else if(val == 0b0'0010'00'0)
			{
				return true;
			} else if(val == 0b0'0011'00'0)
			{
				return true;
			} else if(val == 0b0'0100'00'0)
			{
				return true;
			} else if(val == 0b0'1000'00'0)
			{
				return true;
			} else
			{
				return false;
			}
		};
		auto get_size = [](uT val) -> std::size_t
		{
			val &= 0b0'1111'00'0;
			if(val == 0b0'0001'00'0)
			{
				return 1;
			} else if(val == 0b0'0010'00'0)
			{
				return 2;
			} else if(val == 0b0'0011'00'0)
			{
				return 3;
			} else if(val == 0b0'0100'00'0)
			{
				return 4;
			} else if(val == 0b0'1000'00'0)
			{
				return 8;
			} else
			{
				return 2;  // default to 16bit
			}
		};
		if(!has_size(val))
		{
			if(is_unsigned(val) && is_float(val))
			{
				val = mpt::to_underlying(Enum::Default);
			} else if(is_unsigned(val))
			{
				val = mpt::to_underlying(Enum::Unsigned8);
			} else if(is_float(val))
			{
				val = mpt::to_underlying(Enum::Float32);
			} else
			{
				val = mpt::to_underlying(Enum::Default);
			}
		} else if(!unique_size(val))
		{
			// order of size check matters
			if((val & 0b0'0011'00'0) == 0b0'0011'00'0)
			{
				val = mpt::to_underlying(Enum::Int24);
			} else if(val & 0b0'0010'00'0)
			{
				val = mpt::to_underlying(Enum::Int16);
			} else if(val & 0b0'0100'00'0)
			{
				if(is_float(val))
				{
					val = mpt::to_underlying(Enum::Float32);
				} else
				{
					val = mpt::to_underlying(Enum::Int32);
				}
			} else if(val & 0b0'1000'00'0)
			{
				val = mpt::to_underlying(Enum::Float64);
			} else if(val & 0b0'0001'00'0)
			{
				if(is_unsigned(val))
				{
					val = mpt::to_underlying(Enum::Unsigned8);
				} else
				{
					val = mpt::to_underlying(Enum::Int8);
				}
			}
		} else
		{
			if(is_unsigned(val) && (get_size(val) > 1))
			{
				val &= ~0b0'0000'00'1;  // remove unsigned
			}
			if(is_float(val) && (get_size(val) < 4))
			{
				val &= ~0b1'0000'00'0;  // remove float
			}
			if(!is_float(val) && (get_size(val) == 8))
			{
				val |= 0b1'0000'00'0;  // add float
			}
		}
		return static_cast<SampleFormat::Enum>(val);
	}

public:
	MPT_CONSTEXPRINLINE SampleFormat() noexcept
		: value(SampleFormat::Default)
	{
	}

	MPT_CONSTEXPRINLINE SampleFormat(SampleFormat::Enum v) noexcept
		: value(Sanitize(v))
	{
	}

	friend MPT_CONSTEXPRINLINE bool operator==(const SampleFormat &a, const SampleFormat &b) noexcept
	{
		return a.value == b.value;
	}
	friend MPT_CONSTEXPRINLINE bool operator!=(const SampleFormat &a, const SampleFormat &b) noexcept
	{
		return a.value != b.value;
	}
	friend MPT_CONSTEXPRINLINE bool operator==(const SampleFormat::Enum &a, const SampleFormat &b) noexcept
	{
		return a == b.value;
	}
	friend MPT_CONSTEXPRINLINE bool operator!=(const SampleFormat::Enum &a, const SampleFormat &b) noexcept
	{
		return a != b.value;
	}
	friend MPT_CONSTEXPRINLINE bool operator==(const SampleFormat &a, const SampleFormat::Enum &b) noexcept
	{
		return a.value == b;
	}
	friend MPT_CONSTEXPRINLINE bool operator!=(const SampleFormat &a, const SampleFormat::Enum &b) noexcept
	{
		return a.value != b;
	}

	MPT_CONSTEXPRINLINE bool IsUnsigned() const noexcept
	{
		return false
			|| (value == SampleFormat::Unsigned8);
	}
	MPT_CONSTEXPRINLINE bool IsFloat() const noexcept
	{
		return false
			|| (value == SampleFormat::Float32)
			|| (value == SampleFormat::Float64);
	}
	MPT_CONSTEXPRINLINE bool IsInt() const noexcept
	{
		return false
			|| (value == SampleFormat::Unsigned8)
			|| (value == SampleFormat::Int8)
			|| (value == SampleFormat::Int16)
			|| (value == SampleFormat::Int24)
			|| (value == SampleFormat::Int32);
	}
	MPT_CONSTEXPRINLINE uint8 GetSampleSize() const noexcept
	{
		return false                              ? 0
			 : (value == SampleFormat::Unsigned8) ? 1
			 : (value == SampleFormat::Int8)      ? 1
			 : (value == SampleFormat::Int16)     ? 2
			 : (value == SampleFormat::Int24)     ? 3
			 : (value == SampleFormat::Int32)     ? 4
			 : (value == SampleFormat::Float32)   ? 4
			 : (value == SampleFormat::Float64)   ? 8
												  : 0;
	}
	MPT_CONSTEXPRINLINE uint8 GetBitsPerSample() const noexcept
	{
		return false                              ? 0
			 : (value == SampleFormat::Unsigned8) ? 8
			 : (value == SampleFormat::Int8)      ? 8
			 : (value == SampleFormat::Int16)     ? 16
			 : (value == SampleFormat::Int24)     ? 24
			 : (value == SampleFormat::Int32)     ? 32
			 : (value == SampleFormat::Float32)   ? 32
			 : (value == SampleFormat::Float64)   ? 64
												  : 0;
	}

	MPT_CONSTEXPRINLINE operator SampleFormat::Enum() const noexcept
	{
		return value;
	}

	// backward compatibility, conversion to/from integers
	static MPT_CONSTEXPRINLINE SampleFormat FromInt(int x) noexcept
	{
		return SampleFormat(Sanitize(x));
	}
	static MPT_CONSTEXPRINLINE int ToInt(SampleFormat x) noexcept
	{
		return mpt::to_underlying(x.value);
	}
};


template <typename Container>
Container AllSampleFormats()
{
	return {SampleFormat::Float64, SampleFormat::Float32, SampleFormat::Int32, SampleFormat::Int24, SampleFormat::Int16, SampleFormat::Int8, SampleFormat::Unsigned8};
}

template <typename Container>
Container DefaultSampleFormats()
{
	return {SampleFormat::Float32, SampleFormat::Int32, SampleFormat::Int24, SampleFormat::Int16, SampleFormat::Int8};
}

template <typename Tsample>
struct SampleFormatTraits;
template <>
struct SampleFormatTraits<uint8>
{
	static MPT_CONSTEXPRINLINE SampleFormat sampleFormat() { return SampleFormat::Unsigned8; }
};
template <>
struct SampleFormatTraits<int8>
{
	static MPT_CONSTEXPRINLINE SampleFormat sampleFormat() { return SampleFormat::Int8; }
};
template <>
struct SampleFormatTraits<int16>
{
	static MPT_CONSTEXPRINLINE SampleFormat sampleFormat() { return SampleFormat::Int16; }
};
template <>
struct SampleFormatTraits<int24>
{
	static MPT_CONSTEXPRINLINE SampleFormat sampleFormat() { return SampleFormat::Int24; }
};
template <>
struct SampleFormatTraits<int32>
{
	static MPT_CONSTEXPRINLINE SampleFormat sampleFormat() { return SampleFormat::Int32; }
};
template <>
struct SampleFormatTraits<float>
{
	static MPT_CONSTEXPRINLINE SampleFormat sampleFormat() { return SampleFormat::Float32; }
};
template <>
struct SampleFormatTraits<double>
{
	static MPT_CONSTEXPRINLINE SampleFormat sampleFormat() { return SampleFormat::Float64; }
};

template <SampleFormat::Enum sampleFormat>
struct SampleFormatToType;
template <>
struct SampleFormatToType<SampleFormat::Unsigned8>
{
	typedef uint8 type;
};
template <>
struct SampleFormatToType<SampleFormat::Int8>
{
	typedef int8 type;
};
template <>
struct SampleFormatToType<SampleFormat::Int16>
{
	typedef int16 type;
};
template <>
struct SampleFormatToType<SampleFormat::Int24>
{
	typedef int24 type;
};
template <>
struct SampleFormatToType<SampleFormat::Int32>
{
	typedef int32 type;
};
template <>
struct SampleFormatToType<SampleFormat::Float32>
{
	typedef float type;
};
template <>
struct SampleFormatToType<SampleFormat::Float64>
{
	typedef double type;
};



OPENMPT_NAMESPACE_END
