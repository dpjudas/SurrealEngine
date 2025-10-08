/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_UUID_GUID_HPP
#define MPT_UUID_GUID_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/out_of_memory/out_of_memory.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"
#include "mpt/uuid/uuid.hpp"

#include <stdexcept>
#include <vector>

#if MPT_OS_WINDOWS
#if MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)
#include <guiddef.h>
#endif // MPT_WIN_VISTA
#include <objbase.h>
#include <rpc.h>
#endif // MPT_OS_WINDOWS



namespace mpt {
inline namespace MPT_INLINE_NS {



#if MPT_OS_WINDOWS && !defined(MPT_COMPILER_QUIRK_NO_WCHAR)



// COM CLSID<->string conversion
// A CLSID string is not necessarily a standard UUID string,
// it might also be a symbolic name for the interface.
// (see CLSIDFromString ( http://msdn.microsoft.com/en-us/library/windows/desktop/ms680589%28v=vs.85%29.aspx ))

inline mpt::winstring CLSIDToString(CLSID clsid) {
	std::wstring str;
	LPOLESTR tmp = nullptr;
	switch (::StringFromCLSID(clsid, &tmp)) {
		case S_OK:
			break;
		case E_OUTOFMEMORY:
			if (tmp) {
				::CoTaskMemFree(tmp);
				tmp = nullptr;
			}
			mpt::throw_out_of_memory();
			break;
		default:
			if (tmp) {
				::CoTaskMemFree(tmp);
				tmp = nullptr;
			}
			throw std::logic_error("StringFromCLSID() failed.");
			break;
	}
	if (!tmp) {
		throw std::logic_error("StringFromCLSID() failed.");
	}
	try {
		str = tmp;
	} catch (mpt::out_of_memory e) {
		::CoTaskMemFree(tmp);
		tmp = nullptr;
		mpt::rethrow_out_of_memory(e);
	}
	::CoTaskMemFree(tmp);
	tmp = nullptr;
	return mpt::transcode<mpt::winstring>(str);
}

inline CLSID StringToCLSID(const mpt::winstring & str_) {
	const std::wstring str = mpt::transcode<std::wstring>(str_);
	CLSID clsid = CLSID();
	std::vector<OLECHAR> tmp(str.c_str(), str.c_str() + str.length() + 1);
	switch (::CLSIDFromString(tmp.data(), &clsid)) {
		case NOERROR:
			// nothing
			break;
		case E_INVALIDARG:
			clsid = CLSID();
			break;
		case CO_E_CLASSSTRING:
			clsid = CLSID();
			break;
		case REGDB_E_CLASSNOTREG:
			clsid = CLSID();
			break;
		case REGDB_E_READREGDB:
			clsid = CLSID();
			throw std::runtime_error("CLSIDFromString() failed: REGDB_E_READREGDB.");
			break;
		default:
			clsid = CLSID();
			throw std::logic_error("CLSIDFromString() failed.");
			break;
	}
	return clsid;
}

inline bool VerifyStringToCLSID(const mpt::winstring & str_, CLSID & clsid) {
	const std::wstring str = mpt::transcode<std::wstring>(str_);
	bool result = false;
	clsid = CLSID();
	std::vector<OLECHAR> tmp(str.c_str(), str.c_str() + str.length() + 1);
	switch (::CLSIDFromString(tmp.data(), &clsid)) {
		case NOERROR:
			result = true;
			break;
		case E_INVALIDARG:
			result = false;
			break;
		case CO_E_CLASSSTRING:
			result = false;
			break;
		case REGDB_E_CLASSNOTREG:
			result = false;
			break;
		case REGDB_E_READREGDB:
			throw std::runtime_error("CLSIDFromString() failed: REGDB_E_READREGDB.");
			break;
		default:
			throw std::logic_error("CLSIDFromString() failed.");
			break;
	}
	return result;
}

inline bool IsCLSID(const mpt::winstring & str_) {
	const std::wstring str = mpt::transcode<std::wstring>(str_);
	bool result = false;
	CLSID clsid = CLSID();
	std::vector<OLECHAR> tmp(str.c_str(), str.c_str() + str.length() + 1);
	switch (::CLSIDFromString(tmp.data(), &clsid)) {
		case NOERROR:
			result = true;
			break;
		case E_INVALIDARG:
			result = false;
			break;
		case CO_E_CLASSSTRING:
			result = false;
			break;
		case REGDB_E_CLASSNOTREG:
			result = false;
			break;
		case REGDB_E_READREGDB:
			result = false;
			throw std::runtime_error("CLSIDFromString() failed: REGDB_E_READREGDB.");
			break;
		default:
			result = false;
			throw std::logic_error("CLSIDFromString() failed.");
			break;
	}
	return result;
}


// COM IID<->string conversion

inline IID StringToIID(const mpt::winstring & str_) {
	const std::wstring str = mpt::transcode<std::wstring>(str_);
	IID iid = IID();
	std::vector<OLECHAR> tmp(str.c_str(), str.c_str() + str.length() + 1);
	switch (::IIDFromString(tmp.data(), &iid)) {
		case S_OK:
			// nothing
			break;
		case E_OUTOFMEMORY:
			iid = IID();
			mpt::throw_out_of_memory();
			break;
		case E_INVALIDARG:
			iid = IID();
			break;
		default:
			iid = IID();
			throw std::logic_error("IIDFromString() failed.");
			break;
	}
	return iid;
}

inline mpt::winstring IIDToString(IID iid) {
	std::wstring str;
	LPOLESTR tmp = nullptr;
	switch (::StringFromIID(iid, &tmp)) {
		case S_OK:
			break;
		case E_OUTOFMEMORY:
			if (tmp) {
				::CoTaskMemFree(tmp);
				tmp = nullptr;
			}
			mpt::throw_out_of_memory();
			break;
		default:
			if (tmp) {
				::CoTaskMemFree(tmp);
				tmp = nullptr;
			}
			throw std::logic_error("StringFromIID() failed.");
			break;
	}
	if (!tmp) {
		throw std::logic_error("StringFromIID() failed.");
	}
	try {
		str = tmp;
	} catch (mpt::out_of_memory e) {
		::CoTaskMemFree(tmp);
		tmp = nullptr;
		mpt::rethrow_out_of_memory(e);
	}
	return mpt::transcode<mpt::winstring>(str);
}


// General GUID<->string conversion.
// The string must/will be in standard GUID format: {4F9A455D-E7EF-4367-B2F0-0C83A38A5C72}

inline GUID StringToGUID(const mpt::winstring & str) {
	return StringToIID(str);
}

inline mpt::winstring GUIDToString(GUID guid) {
	std::vector<OLECHAR> tmp(256);
	if (::StringFromGUID2(guid, tmp.data(), static_cast<int>(tmp.size())) <= 0) {
		throw std::logic_error("StringFromGUID2() failed.");
	}
	return mpt::transcode<mpt::winstring>(tmp.data());
}


// Create a COM GUID

inline GUID CreateGUID() {
	GUID guid = GUID();
	switch (::CoCreateGuid(&guid)) {
		case S_OK:
			// nothing
			break;
		default:
			guid = GUID();
			throw std::runtime_error("CoCreateGuid() failed.");
	}
	return guid;
}


// Checks the UUID against the NULL UUID. Returns false if it is NULL, true otherwise.

inline bool IsValid(::UUID uuid) {
	return false
		|| uuid.Data1 != 0
		|| uuid.Data2 != 0
		|| uuid.Data3 != 0
		|| uuid.Data4[0] != 0
		|| uuid.Data4[1] != 0
		|| uuid.Data4[2] != 0
		|| uuid.Data4[3] != 0
		|| uuid.Data4[4] != 0
		|| uuid.Data4[5] != 0
		|| uuid.Data4[6] != 0
		|| uuid.Data4[7] != 0;
}



#endif // MPT_OS_WINDOWS && !MPT_COMPILER_QUIRK_NO_WCHAR



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_UUID_GUID_HPP
