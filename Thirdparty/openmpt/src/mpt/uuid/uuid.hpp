/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_UUID_UUID_HPP
#define MPT_UUID_UUID_HPP



#include "mpt/base/constexpr_throw.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/endian/integer.hpp"
#include "mpt/format/default_formatter.hpp"
#include "mpt/format/simple.hpp"
#include "mpt/parse/parse.hpp"
#include "mpt/random/random.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string/utility.hpp"

#if MPT_OS_WINDOWS
#if MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)
#include <guiddef.h>
#endif // MPT_WIN_VISTA
#include <objbase.h>
#include <rpc.h>
#endif // MPT_OS_WINDOWS



namespace mpt {
inline namespace MPT_INLINE_NS {


// Microsoft on-disk layout
struct GUIDms {
	uint32le Data1;
	uint16le Data2;
	uint16le Data3;
	uint64be Data4; // yes, big endian here
};
constexpr bool declare_binary_safe(const GUIDms &) {
	return true;
}
static_assert(mpt::check_binary_size<GUIDms>(16));

// RFC binary format
struct UUIDbin {
	uint32be Data1;
	uint16be Data2;
	uint16be Data3;
	uint64be Data4;
};
constexpr bool declare_binary_safe(const UUIDbin &) {
	return true;
}
static_assert(mpt::check_binary_size<UUIDbin>(16));



struct UUID {
private:
	uint32 Data1;
	uint16 Data2;
	uint16 Data3;
	uint64 Data4;

public:
	MPT_CONSTEXPRINLINE uint32 GetData1() const noexcept {
		return Data1;
	}
	MPT_CONSTEXPRINLINE uint16 GetData2() const noexcept {
		return Data2;
	}
	MPT_CONSTEXPRINLINE uint16 GetData3() const noexcept {
		return Data3;
	}
	MPT_CONSTEXPRINLINE uint64 GetData4() const noexcept {
		return Data4;
	}

public:
	MPT_CONSTEXPRINLINE uint64 GetData64_1() const noexcept {
		return (static_cast<uint64>(Data1) << 32) | (static_cast<uint64>(Data2) << 16) | (static_cast<uint64>(Data3) << 0);
	}
	MPT_CONSTEXPRINLINE uint64 GetData64_2() const noexcept {
		return Data4;
	}

public:
	// xxxxxxxx-xxxx-Mmxx-Nnxx-xxxxxxxxxxxx
	// <--32-->-<16>-<16>-<-------64------>
	MPT_CONSTEXPRINLINE bool IsNil() const noexcept {
		return (Data1 == 0) && (Data2 == 0) && (Data3 == 0) && (Data4 == 0);
	}
	MPT_CONSTEXPRINLINE bool IsValid() const noexcept {
		return (Data1 != 0) || (Data2 != 0) || (Data3 != 0) || (Data4 != 0);
	}
	MPT_CONSTEXPRINLINE uint8 Variant() const noexcept {
		return Nn() >> 4u;
	}
	MPT_CONSTEXPRINLINE uint8 Version() const noexcept {
		return Mm() >> 4u;
	}
	MPT_CONSTEXPRINLINE bool IsRFC4122() const noexcept {
		return (Variant() & 0xcu) == 0x8u;
	}

private:
	MPT_CONSTEXPRINLINE uint8 Mm() const noexcept {
		return static_cast<uint8>((Data3 >> 8) & 0xffu);
	}
	MPT_CONSTEXPRINLINE uint8 Nn() const noexcept {
		return static_cast<uint8>((Data4 >> 56) & 0xffu);
	}
#if MPT_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif // MPT_COMPILER_GCC
	void MakeRFC4122(uint8 version) noexcept {
		// variant
		uint8 Nn = static_cast<uint8>((Data4 >> 56) & 0xffu);
		Data4 &= 0x00ffffffffffffffull;
		Nn &= ~(0xc0u);
		Nn |= 0x80u;
		Data4 |= static_cast<uint64>(Nn) << 56;
		// version
		version &= 0x0fu;
		uint8 Mm = static_cast<uint8>((Data3 >> 8) & 0xffu);
		Data3 &= 0x00ffu;
		Mm &= ~(0xf0u);
		Mm |= (version << 4u);
		Data3 |= static_cast<uint16>(Mm) << 8;
	}
#if MPT_COMPILER_GCC
#pragma GCC diagnostic pop
#endif // MPT_COMPILER_GCC
#if MPT_OS_WINDOWS
private:
	static mpt::UUID UUIDFromWin32(::UUID uuid) {
		return mpt::UUID(uuid.Data1, uuid.Data2, uuid.Data3, (static_cast<uint64>(0) | (static_cast<uint64>(uuid.Data4[0]) << 56) | (static_cast<uint64>(uuid.Data4[1]) << 48) | (static_cast<uint64>(uuid.Data4[2]) << 40) | (static_cast<uint64>(uuid.Data4[3]) << 32) | (static_cast<uint64>(uuid.Data4[4]) << 24) | (static_cast<uint64>(uuid.Data4[5]) << 16) | (static_cast<uint64>(uuid.Data4[6]) << 8) | (static_cast<uint64>(uuid.Data4[7]) << 0)));
	}
	static ::UUID UUIDToWin32(mpt::UUID uuid) {
		::UUID result = ::UUID();
		result.Data1 = uuid.GetData1();
		result.Data2 = uuid.GetData2();
		result.Data3 = uuid.GetData3();
		result.Data4[0] = static_cast<uint8>(uuid.GetData4() >> 56);
		result.Data4[1] = static_cast<uint8>(uuid.GetData4() >> 48);
		result.Data4[2] = static_cast<uint8>(uuid.GetData4() >> 40);
		result.Data4[3] = static_cast<uint8>(uuid.GetData4() >> 32);
		result.Data4[4] = static_cast<uint8>(uuid.GetData4() >> 24);
		result.Data4[5] = static_cast<uint8>(uuid.GetData4() >> 16);
		result.Data4[6] = static_cast<uint8>(uuid.GetData4() >> 8);
		result.Data4[7] = static_cast<uint8>(uuid.GetData4() >> 0);
		return result;
	}

public:
	explicit UUID(::UUID uuid) {
		*this = UUIDFromWin32(uuid);
	}
	operator ::UUID() const {
		return UUIDToWin32(*this);
	}
#endif // MPT_OS_WINDOWS
private:
	static MPT_CONSTEXPRINLINE uint8 NibbleFromChar(char x) {
		return ('0' <= x && x <= '9') ? static_cast<uint8>(x - '0' + 0) : ('a' <= x && x <= 'z') ? static_cast<uint8>(x - 'a' + 10)
																	: ('A' <= x && x <= 'Z')     ? static_cast<uint8>(x - 'A' + 10)
																								 : mpt::constexpr_throw<uint8>(std::domain_error(""));
	}
	static MPT_CONSTEXPRINLINE uint8 ByteFromHex(char x, char y) {
		return static_cast<uint8>(uint8(0) | (NibbleFromChar(x) << 4) | (NibbleFromChar(y) << 0));
	}
	static MPT_CONSTEXPRINLINE uint16 ParseHex16(const char * str) {
		return static_cast<uint16>(uint16(0) | (static_cast<uint16>(ByteFromHex(str[0], str[1])) << 8) | (static_cast<uint16>(ByteFromHex(str[2], str[3])) << 0));
	}
	static MPT_CONSTEXPRINLINE uint32 ParseHex32(const char * str) {
		return static_cast<uint32>(uint32(0) | (static_cast<uint32>(ByteFromHex(str[0], str[1])) << 24) | (static_cast<uint32>(ByteFromHex(str[2], str[3])) << 16) | (static_cast<uint32>(ByteFromHex(str[4], str[5])) << 8) | (static_cast<uint32>(ByteFromHex(str[6], str[7])) << 0));
	}

public:
	static MPT_CONSTEXPRINLINE UUID ParseLiteral(const char * str, std::size_t len) {
		return (len == 36 && str[8] == '-' && str[13] == '-' && str[18] == '-' && str[23] == '-') ? mpt::UUID(
				   ParseHex32(str + 0),
				   ParseHex16(str + 9),
				   ParseHex16(str + 14),
				   uint64(0)
					   | (static_cast<uint64>(ParseHex16(str + 19)) << 48)
					   | (static_cast<uint64>(ParseHex16(str + 24)) << 32)
					   | (static_cast<uint64>(ParseHex32(str + 28)) << 0))
																								  : mpt::constexpr_throw<mpt::UUID>(std::domain_error(""));
	}

public:
	MPT_CONSTEXPRINLINE UUID() noexcept
		: Data1(0)
		, Data2(0)
		, Data3(0)
		, Data4(0) {
		return;
	}
	MPT_CONSTEXPRINLINE explicit UUID(uint32 Data1, uint16 Data2, uint16 Data3, uint64 Data4) noexcept
		: Data1(Data1)
		, Data2(Data2)
		, Data3(Data3)
		, Data4(Data4) {
		return;
	}
	explicit UUID(UUIDbin uuid) {
		Data1 = uuid.Data1.get();
		Data2 = uuid.Data2.get();
		Data3 = uuid.Data3.get();
		Data4 = uuid.Data4.get();
	}
	explicit UUID(GUIDms guid) {
		Data1 = guid.Data1.get();
		Data2 = guid.Data2.get();
		Data3 = guid.Data3.get();
		Data4 = guid.Data4.get();
	}
	operator UUIDbin() const {
		UUIDbin result{};
		result.Data1 = GetData1();
		result.Data2 = GetData2();
		result.Data3 = GetData3();
		result.Data4 = GetData4();
		return result;
	}
	operator GUIDms() const {
		GUIDms result{};
		result.Data1 = GetData1();
		result.Data2 = GetData2();
		result.Data3 = GetData3();
		result.Data4 = GetData4();
		return result;
	}

public:
	// Create a UUID
	template <typename Trng>
	static UUID Generate(Trng & rng) {
#if MPT_WINRT_AT_LEAST(MPT_WIN_8)
		::GUID guid = ::GUID();
		HRESULT result = CoCreateGuid(&guid);
		if (result != S_OK) {
			return mpt::UUID::RFC4122Random(rng);
		}
		return mpt::UUID::UUIDFromWin32(guid);
#elif MPT_WINRT_BEFORE(MPT_WIN_8)
		return mpt::UUID::RFC4122Random(rng);
#elif MPT_OS_WINDOWS
		::UUID uuid = ::UUID();
		RPC_STATUS status = ::UuidCreate(&uuid);
		if (status != RPC_S_OK && status != RPC_S_UUID_LOCAL_ONLY) {
			return mpt::UUID::RFC4122Random(rng);
		}
		status = RPC_S_OK;
		if (UuidIsNil(&uuid, &status) != FALSE) {
			return mpt::UUID::RFC4122Random(rng);
		}
		if (status != RPC_S_OK) {
			return mpt::UUID::RFC4122Random(rng);
		}
		return mpt::UUID::UUIDFromWin32(uuid);
#else
		return mpt::UUID::RFC4122Random(rng);
#endif
	}
	// Create a UUID that contains local, traceable information.
	// Safe for local use. May be faster.
	template <typename Trng>
	static UUID GenerateLocalUseOnly(Trng & rng) {
#if MPT_WINRT_AT_LEAST(MPT_WIN_8)
		::GUID guid = ::GUID();
		HRESULT result = CoCreateGuid(&guid);
		if (result != S_OK) {
			return mpt::UUID::RFC4122Random(rng);
		}
		return mpt::UUID::UUIDFromWin32(guid);
#elif MPT_WINRT_BEFORE(MPT_WIN_8)
		return mpt::UUID::RFC4122Random(rng);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_XP)
		// Available since Win2000, but we check for WinXP in order to not use this
		// function in Win32old builds. It is not available on some non-fully
		// patched Win98SE installs in the wild.
		::UUID uuid = ::UUID();
		RPC_STATUS status = ::UuidCreateSequential(&uuid);
		if (status != RPC_S_OK && status != RPC_S_UUID_LOCAL_ONLY) {
			return Generate(rng);
		}
		status = RPC_S_OK;
		if (UuidIsNil(&uuid, &status) != FALSE) {
			return mpt::UUID::RFC4122Random(rng);
		}
		if (status != RPC_S_OK) {
			return mpt::UUID::RFC4122Random(rng);
		}
		return mpt::UUID::UUIDFromWin32(uuid);
#elif MPT_OS_WINDOWS
		// Fallback to ::UuidCreate is safe as ::UuidCreateSequential is only a
		// tiny performance optimization.
		return Generate(rng);
#else
		return RFC4122Random(rng);
#endif
	}
	// Create a RFC4122 Random UUID.
	template <typename Trng>
	static UUID RFC4122Random(Trng & prng) {
		UUID result;
		result.Data1 = mpt::random<uint32>(prng);
		result.Data2 = mpt::random<uint16>(prng);
		result.Data3 = mpt::random<uint16>(prng);
		result.Data4 = mpt::random<uint64>(prng);
		result.MakeRFC4122(4);
		return result;
	}
	friend UUID UUIDRFC4122NamespaceV3(const UUID & ns, const mpt::ustring & name);
	friend UUID UUIDRFC4122NamespaceV5(const UUID & ns, const mpt::ustring & name);

public:
	// General UUID<->string conversion.
	// The string must/will be in standard UUID format: 4f9a455d-e7ef-4367-b2f0-0c83a38a5c72
	static UUID FromString(const mpt::ustring & str) {
		std::vector<mpt::ustring> segments = mpt::split<mpt::ustring>(str, MPT_ULITERAL("-"));
		if (segments.size() != 5) {
			return UUID();
		}
		if (segments[0].length() != 8) {
			return UUID();
		}
		if (segments[1].length() != 4) {
			return UUID();
		}
		if (segments[2].length() != 4) {
			return UUID();
		}
		if (segments[3].length() != 4) {
			return UUID();
		}
		if (segments[4].length() != 12) {
			return UUID();
		}
		UUID result;
		result.Data1 = mpt::parse_hex<uint32>(segments[0]);
		result.Data2 = mpt::parse_hex<uint16>(segments[1]);
		result.Data3 = mpt::parse_hex<uint16>(segments[2]);
		result.Data4 = mpt::parse_hex<uint64>(segments[3] + segments[4]);
		return result;
	}
	mpt::ustring ToUString() const {
		return mpt::ustring()
			 + mpt::format<mpt::ustring>::hex0<8>(GetData1())
			 + MPT_USTRING("-")
			 + mpt::format<mpt::ustring>::hex0<4>(GetData2())
			 + MPT_USTRING("-")
			 + mpt::format<mpt::ustring>::hex0<4>(GetData3())
			 + MPT_USTRING("-")
			 + mpt::format<mpt::ustring>::hex0<4>(static_cast<uint16>(GetData4() >> 48))
			 + MPT_USTRING("-")
			 + mpt::format<mpt::ustring>::hex0<4>(static_cast<uint16>(GetData4() >> 32))
			 + mpt::format<mpt::ustring>::hex0<8>(static_cast<uint32>(GetData4() >> 0));
	}
};

MPT_CONSTEXPRINLINE bool operator==(const mpt::UUID & a, const mpt::UUID & b) noexcept {
	return (a.GetData1() == b.GetData1()) && (a.GetData2() == b.GetData2()) && (a.GetData3() == b.GetData3()) && (a.GetData4() == b.GetData4());
}

MPT_CONSTEXPRINLINE bool operator!=(const mpt::UUID & a, const mpt::UUID & b) noexcept {
	return (a.GetData1() != b.GetData1()) || (a.GetData2() != b.GetData2()) || (a.GetData3() != b.GetData3()) || (a.GetData4() != b.GetData4());
}


namespace uuid_literals {

MPT_CONSTEVAL mpt::UUID operator""_uuid(const char * str, std::size_t len) {
	return mpt::UUID::ParseLiteral(str, len);
}

} // namespace uuid_literals




} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_UUID_UUID_HPP
