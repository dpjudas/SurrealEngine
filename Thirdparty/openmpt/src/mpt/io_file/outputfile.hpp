/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_FILE_OUTPUTFILE_HPP
#define MPT_IO_FILE_OUTPUTFILE_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/io_file/fstream.hpp"
#include "mpt/path/os_path.hpp"
#include "mpt/string/types.hpp"

#include <exception>
#include <ios>
#include <vector>

#if MPT_LIBCXX_MS
#include <cstdio>
#endif // MPT_LIBCXX_MS

#if MPT_LIBCXX_MS
#include <tchar.h>
#endif // MPT_LIBCXX_MS



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



enum class FlushMode {
	None = 0,   // no explicit flushes at all
	Single = 1, // explicitly flush higher-leverl API layers
	Full = 2,   // explicitly flush *all* layers, up to and including disk write caches
};

inline FlushMode FlushModeFromBool(bool flush) {
	return flush ? FlushMode::Full : FlushMode::None;
}

class SafeOutputFile {

private:
	FlushMode m_FlushMode;
#if MPT_LIBCXX_MS
	std::FILE * m_f = nullptr;
#else  // !MPT_LIBCXX_MS
	mpt::IO::ofstream m_s;
#endif // MPT_LIBCXX_MS
#if MPT_LIBCXX_MS
	class FILEostream
		: public mpt::IO::ofstream {
	public:
		FILEostream(std::FILE * file)
			: mpt::IO::ofstream(file) {
			return;
		}
	};

	FILEostream m_s;

	static mpt::tstring convert_mode(std::ios_base::openmode mode, FlushMode flushMode) {
		mpt::tstring fopen_mode;
		switch (mode & ~(std::ios_base::ate | std::ios_base::binary)) {
			case std::ios_base::in:
				fopen_mode = _T("r");
				break;
			case std::ios_base::out:
				[[fallthrough]];
			case std::ios_base::out | std::ios_base::trunc:
				fopen_mode = _T("w");
				break;
			case std::ios_base::app:
				[[fallthrough]];
			case std::ios_base::out | std::ios_base::app:
				fopen_mode = _T("a");
				break;
			case std::ios_base::out | std::ios_base::in:
				fopen_mode = _T("r+");
				break;
			case std::ios_base::out | std::ios_base::in | std::ios_base::trunc:
				fopen_mode = _T("w+");
				break;
			case std::ios_base::out | std::ios_base::in | std::ios_base::app:
				[[fallthrough]];
			case std::ios_base::in | std::ios_base::app:
				fopen_mode = _T("a+");
				break;
		}
		if (fopen_mode.empty()) {
			return fopen_mode;
		}
		if (mode & std::ios_base::binary) {
			fopen_mode += _T("b");
		}
		if (flushMode == FlushMode::Full) {
			fopen_mode += _T("c"); // force commit on fflush (MSVC specific)
		}
		return fopen_mode;
	}

	std::FILE * internal_fopen(const mpt::os_path & filename, std::ios_base::openmode mode, FlushMode flushMode) {
		m_f = nullptr;
		mpt::tstring fopen_mode = convert_mode(mode, flushMode);
		if (fopen_mode.empty()) {
			return nullptr;
		}
		std::FILE * f =
#ifdef UNICODE
			_wfopen(mpt::support_long_path(mpt::transcode<mpt::tstring>(filename)).c_str(), fopen_mode.c_str())
#else
			std::fopen(mpt::support_long_path(mpt::transcode<mpt::tstring>(filename)).c_str(), fopen_mode.c_str())
#endif
			;
		if (!f) {
			return nullptr;
		}
		if (mode & std::ios_base::ate) {
			if (std::fseek(f, 0, SEEK_END) != 0) {
				std::fclose(f);
				f = nullptr;
				return nullptr;
			}
		}
		m_f = f;
		return f;
	}

#endif // MPT_LIBCXX_MS

public:
	SafeOutputFile() = delete;

	explicit SafeOutputFile(const mpt::os_path & filename, std::ios_base::openmode mode = std::ios_base::out, FlushMode flushMode = FlushMode::Full)
		: m_FlushMode(flushMode)
#if MPT_LIBCXX_MS
		, m_s(internal_fopen(filename, mode | std::ios_base::out, flushMode))
#else  // !MPT_LIBCXX_MS
		, m_s(filename, mode)
#endif // MPT_LIBCXX_MS
	{
		if (!stream().is_open()) {
			stream().setstate(mpt::IO::ofstream::failbit);
		}
	}

	mpt::IO::ofstream & stream() {
		return m_s;
	}

	operator mpt::IO::ofstream &() {
		return stream();
	}

	const mpt::IO::ofstream & stream() const {
		return m_s;
	}

	operator const mpt::IO::ofstream &() const {
		return stream();
	}

	operator bool() const {
		return stream() ? true : false;
	}

	bool operator!() const {
		return stream().operator!();
	}

	// cppcheck-suppress exceptThrowInDestructor
	~SafeOutputFile() noexcept(false) {
		const bool mayThrow = (std::uncaught_exceptions() == 0);
		if (!stream()) {
#if MPT_LIBCXX_MS
			if (m_f) {
				std::fclose(m_f);
			}
#endif // MPT_LIBCXX_MS
			if (mayThrow && (stream().exceptions() & (std::ios::badbit | std::ios::failbit))) {
				// cppcheck-suppress exceptThrowInDestructor
				throw std::ios_base::failure(std::string("Error before flushing file buffers."));
			}
			return;
		}
		if (!stream().rdbuf()) {
#if MPT_LIBCXX_MS
			if (m_f) {
				std::fclose(m_f);
			}
#endif // MPT_LIBCXX_MS
			if (mayThrow && (stream().exceptions() & (std::ios::badbit | std::ios::failbit))) {
				// cppcheck-suppress exceptThrowInDestructor
				throw std::ios_base::failure(std::string("Error before flushing file buffers."));
			}
			return;
		}
#if MPT_LIBCXX_MS
		if (!m_f) {
			return;
		}
#endif // MPT_LIBCXX_MS
		bool errorOnFlush = false;
		if (m_FlushMode != FlushMode::None) {
			try {
				if (stream().rdbuf()->pubsync() != 0) {
					errorOnFlush = true;
				}
			} catch (const std::exception &) {
				errorOnFlush = true;
#if MPT_LIBCXX_MS
				if (m_FlushMode != FlushMode::None) {
					if (std::fflush(m_f) != 0) {
						errorOnFlush = true;
					}
				}
				if (std::fclose(m_f) != 0) {
					errorOnFlush = true;
				}
#endif // MPT_LIBCXX_MS
				if (mayThrow) {
					// ignore errorOnFlush here, and re-throw the earlier exception
					// cppcheck-suppress exceptThrowInDestructor
					throw;
				}
			}
		}
#if MPT_LIBCXX_MS
		if (m_FlushMode != FlushMode::None) {
			if (std::fflush(m_f) != 0) {
				errorOnFlush = true;
			}
		}
		if (std::fclose(m_f) != 0) {
			errorOnFlush = true;
		}
#endif // MPT_LIBCXX_MS
		if (mayThrow && errorOnFlush && (stream().exceptions() & (std::ios::badbit | std::ios::failbit))) {
			// cppcheck-suppress exceptThrowInDestructor
			throw std::ios_base::failure(std::string("Error flushing file buffers."));
		}
	}
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_FILE_OUTPUTFILE_HPP
