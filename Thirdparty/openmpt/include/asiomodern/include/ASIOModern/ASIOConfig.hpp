
#ifndef ASIO_ASIOCONFIG_HPP
#define ASIO_ASIOCONFIG_HPP



#include "ASIOVersion.hpp"



#include <algorithm>
#include <string>
#include <string_view>

#include <cstddef>
#include <cstdint>
#include <cstring>



#if defined(_WIN32)
#define ASIO_SYSTEM_WINDOWS 1
#endif

#ifndef ASIO_SYSTEM_WINDOWS
#define ASIO_SYSTEM_WINDOWS 0
#endif


#if defined(__clang__)
#define ASIO_COMPILER_CLANG 1
#elif defined(_MSC_VER)
#define ASIO_COMPILER_MSVC 1
#elif defined(__GNUC__)
#define ASIO_COMPILER_GCC 1
#endif

#ifndef ASIO_COMPILER_CLANG
#define ASIO_COMPILER_CLANG 0
#endif
#ifndef ASIO_COMPILER_MSVC
#define ASIO_COMPILER_MSVC 0
#endif
#ifndef ASIO_COMPILER_GCC
#define ASIO_COMPILER_GCC 0
#endif


#if ASIO_COMPILER_MSVC
#if defined(_MSVC_LANG)
#define ASIO_CXX_VERSION _MSVC_LANG
#else
#define ASIO_CXX_VERSION __cplusplus
#endif
#else
#define ASIO_CXX_VERSION __cplusplus
#endif


#if ASIO_COMPILER_MSVC || ASIO_COMPILER_CLANG || ASIO_COMPILER_GCC || ASIO_SYSTEM_WINDOWS
// assume #pragma pack support on Windows
#define ASIO_HAVE_PRAGMA_PACK 1
#else
#define ASIO_HAVE_PRAGMA_PACK 0
#endif


#if ASIO_COMPILER_MSVC || ASIO_COMPILER_CLANG || ASIO_SYSTEM_WINDOWS
// assume #pragma comment lib support on Windows
#define ASIO_HAVE_PRAGMA_COMMENT_LIB 1
#else
#define ASIO_HAVE_PRAGMA_COMMENT_LIB 0
#endif



#if ASIO_SYSTEM_WINDOWS
#include <objbase.h>
#endif // ASIO_SYSTEM_WINDOWS



