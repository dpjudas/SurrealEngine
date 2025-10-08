/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_SOURCE_LOCATION_HPP
#define MPT_BASE_SOURCE_LOCATION_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/namespace.hpp"

#if MPT_CXX_AT_LEAST(20) && !MPT_MSVC_BEFORE(2022, 0) && !MPT_COMPILER_CLANG
#include <source_location>
#endif // C++20



namespace mpt {
inline namespace MPT_INLINE_NS {


#if MPT_CXX_AT_LEAST(20) && !MPT_MSVC_BEFORE(2022, 0) && !MPT_COMPILER_CLANG

using std::source_location;

#define MPT_SOURCE_LOCATION_CURRENT() std::source_location::current()

#else // !C++20

#if MPT_COMPILER_MSVC && MPT_MSVC_AT_LEAST(2019, 6)

#define MPT_SOURCE_LOCATION_FILE     __builtin_FILE()
#define MPT_SOURCE_LOCATION_FUNCTION __builtin_FUNCTION()
#define MPT_SOURCE_LOCATION_LINE     __builtin_LINE()
#define MPT_SOURCE_LOCATION_COLUMN   __builtin_COLUMN()

#elif MPT_COMPILER_GCC

#define MPT_SOURCE_LOCATION_FILE     __builtin_FILE()
#define MPT_SOURCE_LOCATION_FUNCTION __builtin_FUNCTION()
#define MPT_SOURCE_LOCATION_LINE     __builtin_LINE()
#define MPT_SOURCE_LOCATION_COLUMN   0

#elif MPT_COMPILER_CLANG && ((!MPT_OS_MACOSX_OR_IOS && MPT_CLANG_AT_LEAST(9, 0, 0)) || (MPT_OS_MACOSX_OR_IOS && MPT_CLANG_AT_LEAST(12, 0, 0)))

// We do not know which Apple Clang version introduced __builtin_FILE().
// It fails with 10.x (see <https://github.com/fink/fink-distributions/issues/1202>),
// and IRC dicussion decided on 12.x as a somewhat safe choice.

#define MPT_SOURCE_LOCATION_FILE     __builtin_FILE()
#define MPT_SOURCE_LOCATION_FUNCTION __builtin_FUNCTION()
#define MPT_SOURCE_LOCATION_LINE     __builtin_LINE()
#define MPT_SOURCE_LOCATION_COLUMN   __builtin_COLUMN()

#else

#define MPT_SOURCE_LOCATION_FILE     __FILE__
#define MPT_SOURCE_LOCATION_FUNCTION ""
#define MPT_SOURCE_LOCATION_LINE     __LINE__
#define MPT_SOURCE_LOCATION_COLUMN   0

#endif

// compatible with C++20 std::source_location
struct source_location {
private:
	const char * m_file_name;
	const char * m_function_name;
	uint32 m_line;
	uint32 m_column;

public:
	constexpr source_location() noexcept
		: m_file_name("")
		, m_function_name("")
		, m_line(0)
		, m_column(0) {
	}
	constexpr source_location(const char * file, const char * function, uint32 line, uint32 column) noexcept
		: m_file_name(file)
		, m_function_name(function)
		, m_line(line)
		, m_column(column) {
	}
	source_location(const source_location &) = default;
	source_location(source_location &&) = default;
	static constexpr source_location current(const char * file = MPT_SOURCE_LOCATION_FILE, const char * function = MPT_SOURCE_LOCATION_FUNCTION, uint32 line = MPT_SOURCE_LOCATION_LINE, uint32 column = MPT_SOURCE_LOCATION_COLUMN) noexcept {
		return source_location(file, function, line, column);
	}
	constexpr uint32 line() const noexcept {
		return m_line;
	}
	constexpr uint32 column() const noexcept {
		return m_column;
	}
	constexpr const char * file_name() const noexcept {
		return m_file_name;
	}
	constexpr const char * function_name() const noexcept {
		return m_function_name;
	}
};


#if (MPT_COMPILER_MSVC && MPT_MSVC_AT_LEAST(2019, 6)) || MPT_COMPILER_GCC || (MPT_COMPILER_CLANG && MPT_CLANG_AT_LEAST(9, 0, 0))
#define MPT_SOURCE_LOCATION_CURRENT() mpt::source_location::current()
#else
#define MPT_SOURCE_LOCATION_CURRENT() mpt::source_location::current(__FILE__, __func__, __LINE__, 0)
#endif

#endif // C++20


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_SOURCE_LOCATION_HPP
