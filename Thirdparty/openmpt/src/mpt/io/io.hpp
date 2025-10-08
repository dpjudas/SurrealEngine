/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_IO_HPP
#define MPT_IO_IO_HPP



#include "mpt/base/array.hpp"
#include "mpt/base/bit.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/span.hpp"
#include "mpt/endian/integer.hpp"
#include "mpt/endian/type_traits.hpp"
#include "mpt/io/base.hpp"

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

#include <cassert>
#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



template <typename Tbyte, typename Tfile>
inline mpt::byte_span ReadRaw(Tfile & f, Tbyte * data, std::size_t size) {
	return mpt::IO::ReadRawImpl(f, mpt::as_span(mpt::byte_cast<std::byte *>(data), size));
}

template <typename Tbyte, typename Tfile>
inline mpt::byte_span ReadRaw(Tfile & f, mpt::span<Tbyte> data) {
	return mpt::IO::ReadRawImpl(f, mpt::byte_cast<mpt::byte_span>(data));
}

template <typename Tbyte, typename Tfile>
inline bool WriteRaw(Tfile & f, const Tbyte * data, std::size_t size) {
	return mpt::IO::WriteRawImpl(f, mpt::as_span(mpt::byte_cast<const std::byte *>(data), size));
}

template <typename Tbyte, typename Tfile>
inline bool WriteRaw(Tfile & f, mpt::span<Tbyte> data) {
	return mpt::IO::WriteRawImpl(f, mpt::byte_cast<mpt::const_byte_span>(data));
}

template <typename Tbinary, typename Tfile>
inline bool Read(Tfile & f, Tbinary & v) {
	return mpt::IO::ReadRaw(f, mpt::as_raw_memory(v)).size() == mpt::as_raw_memory(v).size();
}

template <typename Tbinary, typename Tfile>
inline bool Write(Tfile & f, const Tbinary & v) {
	return mpt::IO::WriteRaw(f, mpt::as_raw_memory(v));
}

template <typename Tbinary, typename Tfile>
inline bool Write(Tfile & f, const std::vector<Tbinary> & v) {
	static_assert(mpt::is_binary_safe<Tbinary>::value);
	return mpt::IO::WriteRaw(f, mpt::as_raw_memory(v));
}

template <typename T, typename Tfile>
inline bool WritePartial(Tfile & f, const T & v, std::size_t size = sizeof(T)) {
	assert(size <= sizeof(T));
	return mpt::IO::WriteRaw(f, mpt::as_span(mpt::as_raw_memory(v).data(), size));
}

template <typename Tfile>
inline bool ReadByte(Tfile & f, std::byte & v) {
	bool result = false;
	std::byte byte = mpt::as_byte(0);
	const std::size_t readResult = mpt::IO::ReadRaw(f, &byte, sizeof(std::byte)).size();
	result = (readResult == sizeof(std::byte));
	v = byte;
	return result;
}

template <typename T, typename Tfile>
inline bool ReadBinaryTruncatedLE(Tfile & f, T & v, std::size_t size) {
	bool result = false;
	static_assert(std::numeric_limits<T>::is_integer);
	std::array<uint8, sizeof(T)> bytes = mpt::init_array<uint8, sizeof(T)>(uint8{0});
	const std::size_t readResult = mpt::IO::ReadRaw(f, bytes.data(), std::min(size, sizeof(T))).size();
	result = (readResult == std::min(size, sizeof(T)));
	v = mpt::bit_cast<typename mpt::make_le<T>::type>(bytes);
	return result;
}

template <typename T, typename Tfile>
inline bool ReadIntLE(Tfile & f, T & v) {
	bool result = false;
	static_assert(std::numeric_limits<T>::is_integer);
	std::array<uint8, sizeof(T)> bytes = mpt::init_array<uint8, sizeof(T)>(uint8{0});
	const std::size_t readResult = mpt::IO::ReadRaw(f, mpt::as_span(bytes)).size();
	result = (readResult == sizeof(T));
	v = mpt::bit_cast<typename mpt::make_le<T>::type>(bytes);
	return result;
}

template <typename T, typename Tfile>
inline bool ReadIntBE(Tfile & f, T & v) {
	bool result = false;
	static_assert(std::numeric_limits<T>::is_integer);
	std::array<uint8, sizeof(T)> bytes = mpt::init_array<uint8, sizeof(T)>(uint8{0});
	const std::size_t readResult = mpt::IO::ReadRaw(f, mpt::as_span(bytes)).size();
	result = (readResult == sizeof(T));
	v = mpt::bit_cast<typename mpt::make_be<T>::type>(bytes);
	return result;
}

