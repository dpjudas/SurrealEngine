/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_SYSTEM_ERROR_SYSTEM_ERROR_HPP
#define MPT_SYSTEM_ERROR_SYSTEM_ERROR_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/format/message.hpp"
#include "mpt/format/message_macros.hpp"
#include "mpt/format/simple.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"
#include "mpt/out_of_memory/out_of_memory.hpp"

#if MPT_OS_WINDOWS
#include <stdexcept>
#if MPT_OS_WINDOWS_WINRT
#include <vector>
#endif // MPT_OS_WINDOWS_WINRT
#endif // MPT_OS_WINDOWS

#if MPT_OS_WINDOWS
#include <windows.h>
#endif // MPT_OS_WINDOWS



namespace mpt {
inline namespace MPT_INLINE_NS {



#if MPT_OS_WINDOWS



namespace windows {



inline mpt::ustring GetErrorMessage(DWORD errorCode, HANDLE hModule = NULL) {
#if MPT_OS_WINDOWS_WINRT
	std::vector<TCHAR> msgbuf(65536);
	if (FormatMessage(
			(hModule ? FORMAT_MESSAGE_FROM_HMODULE : 0) | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			hModule,
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			msgbuf.data(),
			mpt::saturate_cast<DWORD>(msgbuf.size()),
			NULL)
		== 0)
	{
		DWORD e = GetLastError();
		if ((e == ERROR_NOT_ENOUGH_MEMORY) || (e == ERROR_OUTOFMEMORY)) {
			mpt::throw_out_of_memory();
		}
		return {};
	}
	return mpt::transcode<mpt::ustring>(mpt::winstring{msgbuf.data()});
#else
	mpt::ustring message;
	void * lpMsgBuf = nullptr;
	if (FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | (hModule ? FORMAT_MESSAGE_FROM_HMODULE : 0) | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			hModule,
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0,
			NULL)
		== 0)
	{
		DWORD e = GetLastError();
		if (lpMsgBuf) {
			LocalFree(lpMsgBuf);
		}
		if ((e == ERROR_NOT_ENOUGH_MEMORY) || (e == ERROR_OUTOFMEMORY)) {
			mpt::throw_out_of_memory();
		}
		return {};
	}
	if (!lpMsgBuf) {
		return {};
	}
	try {
		message = mpt::transcode<mpt::ustring>(mpt::winstring{static_cast<LPTSTR>(lpMsgBuf)});
	} catch (mpt::out_of_memory e) {
		LocalFree(lpMsgBuf);
		mpt::rethrow_out_of_memory(e);
	}
	LocalFree(lpMsgBuf);
	return message;
#endif
}


class error
	: public std::runtime_error {
public:
	error(DWORD errorCode, HANDLE hModule = NULL)
		: std::runtime_error(mpt::transcode<std::string>(mpt::exception_encoding, MPT_UFORMAT_MESSAGE("Windows Error: 0x{}: {}")(mpt::format<mpt::ustring>::hex0<8>(errorCode), GetErrorMessage(errorCode, hModule)))) {
		return;
	}
};


inline HANDLE CheckFileHANDLE(HANDLE handle) {
	if (handle == INVALID_HANDLE_VALUE) {
		DWORD err = ::GetLastError();
		if ((err == ERROR_NOT_ENOUGH_MEMORY) || (err == ERROR_OUTOFMEMORY)) {
			mpt::throw_out_of_memory();
		}
		throw windows::error(err);
	}
	return handle;
}


inline HANDLE CheckHANDLE(HANDLE handle) {
	if (handle == NULL) {
		DWORD err = ::GetLastError();
		if ((err == ERROR_NOT_ENOUGH_MEMORY) || (err == ERROR_OUTOFMEMORY)) {
			mpt::throw_out_of_memory();
		}
		throw windows::error(err);
	}
	return handle;
}


inline void CheckBOOL(BOOL result) {
	if (result == FALSE) {
		DWORD err = ::GetLastError();
		if ((err == ERROR_NOT_ENOUGH_MEMORY) || (err == ERROR_OUTOFMEMORY)) {
			mpt::throw_out_of_memory();
		}
		throw windows::error(err);
	}
}


inline void ExpectError(DWORD expected) {
	DWORD err = ::GetLastError();
	if (err != expected) {
		if ((err == ERROR_NOT_ENOUGH_MEMORY) || (err == ERROR_OUTOFMEMORY)) {
			mpt::throw_out_of_memory();
		}
		throw windows::error(err);
	}
}



} // namespace windows



#endif // MPT_OS_WINDOWS



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_SYSTEM_ERROR_SYSTEM_ERROR_HPP
