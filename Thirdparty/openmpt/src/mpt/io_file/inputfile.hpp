/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_FILE_INPUTFILE_HPP
#define MPT_IO_FILE_INPUTFILE_HPP



#include "mpt/base/alloc.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/base/span.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/io_file/fstream.hpp"
#include "mpt/path/os_path.hpp"

#include <ios>
#include <istream>
#include <variant>
#include <vector>

#include <cassert>
#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



class UncachedInputFile {

private:
	mpt::os_path m_Filename;
	mpt::IO::ifstream m_File;
	bool m_IsValid;

public:
	UncachedInputFile(const mpt::os_path & filename)
		: m_Filename(filename)
		, m_File(m_Filename, std::ios::binary | std::ios::in)
		, m_IsValid(true) {
		assert(!m_Filename.empty());
	}

	~UncachedInputFile() = default;

	bool IsValid() const {
		return m_IsValid && m_File.good();
	}

	mpt::os_path GetFilename() const {
		return m_Filename;
	}

	std::istream & GetStream() {
		return m_File;
	}
};



class CachedInputFile {

private:
	mpt::os_path m_Filename;
	mpt::IO::ifstream m_File;
	bool m_IsValid;
	bool m_IsCached;
	std::vector<std::byte> m_Cache;

public:
	CachedInputFile(const mpt::os_path & filename)
		: m_Filename(filename)
		, m_File(m_Filename, std::ios::binary | std::ios::in)
		, m_IsValid(false)
		, m_IsCached(false) {
		assert(!filename.empty());
		if (mpt::IO::IsReadSeekable(m_File)) {
			if (!mpt::IO::SeekEnd(m_File)) {
				m_File.close();
				return;
			}
			mpt::IO::Offset filesize = mpt::IO::TellRead(m_File);
			if (!mpt::IO::SeekBegin(m_File)) {
				m_File.close();
				return;
			}
			if (mpt::in_range<std::size_t>(filesize)) {
				std::size_t buffersize = mpt::saturate_cast<std::size_t>(filesize);
				m_Cache.resize(buffersize);
				if (mpt::IO::ReadRaw(m_File, mpt::as_span(m_Cache)).size() != mpt::saturate_cast<std::size_t>(filesize)) {
					m_File.close();
					return;
				}
				if (!mpt::IO::SeekBegin(m_File)) {
					m_File.close();
					return;
				}
				m_IsCached = true;
				m_IsValid = true;
				return;
			}
		}
		m_IsValid = true;
		return;
	}

	~CachedInputFile() = default;

	bool IsValid() const {
		return m_IsValid && m_File.good();
	}

	bool IsCached() const {
		return m_IsCached;
	}

	mpt::os_path GetFilename() const {
		return m_Filename;
	}

	std::istream & GetStream() {
		assert(!m_IsCached);
		return m_File;
	}

	mpt::const_byte_span GetCache() {
		assert(m_IsCached);
		return mpt::as_span(m_Cache);
	}
};



class InputFile {

private:
	std::variant<std::monostate, UncachedInputFile, CachedInputFile> m_impl;

public:
	InputFile(const mpt::os_path & filename, bool enable_cache = false) {
		if (enable_cache) {
			m_impl.emplace<CachedInputFile>(filename);
		} else {
			m_impl.emplace<UncachedInputFile>(filename);
		}
	}

	~InputFile() = default;

	bool IsValid() const {
		if (std::holds_alternative<UncachedInputFile>(m_impl)) {
			return std::get<UncachedInputFile>(m_impl).IsValid();
		} else if (std::holds_alternative<CachedInputFile>(m_impl)) {
			return std::get<CachedInputFile>(m_impl).IsValid();
		}
		return false;
	}

	bool IsCached() const {
		if (std::holds_alternative<UncachedInputFile>(m_impl)) {
			return false;
		} else if (std::holds_alternative<CachedInputFile>(m_impl)) {
			return std::get<CachedInputFile>(m_impl).IsCached();
		}
		throw false;
	}

	mpt::os_path GetFilename() const {
		if (std::holds_alternative<UncachedInputFile>(m_impl)) {
			return std::get<UncachedInputFile>(m_impl).GetFilename();
		} else if (std::holds_alternative<CachedInputFile>(m_impl)) {
			return std::get<CachedInputFile>(m_impl).GetFilename();
		}
		return {};
	}

	std::istream & GetStream() {
		if (std::holds_alternative<UncachedInputFile>(m_impl)) {
			return std::get<UncachedInputFile>(m_impl).GetStream();
		} else if (std::holds_alternative<CachedInputFile>(m_impl)) {
			return std::get<CachedInputFile>(m_impl).GetStream();
		}
		throw std::bad_variant_access();
	}

	mpt::const_byte_span GetCache() {
		if (std::holds_alternative<UncachedInputFile>(m_impl)) {
			throw std::bad_variant_access();
		} else if (std::holds_alternative<CachedInputFile>(m_impl)) {
			return std::get<CachedInputFile>(m_impl).GetCache();
		}
		throw std::bad_variant_access();
	}
};



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_FILE_INPUTFILE_HPP