template <typename Tfile>
inline bool ReadAdaptiveInt16LE(Tfile & f, uint16 & v) {
	bool result = true;
	uint8 byte = 0;
	std::size_t additionalBytes = 0;
	v = 0;
	byte = 0;
	if (!mpt::IO::ReadIntLE<uint8>(f, byte)) {
		result = false;
	}
	additionalBytes = (byte & 0x01);
	v = byte >> 1;
	for (std::size_t i = 0; i < additionalBytes; ++i) {
		byte = 0;
		if (!mpt::IO::ReadIntLE<uint8>(f, byte)) {
			result = false;
		}
		v |= (static_cast<uint16>(byte) << (((i + 1) * 8) - 1));
	}
	return result;
}

template <typename Tfile>
inline bool ReadAdaptiveInt32LE(Tfile & f, uint32 & v) {
	bool result = true;
	uint8 byte = 0;
	std::size_t additionalBytes = 0;
	v = 0;
	byte = 0;
	if (!mpt::IO::ReadIntLE<uint8>(f, byte)) {
		result = false;
	}
	additionalBytes = (byte & 0x03);
	v = byte >> 2;
	for (std::size_t i = 0; i < additionalBytes; ++i) {
		byte = 0;
		if (!mpt::IO::ReadIntLE<uint8>(f, byte)) {
			result = false;
		}
		v |= (static_cast<uint32>(byte) << (((i + 1) * 8) - 2));
	}
	return result;
}

template <typename Tfile>
inline bool ReadAdaptiveInt64LE(Tfile & f, uint64 & v) {
	bool result = true;
	uint8 byte = 0;
	std::size_t additionalBytes = 0;
	v = 0;
	byte = 0;
	if (!mpt::IO::ReadIntLE<uint8>(f, byte)) {
		result = false;
	}
	additionalBytes = (1 << (byte & 0x03)) - 1;
	v = byte >> 2;
	for (std::size_t i = 0; i < additionalBytes; ++i) {
		byte = 0;
		if (!mpt::IO::ReadIntLE<uint8>(f, byte)) {
			result = false;
		}
		v |= (static_cast<uint64>(byte) << (((i + 1) * 8) - 2));
	}
	return result;
}

template <typename Tsize, typename Tfile>
inline bool ReadSizedStringLE(Tfile & f, std::string & str, Tsize maxSize = std::numeric_limits<Tsize>::max()) {
	static_assert(std::numeric_limits<Tsize>::is_integer);
	str.clear();
	Tsize size = 0;
	if (!mpt::IO::ReadIntLE(f, size)) {
		return false;
	}
	if (size > maxSize) {
		return false;
	}
	for (Tsize i = 0; i != size; ++i) {
		char c = '\0';
		if (!mpt::IO::ReadIntLE(f, c)) {
			return false;
		}
		str.push_back(c);
	}
	return true;
}



template <typename T, typename Tfile>
inline bool WriteIntLE(Tfile & f, const T v) {
	static_assert(std::numeric_limits<T>::is_integer);
	return mpt::IO::Write(f, mpt::as_le(v));
}

template <typename T, typename Tfile>
inline bool WriteIntBE(Tfile & f, const T v) {
	static_assert(std::numeric_limits<T>::is_integer);
	return mpt::IO::Write(f, mpt::as_be(v));
}

template <typename Tfile>
inline bool WriteAdaptiveInt16LE(Tfile & f, const uint16 v, std::size_t fixedSize = 0) {
	std::size_t minSize = fixedSize;
	std::size_t maxSize = fixedSize;
	assert(minSize == 0 || minSize == 1 || minSize == 2);
	assert(maxSize == 0 || maxSize == 1 || maxSize == 2);
	assert(maxSize == 0 || maxSize >= minSize);
	if (maxSize == 0) {
		maxSize = 2;
	}
	if (v < 0x80 && minSize <= 1 && 1 <= maxSize) {
		return mpt::IO::WriteIntLE<uint8>(f, static_cast<uint8>(v << 1) | 0x00);
	} else if (v < 0x8000 && minSize <= 2 && 2 <= maxSize) {
		return mpt::IO::WriteIntLE<uint16>(f, static_cast<uint16>(v << 1) | 0x01);
	} else {
		assert(false);
		return false;
	}
}

