/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_ENVIRONMENT_ENVIRONMENT_HPP
#define MPT_ENVIRONMENT_ENVIRONMENT_HPP

#include "mpt/base/detect.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"
#include "mpt/system_error/system_error.hpp"

#include <optional>
#if MPT_OS_WINDOWS
#if defined(UNICODE) && !MPT_OS_WINDOWS_WINRT
#include <vector>
#endif // !MPT_OS_WINDOWS_WINRT
#endif // MPT_OS_WINDOWS

#include <cstdlib>

#if MPT_OS_WINDOWS
#include <windows.h>
#endif // MPT_OS_WINDOWS



namespace mpt {
inline namespace MPT_INLINE_NS {



inline std::optional<mpt::ustring> getenv(const mpt::ustring & env_var) {
#if MPT_OS_WINDOWS && MPT_OS_WINDOWS_WINRT
	MPT_UNUSED(env_var);
	return std::nullopt;
#elif MPT_OS_WINDOWS && defined(UNICODE)
	std::vector<WCHAR> buf(32767);
	DWORD size = GetEnvironmentVariable(mpt::transcode<std::wstring>(env_var).c_str(), buf.data(), 32767);
	if (size == 0) {
		mpt::windows::ExpectError(ERROR_ENVVAR_NOT_FOUND);
		return std::nullopt;
	}
	return mpt::transcode<mpt::ustring>(buf.data());
#else
	const char * val = std::getenv(mpt::transcode<std::string>(mpt::environment_encoding, env_var).c_str());
	if (!val) {
		return std::nullopt;
	}
	return mpt::transcode<mpt::ustring>(mpt::environment_encoding, val);
#endif
}



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_ENVIRONMENT_ENVIRONMENT_HPP
