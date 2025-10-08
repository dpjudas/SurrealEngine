
#ifndef ASIO_ASIOSAMPLECONVERT_HPP
#define ASIO_ASIOSAMPLECONVERT_HPP



#include "ASIOVersion.hpp"
#include "ASIOConfig.hpp"
#include "ASIOCore.hpp"

#include <algorithm>
#include <array>
#include <iterator>
#include <limits>
#include <type_traits>

#include "ASIOstdcxx20bit.hpp"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>



namespace ASIO {



inline namespace ASIO_VERSION_NAMESPACE {



namespace Sample {



namespace detail {



template <typename Type>
struct TraitsExternal { };

template <>
struct TraitsExternal<std::int8_t> {
	using type                           = std::int8_t;
	using internal_type                  = std::int64_t;
	static constexpr internal_type scale = static_cast<internal_type>(1) << 24;
};
template <>
struct TraitsExternal<std::int16_t> {
	using type                           = std::int16_t;
	using internal_type                  = std::int64_t;
	static constexpr internal_type scale = static_cast<internal_type>(1) << 16;
};
template <>
struct TraitsExternal<std::int32_t> {
	using type                           = std::int32_t;
	using internal_type                  = std::int64_t;
	static constexpr internal_type scale = static_cast<internal_type>(1) << 0;
};
template <>
struct TraitsExternal<float> {
	using type          = float;
	using internal_type = float;
};
template <>
struct TraitsExternal<double> {
	using type          = double;
	using internal_type = double;
};
template <>
struct TraitsExternal<long double> {
	using type          = long double;
	using internal_type = double;
};



struct Int16MSB {
	std::array<std::byte, 2> data;
};
struct Int24MSB {
	std::array<std::byte, 3> data;
};
struct Int32MSB {
	std::array<std::byte, 4> data;
};
struct Int32MSB16 {
	std::array<std::byte, 4> data;
};
struct Int32MSB18 {
	std::array<std::byte, 4> data;
};
struct Int32MSB20 {
	std::array<std::byte, 4> data;
};
struct Int32MSB24 {
	std::array<std::byte, 4> data;
};
struct Float32MSB {
	std::array<std::byte, 4> data;
};
struct Float64MSB {
	std::array<std::byte, 8> data;
};

struct Int16LSB {
	std::array<std::byte, 2> data;
};
struct Int24LSB {
	std::array<std::byte, 3> data;
};
struct Int32LSB {
	std::array<std::byte, 4> data;
};
struct Int32LSB16 {
	std::array<std::byte, 4> data;
};
struct Int32LSB18 {
	std::array<std::byte, 4> data;
};
struct Int32LSB20 {
	std::array<std::byte, 4> data;
};
struct Int32LSB24 {
	std::array<std::byte, 4> data;
};
struct Float32LSB {
	std::array<std::byte, 4> data;
};
struct Float64LSB {
	std::array<std::byte, 8> data;
};



template <typename Type>
struct Types { };

template <>
struct Types<detail::Int16MSB> {
	using type                              = detail::Int16MSB;
	static constexpr SampleType sample_type = SampleType::Int16MSB;
	using internal_type                     = std::int64_t;
	using value_type                        = std::int16_t;
	using unsigned_type                     = std::make_unsigned<value_type>::type;
	static constexpr std::size_t valid_bits = 16;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = true;
};
template <>
struct Types<detail::Int24MSB> {
	using type                              = detail::Int24MSB;
	static constexpr SampleType sample_type = SampleType::Int24MSB;
	using internal_type                     = std::int64_t;
	using value_type                        = std::int32_t;
	using unsigned_type                     = std::make_unsigned<value_type>::type;
	static constexpr std::size_t valid_bits = 24;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = true;
};
template <>
struct Types<detail::Int32MSB> {
	using type                              = detail::Int32MSB;
	static constexpr SampleType sample_type = SampleType::Int32MSB;
	using internal_type                     = std::int64_t;
	using value_type                        = std::int32_t;
	using unsigned_type                     = std::make_unsigned<value_type>::type;
	static constexpr std::size_t valid_bits = 32;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = true;
};
template <>
struct Types<detail::Int32MSB16> {
	using type                              = detail::Int32MSB16;
	static constexpr SampleType sample_type = SampleType::Int32MSB16;
	using internal_type                     = std::int64_t;
	using value_type                        = std::int32_t;
	using unsigned_type                     = std::make_unsigned<value_type>::type;
	static constexpr std::size_t valid_bits = 16;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = true;
};
template <>
struct Types<detail::Int32MSB18> {
	using type                              = detail::Int32MSB18;
	static constexpr SampleType sample_type = SampleType::Int32MSB18;
	using internal_type                     = std::int64_t;
	using value_type                        = std::int32_t;
	using unsigned_type                     = std::make_unsigned<value_type>::type;
	static constexpr std::size_t valid_bits = 18;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = true;
};
template <>
struct Types<detail::Int32MSB20> {
	using type                              = detail::Int32MSB20;
	static constexpr SampleType sample_type = SampleType::Int32MSB20;
	using internal_type                     = std::int64_t;
	using value_type                        = std::int32_t;
	using unsigned_type                     = std::make_unsigned<value_type>::type;
	static constexpr std::size_t valid_bits = 20;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = true;
};
template <>
struct Types<detail::Int32MSB24> {
	using type                              = detail::Int32MSB24;
	static constexpr SampleType sample_type = SampleType::Int32MSB24;
	using internal_type                     = std::int64_t;
	using value_type                        = std::int32_t;
	using unsigned_type                     = std::make_unsigned<value_type>::type;
	static constexpr std::size_t valid_bits = 24;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = true;
};
template <>
struct Types<detail::Float32MSB> {
	using type                              = detail::Float32MSB;
	static constexpr SampleType sample_type = SampleType::Float32MSB;
	using internal_type                     = float;
	using value_type                        = float;
	using unsigned_type                     = float;
	static constexpr std::size_t valid_bits = 32;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = true;
};
template <>
struct Types<detail::Float64MSB> {
	using type                              = detail::Float64MSB;
	static constexpr SampleType sample_type = SampleType::Float64MSB;
	using internal_type                     = double;
	using value_type                        = double;
	using unsigned_type                     = double;
	static constexpr std::size_t valid_bits = 64;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = true;
};

template <>
struct Types<detail::Int16LSB> {
	using type                              = detail::Int16LSB;
	static constexpr SampleType sample_type = SampleType::Int16MSB;
	using internal_type                     = std::int64_t;
	using value_type                        = std::int16_t;
	using unsigned_type                     = std::make_unsigned<value_type>::type;
	static constexpr std::size_t valid_bits = 16;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = false;
};
template <>
struct Types<detail::Int24LSB> {
	using type                              = detail::Int24LSB;
	static constexpr SampleType sample_type = SampleType::Int24LSB;
	using internal_type                     = std::int64_t;
	using value_type                        = std::int32_t;
	using unsigned_type                     = std::make_unsigned<value_type>::type;
	static constexpr std::size_t valid_bits = 24;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = false;
};
template <>
struct Types<detail::Int32LSB> {
	using type                              = detail::Int32LSB;
	static constexpr SampleType sample_type = SampleType::Int32LSB;
	using internal_type                     = std::int64_t;
	using value_type                        = std::int32_t;
	using unsigned_type                     = std::make_unsigned<value_type>::type;
	static constexpr std::size_t valid_bits = 32;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = false;
};
template <>
struct Types<detail::Int32LSB16> {
	using type                              = detail::Int32LSB16;
	static constexpr SampleType sample_type = SampleType::Int32LSB16;
	using internal_type                     = std::int64_t;
	using value_type                        = std::int32_t;
	using unsigned_type                     = std::make_unsigned<value_type>::type;
	static constexpr std::size_t valid_bits = 16;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = false;
};
template <>
struct Types<detail::Int32LSB18> {
	using type                              = detail::Int32LSB18;
	static constexpr SampleType sample_type = SampleType::Int32LSB18;
	using internal_type                     = std::int64_t;
	using value_type                        = std::int32_t;
	using unsigned_type                     = std::make_unsigned<value_type>::type;
	static constexpr std::size_t valid_bits = 18;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = false;
};
template <>
struct Types<detail::Int32LSB20> {
	using type                              = detail::Int32LSB20;
	static constexpr SampleType sample_type = SampleType::Int32LSB20;
	using internal_type                     = std::int64_t;
	using value_type                        = std::int32_t;
	using unsigned_type                     = std::make_unsigned<value_type>::type;
	static constexpr std::size_t valid_bits = 20;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = false;
};
template <>
struct Types<detail::Int32LSB24> {
	using type                              = detail::Int32LSB24;
	static constexpr SampleType sample_type = SampleType::Int32LSB24;
	using internal_type                     = std::int64_t;
	using value_type                        = std::int32_t;
	using unsigned_type                     = std::make_unsigned<value_type>::type;
	static constexpr std::size_t valid_bits = 24;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = false;
};
template <>
struct Types<detail::Float32LSB> {
	using type                              = detail::Float32LSB;
	static constexpr SampleType sample_type = SampleType::Float32LSB;
	using internal_type                     = float;
	using value_type                        = float;
	using unsigned_type                     = float;
	static constexpr std::size_t valid_bits = 32;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = false;
};
template <>
struct Types<detail::Float64LSB> {
	using type                              = detail::Float64LSB;
	static constexpr SampleType sample_type = SampleType::Float64LSB;
	using internal_type                     = double;
	using value_type                        = double;
	using unsigned_type                     = double;
	static constexpr std::size_t valid_bits = 64;
	static constexpr bool        is_float   = std::is_floating_point<value_type>::value;
	static constexpr bool        is_be      = false;
};



template <typename T>
constexpr T clip(T x) noexcept = delete;
template <>
constexpr std::int64_t clip<std::int64_t>(std::int64_t q32) noexcept {
	return std::clamp(q32, static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::min()), static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::max()));
}
template <>
constexpr double clip<double>(double dbl) noexcept {
	return std::clamp(dbl, -1.0, 1.0);
}
template <>
constexpr float clip<float>(float flt) noexcept {
	return std::clamp(flt, -1.0f, 1.0f);
}



template <typename Tdst, typename Tsrc>
inline Tdst convert_internal(Tsrc x) noexcept = delete;
template <>
inline std::int64_t convert_internal<std::int64_t, std::int64_t>(std::int64_t q32) noexcept {
	return q32;
}
template <>
inline std::int64_t convert_internal<std::int64_t, double>(double dbl) noexcept {
	return std::llround(dbl * static_cast<double>(1ll << 32));
}
template <>
inline std::int64_t convert_internal<std::int64_t, float>(float flt) noexcept {
	return std::llround(flt * static_cast<float>(1ll << 32));
}
template <>
inline double convert_internal<double, std::int64_t>(std::int64_t q32) noexcept {
	return static_cast<double>(q32) * (1.0 / static_cast<double>(1ll << 32));
}
template <>
inline double convert_internal<double, double>(double dbl) noexcept {
	return dbl;
}
template <>
inline double convert_internal<double, float>(float flt) noexcept {
	return static_cast<double>(flt);
}
template <>
inline float convert_internal<float, std::int64_t>(std::int64_t q32) noexcept {
	return static_cast<float>(q32) * (1.0f / static_cast<float>(1ll << 32));
}
template <>
inline float convert_internal<float, double>(double dbl) noexcept {
	return static_cast<float>(dbl);
}
template <>
inline float convert_internal<float, float>(float flt) noexcept {
	return flt;
}



template <typename SampleType>
inline SampleType sample_from_value(typename Types<SampleType>::value_type val) noexcept {
	if constexpr (Types<SampleType>::is_float) {
		static_assert(sizeof(SampleType) == sizeof(typename Types<SampleType>::value_type));
		static_assert((stdcxx20::endian::native == stdcxx20::endian::little) || (stdcxx20::endian::native == stdcxx20::endian::big));
		SampleType result = stdcxx20::bit_cast<SampleType>(val);
		if constexpr (Types<SampleType>::is_be) {
			if constexpr (stdcxx20::endian::native == stdcxx20::endian::little) {
				std::reverse(std::begin(result.data), std::end(result.data));
			}
		} else {
			if constexpr (stdcxx20::endian::native == stdcxx20::endian::big) {
				std::reverse(std::begin(result.data), std::end(result.data));
			}
		}
		return result;
	} else {
		SampleType                                result{};
		typename Types<SampleType>::unsigned_type uval = static_cast<typename Types<SampleType>::unsigned_type>(val);
		for (std::size_t byte = 0; byte < sizeof(SampleType); ++byte) {
			if constexpr (Types<SampleType>::is_be) {
				result.data[sizeof(SampleType) - 1 - byte] = std::byte{static_cast<std::uint8_t>(uval)};
			} else {
				result.data[byte] = std::byte{static_cast<std::uint8_t>(uval)};
			}
			uval /= (1u << 8);
		}
		return result;
	}
}

template <typename SampleType>
inline typename Types<SampleType>::value_type value_from_sample(SampleType smp) noexcept {
	if constexpr (Types<SampleType>::is_float) {
		static_assert(sizeof(SampleType) == sizeof(typename Types<SampleType>::value_type));
		static_assert((stdcxx20::endian::native == stdcxx20::endian::little) || (stdcxx20::endian::native == stdcxx20::endian::big));
		if constexpr (Types<SampleType>::is_be) {
			if constexpr (stdcxx20::endian::native == stdcxx20::endian::little) {
				std::reverse(std::begin(smp.data), std::end(smp.data));
			}
		} else {
			if constexpr (stdcxx20::endian::native == stdcxx20::endian::big) {
				std::reverse(std::begin(smp.data), std::end(smp.data));
			}
		}
		return stdcxx20::bit_cast<typename Types<SampleType>::value_type>(smp.data);
	} else {
		typename Types<SampleType>::unsigned_type uval = 0;
		for (std::size_t byte = 0; byte < sizeof(SampleType); ++byte) {
			uval *= (1u << 8);
			if constexpr (Types<SampleType>::is_be) {
				uval |= std::to_integer<std::uint8_t>(smp.data[byte]);
			} else {
				uval |= std::to_integer<std::uint8_t>(smp.data[sizeof(SampleType) - 1 - byte]);
			}
		}
		return static_cast<typename Types<SampleType>::value_type>(uval);
	}
}



template <typename SampleType>
inline typename Types<SampleType>::value_type value_from_internal(typename Types<SampleType>::internal_type smp_int) noexcept {
	if constexpr (Types<SampleType>::is_float) {
		return static_cast<typename Types<SampleType>::value_type>(smp_int);
	} else {
		return static_cast<typename Types<SampleType>::value_type>(clip(smp_int) / (static_cast<typename Types<SampleType>::internal_type>(1) << (32 - Types<SampleType>::valid_bits)));
	}
}

template <typename SampleType>
inline typename Types<SampleType>::internal_type internal_from_value(typename Types<SampleType>::value_type val) noexcept {
	if constexpr (Types<SampleType>::is_float) {
		return static_cast<typename Types<SampleType>::internal_type>(val);
	} else {
		return clip(static_cast<typename Types<SampleType>::internal_type>(val) * (static_cast<typename Types<SampleType>::internal_type>(1) << (32 - Types<SampleType>::valid_bits)));
	}
}



template <typename ExternalType>
inline typename TraitsExternal<ExternalType>::internal_type internal_from_external(ExternalType x) noexcept {
	static_assert(std::is_floating_point<typename TraitsExternal<ExternalType>::internal_type>::value == std::is_floating_point<ExternalType>::value);
	if constexpr (std::is_floating_point<ExternalType>::value) {
		return static_cast<typename TraitsExternal<ExternalType>::internal_type>(x);
	} else {
		return static_cast<typename TraitsExternal<ExternalType>::internal_type>(x) * TraitsExternal<ExternalType>::scale;
	}
}

template <typename ExternalType>
inline ExternalType external_from_internal(typename TraitsExternal<ExternalType>::internal_type x) noexcept {
	static_assert(std::is_floating_point<typename TraitsExternal<ExternalType>::internal_type>::value == std::is_floating_point<ExternalType>::value);
	if constexpr (std::is_floating_point<ExternalType>::value) {
		return static_cast<ExternalType>(x);
	} else {
		return static_cast<ExternalType>(clip(x) / TraitsExternal<ExternalType>::scale);
	}
}



template <typename SampleType, typename ExternalType>
inline SampleType sample_from_external(ExternalType x) noexcept {
	return sample_from_value<SampleType>(value_from_internal<SampleType>(convert_internal<typename Types<SampleType>::internal_type, typename TraitsExternal<ExternalType>::internal_type>(internal_from_external<ExternalType>(x))));
}

template <typename ExternalType, typename SampleType>
inline ExternalType external_from_sample(SampleType x) noexcept {
	return external_from_internal<ExternalType>(convert_internal<typename TraitsExternal<ExternalType>::internal_type, typename Types<SampleType>::internal_type>(internal_from_value<SampleType>(value_from_sample<SampleType>(x))));
}



template <typename Tdst>
inline void ClearBuffer(Tdst * dst, std::size_t dst_stride, std::size_t count) noexcept {
	for (std::size_t i = 0; i < count; ++i)
	{
		Tdst val{};
		std::fill(val.data.begin(), val.data.end(), std::byte{0});
		*dst = val;
		dst += dst_stride;
	}
}



template <typename Tdst, typename Tsrc, typename Tfunc>
inline void ConvertBuffer(Tdst * dst, std::size_t dst_stride, const Tsrc * src, std::size_t src_stride, std::size_t count, Tfunc func) noexcept(noexcept(func(Tsrc()))) {
	for (std::size_t i = 0; i < count; ++i)
	{
		*dst = func(*src);
		src += src_stride;
		dst += dst_stride;
	}
}



} // namespace detail



inline void ClearBufferASIO(void * dst, SampleType type, std::size_t count) noexcept {
	switch (type) {
		case SampleType::Int16MSB:
			{
				using SampleType = detail::Int16MSB;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Int16LSB:
			{
				using SampleType = detail::Int16LSB;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Int24MSB:
			{
				using SampleType = detail::Int24MSB;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Int24LSB:
			{
				using SampleType = detail::Int24LSB;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Int32MSB:
			{
				using SampleType = detail::Int32MSB;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Int32LSB:
			{
				using SampleType = detail::Int32LSB;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Float32MSB:
			{
				using SampleType = detail::Float32MSB;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Float32LSB:
			{
				using SampleType = detail::Float32LSB;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Float64MSB:
			{
				using SampleType = detail::Float64MSB;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Float64LSB:
			{
				using SampleType = detail::Float64LSB;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Int32MSB16:
			{
				using SampleType = detail::Int32MSB16;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Int32LSB16:
			{
				using SampleType = detail::Int32LSB16;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Int32MSB18:
			{
				using SampleType = detail::Int32MSB18;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Int32LSB18:
			{
				using SampleType = detail::Int32LSB18;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Int32MSB20:
			{
				using SampleType = detail::Int32MSB20;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Int32LSB20:
			{
				using SampleType = detail::Int32LSB20;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Int32MSB24:
			{
				using SampleType = detail::Int32MSB24;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		case SampleType::Int32LSB24:
			{
				using SampleType = detail::Int32LSB24;
				detail::ClearBuffer(static_cast<SampleType *>(dst), 1, count);
			}
			break;
		default:
			assert(false);
			break;
	}
}


template <typename Tdst>
inline void CopyRawFromASIO(Tdst * dst, std::size_t dst_stride, const void * src, std::size_t count) noexcept {
	detail::ConvertBuffer(dst, dst_stride, static_cast<const Tdst *>(src), 1, count, [](Tdst smp) { return smp; });
}

template <typename Tsrc>
inline void CopyRawToASIO(void * dst, const Tsrc * src, std::size_t src_stride, std::size_t count) noexcept {
	detail::ConvertBuffer(static_cast<Tsrc *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return smp; });
}



template <typename Tdst>
inline void ConvertFromASIO(Tdst * dst, std::size_t dst_stride, SampleType type, const void * src, std::size_t count) noexcept {
	switch (type) {
		case SampleType::Int16MSB:
			{
				using SampleType = detail::Int16MSB;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Int16LSB:
			{
				using SampleType = detail::Int16LSB;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Int24MSB:
			{
				using SampleType = detail::Int24MSB;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Int24LSB:
			{
				using SampleType = detail::Int24LSB;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Int32MSB:
			{
				using SampleType = detail::Int32MSB;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Int32LSB:
			{
				using SampleType = detail::Int32LSB;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Float32MSB:
			{
				using SampleType = detail::Float32MSB;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Float32LSB:
			{
				using SampleType = detail::Float32LSB;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Float64MSB:
			{
				using SampleType = detail::Float64MSB;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Float64LSB:
			{
				using SampleType = detail::Float64LSB;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Int32MSB16:
			{
				using SampleType = detail::Int32MSB16;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Int32LSB16:
			{
				using SampleType = detail::Int32LSB16;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Int32MSB18:
			{
				using SampleType = detail::Int32MSB18;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Int32LSB18:
			{
				using SampleType = detail::Int32LSB18;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Int32MSB20:
			{
				using SampleType = detail::Int32MSB20;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Int32LSB20:
			{
				using SampleType = detail::Int32LSB20;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Int32MSB24:
			{
				using SampleType = detail::Int32MSB24;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		case SampleType::Int32LSB24:
			{
				using SampleType = detail::Int32LSB24;
				detail::ConvertBuffer(dst, dst_stride, static_cast<const SampleType *>(src), 1, count, [](SampleType smp) { return detail::external_from_sample<Tdst, SampleType>(smp); });
			}
			break;
		default:
			assert(false);
			break;
	}
}

template <typename Tsrc>
inline void ConvertToASIO(void * dst, SampleType type, const Tsrc * src, std::size_t src_stride, std::size_t count) noexcept {
	switch (type) {
		case SampleType::Int16MSB:
			{
				using SampleType = detail::Int16MSB;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Int16LSB:
			{
				using SampleType = detail::Int16LSB;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Int24MSB:
			{
				using SampleType = detail::Int24MSB;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Int24LSB:
			{
				using SampleType = detail::Int24LSB;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Int32MSB:
			{
				using SampleType = detail::Int32MSB;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Int32LSB:
			{
				using SampleType = detail::Int32LSB;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Float32MSB:
			{
				using SampleType = detail::Float32MSB;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Float32LSB:
			{
				using SampleType = detail::Float32LSB;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Float64MSB:
			{
				using SampleType = detail::Float64MSB;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Float64LSB:
			{
				using SampleType = detail::Float64LSB;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Int32MSB16:
			{
				using SampleType = detail::Int32MSB16;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Int32LSB16:
			{
				using SampleType = detail::Int32LSB16;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Int32MSB18:
			{
				using SampleType = detail::Int32MSB18;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Int32LSB18:
			{
				using SampleType = detail::Int32LSB18;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Int32MSB20:
			{
				using SampleType = detail::Int32MSB20;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Int32LSB20:
			{
				using SampleType = detail::Int32LSB20;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Int32MSB24:
			{
				using SampleType = detail::Int32MSB24;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		case SampleType::Int32LSB24:
			{
				using SampleType = detail::Int32LSB24;
				detail::ConvertBuffer(static_cast<SampleType *>(dst), 1, src, src_stride, count, [](Tsrc smp) { return detail::sample_from_external<SampleType, Tsrc>(smp); });
			}
			break;
		default:
			assert(false);
			break;
	}
}



struct Traits {
	std::size_t size_bytes = 0;
	std::size_t valid_bits = 0;
	bool        is_float   = false;
	bool        is_be      = false;
	constexpr explicit Traits(SampleType type) noexcept {
		switch (type) {
			case SampleType::Int16MSB:
				{
					using Type = detail::Types<detail::Int16MSB>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Int24MSB:
				{
					using Type = detail::Types<detail::Int24MSB>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Int32MSB:
				{
					using Type = detail::Types<detail::Int32MSB>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Float32MSB:
				{
					using Type = detail::Types<detail::Float32MSB>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Float64MSB:
				{
					using Type = detail::Types<detail::Float64MSB>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Int32MSB16:
				{
					using Type = detail::Types<detail::Int32MSB16>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Int32MSB18:
				{
					using Type = detail::Types<detail::Int32MSB18>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Int32MSB20:
				{
					using Type = detail::Types<detail::Int32MSB20>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Int32MSB24:
				{
					using Type = detail::Types<detail::Int32MSB24>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Int16LSB:
				{
					using Type = detail::Types<detail::Int16LSB>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Int24LSB:
				{
					using Type = detail::Types<detail::Int24LSB>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Int32LSB:
				{
					using Type = detail::Types<detail::Int32LSB>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Float32LSB:
				{
					using Type = detail::Types<detail::Float32LSB>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Float64LSB:
				{
					using Type = detail::Types<detail::Float64LSB>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Int32LSB16:
				{
					using Type = detail::Types<detail::Int32LSB16>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Int32LSB18:
				{
					using Type = detail::Types<detail::Int32LSB18>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Int32LSB20:
				{
					using Type = detail::Types<detail::Int32LSB20>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			case SampleType::Int32LSB24:
				{
					using Type = detail::Types<detail::Int32LSB24>;
					size_bytes = sizeof(Type::type);
					valid_bits = Type::valid_bits;
					is_float   = Type::is_float;
					is_be      = Type::is_be;
				}
				break;
			default:
				break;
		}
	}
};



} // namespace Sample



} // namespace ASIO_VERSION_NAMESPACE



} // namespace ASIO



#endif // ASIO_ASIOSAMPLECONVERT_HPP