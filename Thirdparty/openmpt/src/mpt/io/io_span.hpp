/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_IO_SPAN_HPP
#define MPT_IO_IO_SPAN_HPP



#include "mpt/base/macros.hpp"
#include "mpt/base/memory.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/base/span.hpp"
#include "mpt/io/base.hpp"

#include <algorithm>
#include <utility>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



template <typename Tbyte>
struct FileOperations<std::pair<mpt::span<Tbyte>, IO::Offset>> {

private:
	std::pair<mpt::span<Tbyte>, IO::Offset> & f;

public:
	FileOperations(std::pair<mpt::span<Tbyte>, IO::Offset> & f_)
		: f(f_) {
		return;
	}

public:
	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsValid() {
		return (f.second >= 0);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsReadSeekable() {
		MPT_UNUSED(f);
		return true;
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsWriteSeekable() {
		MPT_UNUSED(f);
		return true;
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline IO::Offset TellRead() {
		return f.second;
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline IO::Offset TellWrite() {
		return f.second;
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekBegin() {
		f.second = 0;
		return true;
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekEnd() {
		f.second = f.first.size();
		return true;
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekAbsolute(IO::Offset pos) {
		f.second = pos;
		return true;
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool SeekRelative(IO::Offset off) {
		if (f.second < 0) {
			return false;
		}
		f.second += off;
		return true;
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline mpt::byte_span ReadRawImpl(mpt::byte_span data) {
		if (f.second < 0) {
			return data.first(0);
		}
		if (f.second >= static_cast<IO::Offset>(f.first.size())) {
			return data.first(0);
		}
		std::size_t num = mpt::saturate_cast<std::size_t>(std::min(static_cast<IO::Offset>(f.first.size()) - f.second, static_cast<IO::Offset>(data.size())));
		std::copy(mpt::byte_cast<const std::byte *>(f.first.data() + f.second), mpt::byte_cast<const std::byte *>(f.first.data() + f.second + num), data.data());
		f.second += num;
		return data.first(num);
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool WriteRawImpl(mpt::const_byte_span data) {
		if (f.second < 0) {
			return false;
		}
		if (f.second > static_cast<IO::Offset>(f.first.size())) {
			return false;
		}
		std::size_t num = mpt::saturate_cast<std::size_t>(std::min(static_cast<IO::Offset>(f.first.size()) - f.second, static_cast<IO::Offset>(data.size())));
		if (num != data.size()) {
			return false;
		}
		std::copy(data.data(), data.data() + num, mpt::byte_cast<std::byte *>(f.first.data() + f.second));
		f.second += num;
		return true;
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool IsEof() {
		return (f.second >= static_cast<IO::Offset>(f.first.size()));
	}

	// cppcheck false-positive
	// cppcheck-suppress duplInheritedMember
	inline bool Flush() {
		MPT_UNUSED(f);
		return true;
	}
};




} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_IO_SPAN_HPP
