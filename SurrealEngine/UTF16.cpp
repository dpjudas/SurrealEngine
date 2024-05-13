
#include "Precomp.h"
#include "UTF16.h"
#include "Exception.h"

#ifdef WIN32

std::string from_utf16(const std::wstring& str)
{
	if (str.empty()) return {};
	int needed = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0, nullptr, nullptr);
	if (needed == 0)
		Exception::Throw("WideCharToMultiByte failed");
	std::string result;
	result.resize(needed);
	needed = WideCharToMultiByte(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], (int)result.size(), nullptr, nullptr);
	if (needed == 0)
		Exception::Throw("WideCharToMultiByte failed");
	return result;
}

std::wstring to_utf16(const std::string& str)
{
	if (str.empty()) return {};
	int needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
	if (needed == 0)
		Exception::Throw("MultiByteToWideChar failed");
	std::wstring result;
	result.resize(needed);
	needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], (int)result.size());
	if (needed == 0)
		Exception::Throw("MultiByteToWideChar failed");
	return result;
	}

#else

std::string from_utf16(const std::wstring& str)
{
	Exception::Throw("from_utf16 not implemented on unix");
}

std::wstring to_utf16(const std::string& str)
{
	Exception::Throw("to_utf16 not implemented on unix");
}

#endif