namespace ASIO {



inline namespace ASIO_VERSION_NAMESPACE {



#define ASIO_PP_DEFER(m, ...) m(__VA_ARGS__)

#define ASIO_PP_STRINGIFY(x) #x

#define ASIO_PP_JOIN_HELPER(a, b) a##b
#define ASIO_PP_JOIN(a, b)        ASIO_PP_JOIN_HELPER(a, b)

#define ASIO_PP_UNIQUE_IDENTIFIER(prefix) ASIO_PP_JOIN(prefix, __LINE__)



#if ASIO_COMPILER_MSVC
#define ASIO_WARNING(text) __pragma(message(__FILE__ "(" ASIO_PP_DEFER(ASIO_PP_STRINGIFY, __LINE__) "): Warning: " text))
#elif ASIO_COMPILER_GCC || ASIO_COMPILER_CLANG
#define ASIO_WARNING(text) _Pragma(MPT_PP_STRINGIFY(GCC warning text))
#else
#define ASIO_WARNING(text) \
	static inline int ASIO_PP_UNIQUE_IDENTIFIER(ASIO_WARNING_NAME)() noexcept { \
		int warning [[deprecated("Warning: " text)]] = 0; \
		return warning; \
	} \
/**/
#endif



#if ASIO_SYSTEM_WINDOWS


#if !ASIO_HAVE_PRAGMA_PACK
#error "ASIO on Windows requires #pragma pack support"
#endif // !ASIO_HAVE_PRAGMA_PACK

#define ASIO_INTERFACE interface

#if ASIO_COMPILER_MSVC
#define ASIO_ATTR_DRIVERCALL
#define ASIO_DRIVERCALL __thiscall
#elif ASIO_COMPILER_CLANG
#define ASIO_ATTR_DRIVERCALL
#define ASIO_DRIVERCALL __thiscall
#elif ASIO_COMPILER_GCC
#pragma push_macro("thiscall")
#ifdef thiscall
#undef thiscall
#endif
#define ASIO_ATTR_DRIVERCALL [[gnu::thiscall]]
#define ASIO_DRIVERCALL
#pragma pop_macro("thiscall")
#else
#define ASIO_ATTR_DRIVERCALL
#define ASIO_DRIVERCALL __thiscall
#endif

#if ASIO_COMPILER_MSVC
#define ASIO_ATTR_CALL
#define ASIO_CALL __cdecl
#elif ASIO_COMPILER_CLANG
#define ASIO_ATTR_CALL
#define ASIO_CALL __cdecl
#elif ASIO_COMPILER_GCC
#pragma push_macro("cdecl")
#ifdef cdecl
#undef cdecl
#endif
#define ASIO_ATTR_CALL [[gnu::cdecl]]
#define ASIO_CALL
#pragma pop_macro("cdecl")
#else
#define ASIO_ATTR_CALL
#define ASIO_CALL __cdecl
#endif


#else // !ASIO_SYSTEM_WINDOWS


#error "Only Windows is supported by this ASIO header"


#define ASIO_INTERFACE struct

#define ASIO_ATTR_DRIVERCALL
#define ASIO_DRIVERCALL

#define ASIO_ATTR_CALL
#define ASIO_CALL


#endif // ASIO_SYSTEM_WINDOWS



inline namespace Core {



#if ASIO_SYSTEM_WINDOWS


using SysHandle = void *;

using Byte = std::uint8_t;

using Long = std::int32_t;

using ULong = std::uint32_t;

using LongLong = std::int64_t;

using ULongLong = std::uint64_t;

using Double = double;

using Char = char;

using Padding1 = std::uint8_t;

using PaddingLong = ULong;


#else // !ASIO_SYSTEM_WINDOWS


#error "Only Windows is supported by this ASIO header"


#define ASIO_INTERFACE struct

#define ASIO_ATTR_DRIVERCALL
#define ASIO_DRIVERCALL

#define ASIO_ATTR_CALL
#define ASIO_CALL

using SysHandle = void *;

using Byte = unsigned char;

using Long = signed long;

using ULong = unsigned long;

using LongLong = signed long long;

using ULongLong = unsigned long long;

using Double = double;

using Char = char;

using Padding1 = std::uint8_t;

using PaddingLong = ULong;


#endif // ASIO_SYSTEM_WINDOWS



struct Bool {
private:
	ULong m_val;

public:
	constexpr Bool() noexcept
		: m_val(0) { }
	constexpr Bool(bool val) noexcept
		: m_val(val ? 1 : 0) { }
	constexpr explicit Bool(ULong val) noexcept
		: m_val(val ? 1 : 0) { }
	constexpr bool operator!() const noexcept {
		return m_val ? false : true;
	}
	constexpr operator bool() const noexcept {
		return m_val ? true : false;
	}
};

struct HiLoLongLong {
private:
	ULong m_hi;
	ULong m_lo;

public:
	constexpr HiLoLongLong() noexcept
		: m_hi(0)
		, m_lo(0) {
	}
	constexpr HiLoLongLong(LongLong val) noexcept
		: m_hi(static_cast<ULong>((static_cast<ULongLong>(val) & 0xffffffff00000000ull) >> 32))
		, m_lo(static_cast<ULong>((static_cast<ULongLong>(val) & 0x00000000ffffffffull) >> 0)) {
	}
	constexpr operator LongLong() const noexcept {
		return static_cast<LongLong>((static_cast<ULongLong>(m_hi) << 32) | (static_cast<ULongLong>(m_lo) << 0));
	}
};

using ResultBool = ULong;

template <std::size_t size>
struct CharBuf {
private:
	Char buf[size] = "";

public:
	CharBuf()                            = default;
	CharBuf(const CharBuf &)             = default;
	CharBuf(CharBuf &&)                  = default;
	CharBuf & operator=(const CharBuf &) = default;
	CharBuf & operator=(CharBuf &&)      = default;

public:
	constexpr CharBuf(std::nullptr_t) noexcept
		: CharBuf() {
	}
	inline CharBuf(const char * str) noexcept
		: CharBuf() {
		if (str) {
			std::copy(str, str + std::min(std::strlen(str), size - 1), buf);
			std::fill(buf + std::min(std::strlen(str), size - 1), buf + size, Char('\0'));
		}
	}
	inline CharBuf(const std::string_view & str) noexcept
		: CharBuf() {
		std::copy(str.data(), str.data() + std::min(str.length(), size - 1), buf);
		std::fill(buf + std::min(str.length(), size - 1), buf + size, Char('\0'));
	}
	inline CharBuf(const std::string & str) noexcept
		: CharBuf() {
		std::copy(str.data(), str.data() + std::min(str.length(), size - 1), buf);
		std::fill(buf + std::min(str.length(), size - 1), buf + size, Char('\0'));
	}
	inline CharBuf & operator=(std::nullptr_t) noexcept {
		std::fill(buf, buf + size, Char('\0'));
		return *this;
	}
	inline CharBuf & operator=(const char * str) noexcept {
		if (str) {
			std::copy(str, str + std::min(std::strlen(str), size - 1), buf);
			std::fill(buf + std::min(std::strlen(str), size - 1), buf + size, Char('\0'));
		} else {
			std::fill(buf, buf + size, Char('\0'));
		}
		return *this;
	}
	inline CharBuf & operator=(const std::string & str) noexcept {
		std::fill(buf, buf + size, Char('\0'));
		std::copy(str.data(), str.data() + std::min(str.length(), size - 1), buf);
		std::fill(buf + std::min(str.length(), size - 1), buf + size, Char('\0'));
		return *this;
	}
	inline CharBuf & operator=(const std::string_view & str) noexcept {
		std::fill(buf, buf + size, Char('\0'));
		std::copy(str.data(), str.data() + std::min(str.length(), size - 1), buf);
		std::fill(buf + std::min(str.length(), size - 1), buf + size, Char('\0'));
		return *this;
	}
	inline explicit operator std::string_view() const noexcept {
		std::size_t len = std::find(buf, buf + size - 1, Char('\0')) - buf;
		return std::string_view(buf, buf + len);
	}
	inline operator std::string() const {
		std::size_t len = std::find(buf, buf + size - 1, Char('\0')) - buf;
		return std::string(buf, buf + len);
	}
};

inline constexpr std::size_t SizeOfChar         = sizeof(Char);
inline constexpr std::size_t SizeOfByte         = sizeof(Byte);
inline constexpr std::size_t SizeOfBool         = sizeof(Bool);
inline constexpr std::size_t SizeOfLong         = sizeof(Long);
inline constexpr std::size_t SizeOfLongLong     = sizeof(LongLong);
inline constexpr std::size_t SizeOfDouble       = sizeof(Double);
inline constexpr std::size_t SizeOfHiLoLongLong = sizeof(HiLoLongLong);

static_assert(SizeOfChar == 1);
static_assert(SizeOfByte == 1);

static_assert(SizeOfBool == SizeOfLong);

static_assert(SizeOfHiLoLongLong == SizeOfLongLong);

static_assert(SizeOfLongLong == 8);

static_assert(sizeof(Padding1) == 1);

static_assert(sizeof(PaddingLong) == SizeOfLong);

static_assert(sizeof(CharBuf<1>) == 1);



} // namespace Core



} // namespace ASIO_VERSION_NAMESPACE



} // namespace ASIO



#endif // ASIO_ASIOCONFIG_HPP
