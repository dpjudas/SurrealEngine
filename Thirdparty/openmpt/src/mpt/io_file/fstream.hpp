/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_FILE_FSTREAM_HPP
#define MPT_IO_FILE_FSTREAM_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/span.hpp"
#include "mpt/io/base.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/path/os_path.hpp"
#include "mpt/path/os_path_long.hpp"
#include "mpt/string_transcode/transcode.hpp"

#if defined(MPT_COMPILER_QUIRK_WINDOWS_FSTREAM_NO_WCHAR)
#if MPT_GCC_AT_LEAST(9, 1, 0) && !defined(MPT_COMPILER_QUIRK_NO_FILESYSTEM)
#include <filesystem>
#endif // MPT_GCC_AT_LEAST(9,1,0) && !defined(MPT_COMPILER_QUIRK_NO_FILESYSTEM)
#endif // MPT_COMPILER_QUIRK_WINDOWS_FSTREAM_NO_WCHAR
#include <fstream>
#include <ios>
#include <string>
#include <string_view>

#if MPT_LIBCXX_MS
#include <cstdio>
#endif // MPT_LIBCXX_MS



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



namespace detail {

template <typename Tbase>
inline void fstream_open(Tbase & base, const mpt::os_path & filename, std::ios_base::openmode mode) {
#if defined(MPT_COMPILER_QUIRK_WINDOWS_FSTREAM_NO_WCHAR)
#if MPT_GCC_AT_LEAST(9, 1, 0) && !defined(MPT_COMPILER_QUIRK_NO_FILESYSTEM)
	base.open(static_cast<std::filesystem::path>(mpt::support_long_path(filename)), mode);
#else  // !MPT_GCC_AT_LEAST(9,1,0) || MPT_COMPILER_QUIRK_NO_FILESYSTEM
	// Warning: MinGW with GCC earlier than 9.1 detected. Standard library does neither provide std::fstream wchar_t overloads nor std::filesystem with wchar_t support. Unicode filename support is thus unavailable.
	base.open(mpt::transcode<std::string>(mpt::logical_encoding::locale, mpt::support_long_path(filename)).c_str(), mode);
#endif // MPT_GCC_AT_LEAST(9,1,0) && !MPT_COMPILER_QUIRK_NO_FILESYSTEM
#else  // !MPT_COMPILER_QUIRK_WINDOWS_FSTREAM_NO_WCHAR
	base.open(mpt::support_long_path(filename).c_str(), mode);
#endif // MPT_COMPILER_QUIRK_WINDOWS_FSTREAM_NO_WCHAR
}

} // namespace detail

// We cannot rely on implicit conversion of mpt::path to std::filesystem::path when constructing std::fstream
// because of broken overload implementation in GCC libstdc++ 8, 9, 10.
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=95642
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=90704

class fstream
	: public std::fstream {
private:
	using base_type = std::fstream;

public:
	explicit fstream(const mpt::os_path & filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) {
		detail::fstream_open<base_type>(*this, filename, mode);
	}
#if MPT_LIBCXX_MS
protected:
	fstream(std::FILE * file)
		: std::fstream(file) {
	}
#endif // MPT_LIBCXX_MS
public:
	void open(const char * filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) = delete;
	void open(const std::string_view & filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) = delete;
	void open(const std::string & filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) = delete;
#if MPT_OS_WINDOWS && !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
	void open(const wchar_t * filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) = delete;
	void open(const std::wstring_view & filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) = delete;
	void open(const std::wstring & filename, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) = delete;
#endif
};

class ifstream
	: public std::ifstream {
private:
	using base_type = std::ifstream;

public:
	explicit ifstream(const mpt::os_path & filename, std::ios_base::openmode mode = std::ios_base::in) {
		detail::fstream_open<base_type>(*this, filename, mode);
	}
#if MPT_LIBCXX_MS
protected:
	ifstream(std::FILE * file)
		: std::ifstream(file) {
	}
#endif // MPT_LIBCXX_MS
public:
	void open(const char * filename, std::ios_base::openmode mode = std::ios_base::in) = delete;
	void open(const std::string_view & filename, std::ios_base::openmode mode = std::ios_base::in) = delete;
	void open(const std::string & filename, std::ios_base::openmode mode = std::ios_base::in) = delete;
#if MPT_OS_WINDOWS && !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
	void open(const wchar_t * filename, std::ios_base::openmode mode = std::ios_base::in) = delete;
	void open(const std::wstring_view & filename, std::ios_base::openmode mode = std::ios_base::in) = delete;
	void open(const std::wstring & filename, std::ios_base::openmode mode = std::ios_base::in) = delete;
#endif
};

class ofstream
	: public std::ofstream {
private:
	using base_type = std::ofstream;

public:
	explicit ofstream(const mpt::os_path & filename, std::ios_base::openmode mode = std::ios_base::out) {
		detail::fstream_open<base_type>(*this, filename, mode);
	}
#if MPT_LIBCXX_MS
protected:
	ofstream(std::FILE * file)
		: std::ofstream(file) {
	}
#endif // MPT_LIBCXX_MS
public:
	void open(const char * filename, std::ios_base::openmode mode = std::ios_base::out) = delete;
	void open(const std::string_view & filename, std::ios_base::openmode mode = std::ios_base::out) = delete;
	void open(const std::string & filename, std::ios_base::openmode mode = std::ios_base::out) = delete;
#if MPT_OS_WINDOWS && !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
	void open(const wchar_t * filename, std::ios_base::openmode mode = std::ios_base::out) = delete;
	void open(const std::wstring_view & filename, std::ios_base::openmode mode = std::ios_base::out) = delete;
	void open(const std::wstring & filename, std::ios_base::openmode mode = std::ios_base::out) = delete;
#endif
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_FILE_FSTREAM_HPP