template <typename Tfile>
inline bool WriteAdaptiveInt32LE(Tfile & f, const uint32 v, std::size_t fixedSize = 0) {
	std::size_t minSize = fixedSize;
	std::size_t maxSize = fixedSize;
	assert(minSize == 0 || minSize == 1 || minSize == 2 || minSize == 3 || minSize == 4);
	assert(maxSize == 0 || maxSize == 1 || maxSize == 2 || maxSize == 3 || maxSize == 4);
	assert(maxSize == 0 || maxSize >= minSize);
	if (maxSize == 0) {
		maxSize = 4;
	}
	if (v < 0x40 && minSize <= 1 && 1 <= maxSize) {
		return mpt::IO::WriteIntLE<uint8>(f, static_cast<uint8>(v << 2) | 0x00);
	} else if (v < 0x4000 && minSize <= 2 && 2 <= maxSize) {
		return mpt::IO::WriteIntLE<uint16>(f, static_cast<uint16>(v << 2) | 0x01);
	} else if (v < 0x400000 && minSize <= 3 && 3 <= maxSize) {
		uint32 value = static_cast<uint32>(v << 2) | 0x02;
		std::byte bytes[3];
		bytes[0] = static_cast<std::byte>(value >> 0);
		bytes[1] = static_cast<std::byte>(value >> 8);
		bytes[2] = static_cast<std::byte>(value >> 16);
		return mpt::IO::WriteRaw(f, bytes, 3);
	} else if (v < 0x40000000 && minSize <= 4 && 4 <= maxSize) {
		return mpt::IO::WriteIntLE<uint32>(f, static_cast<uint32>(v << 2) | 0x03);
	} else {
		assert(false);
		return false;
	}
}

template <typename Tfile>
inline bool WriteAdaptiveInt64LE(Tfile & f, const uint64 v, std::size_t fixedSize = 0) {
	std::size_t minSize = fixedSize;
	std::size_t maxSize = fixedSize;
	assert(minSize == 0 || minSize == 1 || minSize == 2 || minSize == 4 || minSize == 8);
	assert(maxSize == 0 || maxSize == 1 || maxSize == 2 || maxSize == 4 || maxSize == 8);
	assert(maxSize == 0 || maxSize >= minSize);
	if (maxSize == 0) {
		maxSize = 8;
	}
	if (v < 0x40 && minSize <= 1 && 1 <= maxSize) {
		return mpt::IO::WriteIntLE<uint8>(f, static_cast<uint8>(v << 2) | 0x00);
	} else if (v < 0x4000 && minSize <= 2 && 2 <= maxSize) {
		return mpt::IO::WriteIntLE<uint16>(f, static_cast<uint16>(v << 2) | 0x01);
	} else if (v < 0x40000000 && minSize <= 4 && 4 <= maxSize) {
		return mpt::IO::WriteIntLE<uint32>(f, static_cast<uint32>(v << 2) | 0x02);
	} else if (v < 0x4000000000000000ull && minSize <= 8 && 8 <= maxSize) {
		return mpt::IO::WriteIntLE<uint64>(f, static_cast<uint64>(v << 2) | 0x03);
	} else {
		assert(false);
		return false;
	}
}

// Write a variable-length integer, as found in MIDI files. The number of written bytes is placed in the bytesWritten parameter.
template <typename Tfile, typename T>
bool WriteVarInt(Tfile & f, const T v, std::size_t * bytesWritten = nullptr) {
	static_assert(std::numeric_limits<T>::is_integer);
	static_assert(!std::numeric_limits<T>::is_signed);
	std::byte out[(sizeof(T) * 8 + 6) / 7];
	std::size_t numBytes = 0;
	for (uint32 n = (sizeof(T) * 8) / 7; n > 0; n--) {
		if (v >= (static_cast<T>(1) << (n * 7u))) {
			out[numBytes++] = static_cast<std::byte>(((v >> (n * 7u)) & 0x7F) | 0x80);
		}
	}
	out[numBytes++] = static_cast<std::byte>(v & 0x7F);
	assert(numBytes <= std::size(out));
	if (bytesWritten != nullptr) {
		*bytesWritten = numBytes;
	}
	return mpt::IO::WriteRaw(f, out, numBytes);
}

template <typename Tsize, typename Tfile>
inline bool WriteSizedStringLE(Tfile & f, const std::string & str) {
	static_assert(std::numeric_limits<Tsize>::is_integer);
	if (str.size() > std::numeric_limits<Tsize>::max()) {
		return false;
	}
	Tsize size = static_cast<Tsize>(str.size());
	if (!mpt::IO::WriteIntLE(f, size)) {
		return false;
	}
	if (!mpt::IO::WriteRaw(f, str.data(), str.size())) {
		return false;
	}
	return true;
}

template <typename Tfile>
inline bool WriteText(Tfile & f, const std::string & s) {
	return mpt::IO::WriteRaw(f, s.data(), s.size());
}

template <typename Tfile>
inline bool WriteTextCRLF(Tfile & f) {
	return mpt::IO::WriteText(f, "\r\n");
}

template <typename Tfile>
inline bool WriteTextLF(Tfile & f) {
	return mpt::IO::WriteText(f, "\n");
}

template <typename Tfile>
inline bool WriteTextCRLF(Tfile & f, const std::string & s) {
	return mpt::IO::WriteText(f, s) && mpt::IO::WriteTextCRLF(f);
}

template <typename Tfile>
inline bool WriteTextLF(Tfile & f, const std::string & s) {
	return mpt::IO::WriteText(f, s) && mpt::IO::WriteTextLF(f);
}



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_IO_HPP
