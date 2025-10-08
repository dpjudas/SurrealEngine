/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_FILE_ADAPTER_FILEADAPTER_HPP
#define MPT_IO_FILE_ADAPTER_FILEADAPTER_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/saturate_cast.hpp"
#include "mpt/io/base.hpp"
#if !MPT_OS_WINDOWS
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/io_file/fstream.hpp"
#endif // !MPT_OS_WINDOWS
#include "mpt/io_file_unique/unique_basename.hpp"
#include "mpt/io_file_unique/unique_tempfilename.hpp"
#if MPT_OS_WINDOWS
#include "mpt/path/os_path.hpp"
#endif // MPT_OS_WINDOWS
#include "mpt/string_transcode/transcode.hpp"
#if MPT_OS_WINDOWS
#include "mpt/system_error/system_error.hpp"
#endif // MPT_OS_WINDOWS

#include <stdexcept>
#if !MPT_OS_WINDOWS
#include <system_error>
#endif // MPT_OS_WINDOWS

#include <cstddef>
#if !MPT_OS_WINDOWS
#include <cstdio>
#endif // !MPT_OS_WINDOWS

#if MPT_OS_WINDOWS
#include <windows.h>
#endif // MPT_OS_WINDOWS

#if MPT_OS_WINDOWS
#if MPT_OS_WINDOWS_WINRT
#if MPT_WINRT_AT_LEAST(MPT_WIN_8)
#define MPT_IO_FILE_ONDISKFILEWRAPPER_USE_CREATEFILE
#endif
#else // !MPT_OS_WINDOWS_WINRT
#define MPT_IO_FILE_ONDISKFILEWRAPPER_USE_CREATEFILE
#endif // MPT_OS_WINDOWS_WINRT
#endif // MPT_OS_WINDOWS



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



template <typename TFileCursor>
class FileAdapter {

private:
	mpt::os_path m_Filename;
	bool m_IsTempFile;

public:
	FileAdapter(TFileCursor & file, const mpt::os_path & tempName = unique_tempfilename{unique_basename{}, MPT_OS_PATH("tmp")})
		: m_IsTempFile(false) {
		try {
			file.Rewind();
			if (!file.GetOptionalFileName()) {

#ifdef MPT_IO_FILE_ONDISKFILEWRAPPER_USE_CREATEFILE

				HANDLE hFile = NULL;
#if MPT_OS_WINDOWS_WINRT
				hFile = mpt::windows::CheckFileHANDLE(CreateFile2(mpt::support_long_path(tempName).c_str(), GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS, NULL));
#else
				hFile = mpt::windows::CheckFileHANDLE(CreateFile(mpt::support_long_path(tempName).c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL));
#endif
				while (!file.EndOfFile()) {
					typename TFileCursor::PinnedView view = file.ReadPinnedView(mpt::IO::BUFFERSIZE_NORMAL);
					std::size_t towrite = view.size();
					std::size_t written = 0;
					do {
						DWORD chunkSize = mpt::saturate_cast<DWORD>(towrite);
						DWORD chunkDone = 0;
						try {
							mpt::windows::CheckBOOL(WriteFile(hFile, view.data() + written, chunkSize, &chunkDone, NULL));
						} catch (...) {
							CloseHandle(hFile);
							hFile = NULL;
							throw;
						}
						if (chunkDone != chunkSize) {
							CloseHandle(hFile);
							hFile = NULL;
							throw std::runtime_error("Incomplete WriteFile().");
						}
						towrite -= chunkDone;
						written += chunkDone;
					} while (towrite > 0);
				}
				CloseHandle(hFile);
				hFile = NULL;

#else // !MPT_IO_FILE_ONDISKFILEWRAPPER_USE_CREATEFILE

				mpt::IO::ofstream f(tempName, std::ios::binary);
				if (!f) {
					throw std::runtime_error("Error creating temporary file.");
				}
				while (!file.EndOfFile()) {
					typename TFileCursor::PinnedView view = file.ReadPinnedView(mpt::IO::BUFFERSIZE_NORMAL);
					std::size_t towrite = view.size();
					std::size_t written = 0;
					do {
						std::size_t chunkSize = mpt::saturate_cast<std::size_t>(towrite);
						bool chunkOk = false;
						chunkOk = mpt::IO::WriteRaw(f, mpt::const_byte_span(view.data() + written, chunkSize));
						if (!chunkOk) {
							throw std::runtime_error("Incomplete Write.");
						}
						towrite -= chunkSize;
						written += chunkSize;
					} while (towrite > 0);
				}
				f.close();

#endif // MPT_ONDISKFILEWRAPPER_NO_CREATEFILE

				m_Filename = tempName;
				m_IsTempFile = true;
			} else {
#if defined(MPT_LIBCXX_QUIRK_NO_OPTIONAL_VALUE)
				m_Filename = *(file.GetOptionalFileName());
#else
				m_Filename = file.GetOptionalFileName().value();
#endif
			}
		} catch (const std::runtime_error &) {
			m_Filename = mpt::os_path{};
			m_IsTempFile = false;
		}
	}

	~FileAdapter() {
		if (m_IsTempFile) {
#if MPT_OS_WINDOWS
			DeleteFile(m_Filename.c_str());
#else  // !MPT_OS_WINDOWS
			std::remove(m_Filename.c_str());
			//std::error_code ec{};
			//std::filesystem::remove(mpt::transcode<std::filesystem::path>(m_Filename.AsNative()), ec);
#endif // MPT_OS_WINDOWS
			m_IsTempFile = false;
		}
		m_Filename = mpt::os_path{};
	}

public:
	bool IsValid() const {
		return !m_Filename.empty();
	}

	mpt::os_path GetFilename() const {
		return m_Filename;
	}

}; // class FileAdapter



} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_FILE_ADAPTER_FILEADAPTER_HPP
