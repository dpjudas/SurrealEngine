/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_PATH_PATH_HPP
#define MPT_PATH_PATH_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/path/os_path.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"

#if !defined(MPT_COMPILER_QUIRK_NO_FILESYSTEM)
#include <filesystem>
#endif // !MPT_COMPILER_QUIRK_NO_FILESYSTEM
#include <type_traits>
#include <utility>
#if MPT_OS_WINDOWS && defined(MPT_COMPILER_QUIRK_NO_FILESYSTEM)
#include <vector>
#endif // MPT_OS_WINDOWS && MPT_COMPILER_QUIRK_NO_FILESYSTEM

#if MPT_OS_WINDOWS
#include <windows.h>
#endif // MPT_OS_WINDOWS



namespace mpt {
inline namespace MPT_INLINE_NS {



#if defined(MPT_COMPILER_QUIRK_NO_FILESYSTEM)



using path = mpt::os_path;
#define MPT_PATH_CHAR(x)    MPT_OSPATH_CHAR(x)
#define MPT_PATH_LITERAL(x) MPT_OSPATH_LITERAL(x)
#define MPT_PATH(x)         MPT_OSPATH(x)




#else // !MPT_COMPILER_QUIRK_NO_FILESYSTEM



template <>
struct make_string_type<std::filesystem::path> {
	using type = std::filesystem::path;
};


template <>
struct is_string_type<std::filesystem::path> : public std::true_type { };



template <>
struct string_transcoder<std::filesystem::path> {
	using string_type = std::filesystem::path;
	static inline mpt::widestring decode(const string_type & src) {
		if constexpr (std::is_same<std::filesystem::path::value_type, char>::value) {
			// In contrast to standard recommendation and cppreference,
			// native encoding on unix-like systems with libstdc++ or libc++ is actually *NOT* UTF8,
			// but instead the conventional std::locale::locale("") encoding (which happens to be UTF8 on all modern systems, but this is not guaranteed).
			// Note: libstdc++ and libc++ however assume that their internal representation is UTF8,
			// which implies that wstring/u32string/u16string/u8string conversions are actually broken and MUST NOT be used, ever.
			return mpt::transcode<mpt::widestring>(mpt::logical_encoding::locale, src.string());
#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
		} else if constexpr (std::is_same<std::filesystem::path::value_type, wchar_t>::value) {
			return mpt::transcode<mpt::widestring>(src.wstring());
#endif // !MPT_COMPILER_QUIRK_NO_WCHAR
		} else if constexpr (std::is_same<std::filesystem::path::value_type, char32_t>::value) {
			return mpt::transcode<mpt::widestring>(src.u32string());
		} else if constexpr (std::is_same<std::filesystem::path::value_type, char16_t>::value) {
			return mpt::transcode<mpt::widestring>(src.u16string());
#if MPT_CXX_AT_LEAST(20)
		} else if constexpr (std::is_same<std::filesystem::path::value_type, char8_t>::value) {
			return mpt::transcode<mpt::widestring>(src.u8string());
#endif
		} else {
#if MPT_OS_WINDOWS && !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
			return mpt::transcode<mpt::widestring>(src.wstring());
#elif MPT_OS_WINDOWS
			return mpt::transcode<mpt::widestring>(mpt::logical_encoding::locale, src.string());
#else
			// completely unknown implementation, assume it can sanely convert to/from UTF16/UTF32
			if constexpr (sizeof(mpt::widechar) == sizeof(char32_t)) {
				return mpt::transcode<mpt::widestring>(src.u32string());
			} else if constexpr (sizeof(mpt::widechar) == sizeof(char16_t)) {
				return mpt::transcode<mpt::widestring>(src.u16string());
			} else {
				return mpt::transcode<mpt::widestring>(src.u32string());
			}
#endif
		}
	}
	static inline string_type encode(const mpt::widestring & src, std::filesystem::path::format fmt) {
		if constexpr (std::is_same<std::filesystem::path::value_type, char>::value) {
			// In contrast to standard recommendation and cppreference,
			// native encoding on unix-like systems with libstdc++ or libc++ is actually *NOT* UTF8,
			// but instead the conventional std::locale::locale("") encoding (which happens to be UTF8 on all modern systems, but this is not guaranteed).
			// Note: libstdc++ and libc++ however assume that their internal representation is UTF8,
			// which implies that wstring/u32string/u16string/u8string conversions are actually broken and MUST NOT be used, ever.
			return std::filesystem::path{mpt::transcode<std::string>(mpt::logical_encoding::locale, src), fmt};
#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
		} else if constexpr (std::is_same<std::filesystem::path::value_type, wchar_t>::value) {
			return std::filesystem::path{mpt::transcode<std::wstring>(src), fmt};
#endif // !MPT_COMPILER_QUIRK_NO_WCHAR
		} else if constexpr (std::is_same<std::filesystem::path::value_type, char32_t>::value) {
			return std::filesystem::path{mpt::transcode<std::u32string>(src), fmt};
		} else if constexpr (std::is_same<std::filesystem::path::value_type, char16_t>::value) {
			return std::filesystem::path{mpt::transcode<std::u16string>(src), fmt};
#if MPT_CXX_AT_LEAST(20)
		} else if constexpr (std::is_same<std::filesystem::path::value_type, char8_t>::value) {
			return std::filesystem::path{mpt::transcode<std::u8string>(src), fmt};
#endif
		} else {
#if MPT_OS_WINDOWS && !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
			return std::filesystem::path{mpt::transcode<std::wstring>(src), fmt};
#elif MPT_OS_WINDOWS
			return std::filesystem::path{mpt::transcode<std::string>(mpt::logical_encoding::locale, src), fmt};
#else
			// completely unknown implementation, assume it can sanely convert to/from UTF16/UTF32
			if constexpr (sizeof(mpt::widechar) == sizeof(char32_t)) {
				return std::filesystem::path{mpt::transcode<std::u32string>(src), fmt};
			} else if constexpr (sizeof(mpt::widechar) == sizeof(char16_t)) {
				return std::filesystem::path{mpt::transcode<std::u16string>(src), fmt};
			} else {
				return std::filesystem::path{mpt::transcode<std::u32string>(src), fmt};
			}
#endif
		}
	}
	static inline string_type encode(const mpt::widestring & src) {
		return encode(src, std::filesystem::path::auto_format);
	}
};



// Best heuristics we can come up with to define std::filesystem::path literals that do not involve (or at least only non-lossy) runtime conversion.

#if MPT_OS_WINDOWS && !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
#define MPT_STDPATH_CHAR(x)    L##x
#define MPT_STDPATH_LITERAL(x) L##x
#define MPT_STDPATH(x) \
	std::filesystem::path { \
		L##x \
	}
#elif MPT_OS_WINDOWS
#define MPT_STDPATH_CHAR(x)    x
#define MPT_STDPATH_LITERAL(x) x
#define MPT_STDPATH(x) \
	std::filesystem::path { \
		x \
	}
#elif MPT_CXX_AT_LEAST(20)
#define MPT_STDPATH_CHAR(x)    u8##x
#define MPT_STDPATH_LITERAL(x) u8##x
#define MPT_STDPATH(x) \
	std::filesystem::path { \
		u8##x \
	}
#else
#define MPT_STDPATH_CHAR(x)    U##x
#define MPT_STDPATH_LITERAL(x) U##x
#define MPT_STDPATH(x) \
	std::filesystem::path { \
		U##x \
	}
#endif



// std::filesystem::path offers implicit conversions to/from types of which it is confused about their encodings.
// The only way to work around this problem is to implement our own mpt::path that does not do such broken nonsense.
// We offer no implicit conversions and only explicit conversions from std::filesystem::path and mpt::os_path.

class path {
public:
	using format = std::filesystem::path::format;
	using std_value_type = std::filesystem::path::value_type;
	using std_string_type = std::filesystem::path;
	static constexpr inline std_value_type std_preferred_separator = std::filesystem::path::preferred_separator;
	using os_value_type = os_path::value_type;
	using os_string_type = os_path;
	static constexpr inline os_value_type os_preferred_separator = static_cast<os_value_type>(std::filesystem::path::preferred_separator);

private:
	std::filesystem::path m_path;

private:
	template <typename path_type, std::enable_if_t<std::is_same<path_type, std::filesystem::path>::value, bool> = true>
	explicit path(const path_type & p)
		: m_path(p) {
		return;
	}
	template <typename path_type, std::enable_if_t<std::is_same<path_type, std::filesystem::path>::value, bool> = true>
	explicit path(path_type && p)
		: m_path(std::forward<path_type>(p)) {
		return;
	}

public:
	struct internal {
		static inline path make_path(std::filesystem::path && p) {
			return path{std::move(p)};
		}
	};

public:
	template <typename path_type, std::enable_if_t<std::is_same<path_type, std::filesystem::path>::value, bool> = true>
	static path from_stdpath(const path_type & p) {
		return path{p};
	}
	static std::filesystem::path to_stdpath(const path & p) {
		return p.m_path;
	}
	static os_path to_ospath(const path & p) {
		return mpt::transcode<os_path>(p.m_path);
	}
	static std::filesystem::path from_ospath(const os_path & s, std::filesystem::path::format fmt = std::filesystem::path::auto_format) {
		return string_transcoder<std::filesystem::path>{}.encode(mpt::transcode<mpt::widestring>(s), fmt);
	}

public:
	path() noexcept = default;
	path(const path & p)
		: m_path(p.m_path) {
		return;
	}
	path(path && p)
		: m_path(std::move(p.m_path)) {
		return;
	}
	explicit path(const os_path & s, std::filesystem::path::format fmt = std::filesystem::path::auto_format)
		: m_path(from_ospath(s, fmt)) {
		return;
	}
	path & operator=(const path & p) {
		m_path = p.m_path;
		return *this;
	}
	path & operator=(path && p) {
		m_path = std::move(p.m_path);
		return *this;
	}
	path & assign(const path & p) {
		m_path = p.m_path;
		return *this;
	}
	path & assign(path && p) {
		m_path = std::move(p.m_path);
		return *this;
	}
	path & operator/=(const path & p) {
		m_path /= p.m_path;
		return *this;
	}
	path & operator/=(path && p) {
		m_path /= std::move(p.m_path);
		return *this;
	}
	// concatenation
	path & append(const path & p) {
		m_path /= p.m_path;
		return *this;
	}
	path & append(path && p) {
		m_path /= std::move(p.m_path);
		return *this;
	}
	path & operator+=(const path & p) {
		m_path += p.m_path;
		return *this;
	}
	path & operator+=(path && p) {
		m_path += std::move(p.m_path);
		return *this;
	}
	path & concat(const path & p) {
		m_path += p.m_path;
		return *this;
	}
	path & concat(path && p) {
		m_path += std::move(p.m_path);
		return *this;
	}
	// modifiers
	void clear() noexcept {
		m_path.clear();
	}
	path & make_preferred() {
		m_path.make_preferred();
		return *this;
	}
	path & remove_filename() {
		m_path.remove_filename();
		return *this;
	}
	path & replace_filename(const path & replacement) {
		m_path.replace_filename(replacement.m_path);
		return *this;
	}
	path & replace_extension(const path & replacement = path()) {
		m_path.replace_extension(replacement.m_path);
		return *this;
	}
	void swap(path & other) {
		m_path.swap(other.m_path);
	}
	// format observers
	std::filesystem::path stdpath() const {
		return m_path;
	}
	os_path ospath() const {
		return to_ospath(*this);
	}
	// compare
	int compare(const path & p) const noexcept {
		return m_path.compare(p.m_path);
	}
	// generation
	path lexically_normal() const {
		return path{m_path.lexically_normal()};
	}
	path lexically_relative(const path & base) const {
		return path{m_path.lexically_relative(base.m_path)};
	}
	path lexically_proximate(const path & base) const {
		return path{m_path.lexically_proximate(base.m_path)};
	}
	// decomposition
	path root_name() const {
		return path{m_path.root_name()};
	}
	path root_directory() const {
		return path{m_path.root_directory()};
	}
	path root_path() const {
		return path{m_path.root_path()};
	}
	path relative_path() const {
		return path{m_path.relative_path()};
	}
	path parent_path() const {
		return path{m_path.parent_path()};
	}
	path filename() const {
		return path{m_path.filename()};
	}
	path stem() const {
		return path{m_path.stem()};
	}
	path extension() const {
		return path{m_path.extension()};
	}
	// queries
	[[nodiscard]] bool empty() const noexcept {
		return m_path.empty();
	}
	bool has_root_path() const {
		return m_path.has_root_path();
	}
	bool has_root_name() const {
		return m_path.has_root_name();
	}
	bool has_root_directory() const {
		return m_path.has_root_directory();
	}
	bool has_relative_path() const {
		return m_path.has_relative_path();
	}
	bool has_parent_path() const {
		return m_path.has_parent_path();
	}
	bool has_filename() const {
		return m_path.has_filename();
	}
	bool has_stem() const {
		return m_path.has_stem();
	}
	bool has_extension() const {
		return m_path.has_extension();
	}
	bool is_absolute() const {
		return m_path.is_absolute();
	}
	bool is_relative() const {
		return m_path.is_relative();
	}
	// comparison operators
	friend bool operator==(const path & lhs, const path & rhs) noexcept {
		return lhs.m_path == rhs.m_path;
	}
	friend bool operator!=(const path & lhs, const path & rhs) noexcept {
		return lhs.m_path != rhs.m_path;
	}
	friend bool operator<(const path & lhs, const path & rhs) noexcept {
		return lhs.m_path < rhs.m_path;
	}
	friend bool operator<=(const path & lhs, const path & rhs) noexcept {
		return lhs.m_path <= rhs.m_path;
	}
	friend bool operator>(const path & lhs, const path & rhs) noexcept {
		return lhs.m_path > rhs.m_path;
	}
	friend bool operator>=(const path & lhs, const path & rhs) noexcept {
		return lhs.m_path >= rhs.m_path;
	}
	// copncatenation operator
	friend path operator/(const path & lhs, const path & rhs) {
		return path{lhs.m_path / rhs.m_path};
	}
};



// Best heuristics we can come up with to define mpt::path literals that do not involve (or at least only non-lossy) runtime conversion.

#if MPT_OS_WINDOWS && !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
#define MPT_PATH_CHAR(x)    L##x
#define MPT_PATH_LITERAL(x) L##x
#define MPT_PATH(x)         mpt::path::internal::make_path(L##x)
#elif MPT_OS_WINDOWS
#define MPT_PATH_CHAR(x)    x
#define MPT_PATH_LITERAL(x) x
#define MPT_PATH(x)         mpt::path::internal::make_path(x)
#elif MPT_CXX_AT_LEAST(20)
#define MPT_PATH_CHAR(x)    u8##x
#define MPT_PATH_LITERAL(x) u8##x
#define MPT_PATH(x)         mpt::path::internal::make_path(u8##x)
#else
#define MPT_PATH_CHAR(x)    U##x
#define MPT_PATH_LITERAL(x) U##x
#define MPT_PATH(x)         mpt::path::internal::make_path(U##x)
#endif



template <>
struct make_string_type<mpt::path> {
	using type = mpt::path;
};


template <>
struct is_string_type<mpt::path> : public std::true_type { };



template <>
struct string_transcoder<mpt::path> {
	using string_type = mpt::path;
	static inline mpt::widestring decode(const string_type & src) {
		return mpt::transcode<mpt::widestring>(src.ospath());
	}
	static inline string_type encode(const mpt::widestring & src) {
		return mpt::path{mpt::transcode<mpt::os_path>(src)};
	}
};



#endif // MPT_COMPILER_QUIRK_NO_FILESYSTEM



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_PATH_PATH_HPP
