/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_PATH_BASIC_PATH_HPP
#define MPT_PATH_BASIC_PATH_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/detect/mfc.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace path_literals {

template <typename Tchar>
struct literals;

template <typename Tchar>
MPT_CONSTEVAL Tchar L(char x) {
	return path_literals::literals<Tchar>::L(x);
}

template <typename Tchar, std::size_t N>
MPT_CONSTEVAL const Tchar * L(const char (&x)[N]) {
	return path_literals::literals<Tchar>::L(x);
}

template <>
struct literals<char> {
	using char_type = char;
	static MPT_CONSTEVAL char_type L(char c) {
		if (c == '\0')
			return '\0';
		if (c == '\\')
			return '\\';
		if (c == '/')
			return '/';
		if (c == '.')
			return '.';
		if (c == ':')
			return ':';
		if (c == '?')
			return '?';
		if (c == '*')
			return '*';
		if (c == '|')
			return '|';
		if (c == '<')
			return '<';
		if (c == '>')
			return '>';
		if (c == '\"')
			return '\"';
		if (c == '_')
			return '_';
#if defined(MPT_COMPILER_QUIRK_NO_CONSTEXPR_THROW)
		else
			return false ? 0 : throw std::domain_error("invalid path literal");
#else
		throw std::domain_error("invalid path literal");
#endif
	}
	template <std::size_t N>
	static MPT_CONSTEVAL const char_type * L(const char (&s)[N]) {
		if (std::string_view(s) == std::string_view(""))
			return "";
		if (std::string_view(s) == std::string_view("/"))
			return "/";
		if (std::string_view(s) == std::string_view("."))
			return ".";
		if (std::string_view(s) == std::string_view("\\"))
			return "\\";
		if (std::string_view(s) == std::string_view(".."))
			return "..";
		if (std::string_view(s) == std::string_view("//"))
			return "//";
		if (std::string_view(s) == std::string_view("./"))
			return "./";
		if (std::string_view(s) == std::string_view(".\\"))
			return ".\\";
		if (std::string_view(s) == std::string_view("\\/"))
			return "\\/";
		if (std::string_view(s) == std::string_view("/\\"))
			return "/\\";
		if (std::string_view(s) == std::string_view("\\\\"))
			return "\\\\";
		if (std::string_view(s) == std::string_view("\\\\?\\"))
			return "\\\\?\\";
		if (std::string_view(s) == std::string_view("\\\\?\\UNC"))
			return "\\\\?\\UNC";
		if (std::string_view(s) == std::string_view("\\\\?\\UNC\\"))
			return "\\\\?\\UNC\\";
#if defined(MPT_COMPILER_QUIRK_NO_CONSTEXPR_THROW)
		else
			return false ? nullptr : throw std::domain_error("invalid path literal");
#else
		throw std::domain_error("invalid path literal");
#endif
	}
};

#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
template <>
struct literals<wchar_t> {
	using char_type = wchar_t;
	static MPT_CONSTEVAL char_type L(char c) {
		if (c == '\0')
			return L'\0';
		if (c == '\\')
			return L'\\';
		if (c == '/')
			return L'/';
		if (c == '.')
			return L'.';
		if (c == ':')
			return L':';
		if (c == '?')
			return L'?';
		if (c == '*')
			return L'*';
		if (c == '|')
			return L'|';
		if (c == '<')
			return L'<';
		if (c == '>')
			return L'>';
		if (c == '\"')
			return L'\"';
		if (c == '_')
			return L'_';
#if defined(MPT_COMPILER_QUIRK_NO_CONSTEXPR_THROW)
		else
			return false ? 0 : throw std::domain_error("invalid path literal");
#else
		throw std::domain_error("invalid path literal");
#endif
	}
	template <std::size_t N>
	static MPT_CONSTEVAL const char_type * L(const char (&s)[N]) {
		if (std::string_view(s) == std::string_view(""))
			return L"";
		if (std::string_view(s) == std::string_view("/"))
			return L"/";
		if (std::string_view(s) == std::string_view("."))
			return L".";
		if (std::string_view(s) == std::string_view("\\"))
			return L"\\";
		if (std::string_view(s) == std::string_view(".."))
			return L"..";
		if (std::string_view(s) == std::string_view("//"))
			return L"//";
		if (std::string_view(s) == std::string_view("./"))
			return L"./";
		if (std::string_view(s) == std::string_view(".\\"))
			return L".\\";
		if (std::string_view(s) == std::string_view("\\/"))
			return L"\\/";
		if (std::string_view(s) == std::string_view("/\\"))
			return L"/\\";
		if (std::string_view(s) == std::string_view("\\\\"))
			return L"\\\\";
		if (std::string_view(s) == std::string_view("\\\\?\\"))
			return L"\\\\?\\";
		if (std::string_view(s) == std::string_view("\\\\?\\UNC"))
			return L"\\\\?\\UNC";
		if (std::string_view(s) == std::string_view("\\\\?\\UNC\\"))
			return L"\\\\?\\UNC\\";
#if defined(MPT_COMPILER_QUIRK_NO_CONSTEXPR_THROW)
		else
			return false ? nullptr : throw std::domain_error("invalid path literal");
#else
		throw std::domain_error("invalid path literal");
#endif
	}
};
#endif // !MPT_COMPILER_QUIRK_NO_WCHAR

#if MPT_CXX_AT_LEAST(20)
template <>
struct literals<char8_t> {
	using char_type = char8_t;
	static MPT_CONSTEVAL char_type L(char c) {
		if (c == '\0')
			return u8'\0';
		if (c == '\\')
			return u8'\\';
		if (c == '/')
			return u8'/';
		if (c == '.')
			return u8'.';
		if (c == ':')
			return u8':';
		if (c == '?')
			return u8'?';
		if (c == '*')
			return u8'*';
		if (c == '|')
			return u8'|';
		if (c == '<')
			return u8'<';
		if (c == '>')
			return u8'>';
		if (c == '\"')
			return u8'\"';
		if (c == '_')
			return u8'_';
#if defined(MPT_COMPILER_QUIRK_NO_CONSTEXPR_THROW)
		else
			return false ? 0 : throw std::domain_error("invalid path literal");
#else
		throw std::domain_error("invalid path literal");
#endif
	}
	template <std::size_t N>
	static MPT_CONSTEVAL const char_type * L(const char (&s)[N]) {
		if (std::string_view(s) == std::string_view(""))
			return u8"";
		if (std::string_view(s) == std::string_view("/"))
			return u8"/";
		if (std::string_view(s) == std::string_view("."))
			return u8".";
		if (std::string_view(s) == std::string_view("\\"))
			return u8"\\";
		if (std::string_view(s) == std::string_view(".."))
			return u8"..";
		if (std::string_view(s) == std::string_view("//"))
			return u8"//";
		if (std::string_view(s) == std::string_view("./"))
			return u8"./";
		if (std::string_view(s) == std::string_view(".\\"))
			return u8".\\";
		if (std::string_view(s) == std::string_view("\\/"))
			return u8"\\/";
		if (std::string_view(s) == std::string_view("/\\"))
			return u8"/\\";
		if (std::string_view(s) == std::string_view("\\\\"))
			return u8"\\\\";
		if (std::string_view(s) == std::string_view("\\\\?\\"))
			return u8"\\\\?\\";
		if (std::string_view(s) == std::string_view("\\\\?\\UNC"))
			return u8"\\\\?\\UNC";
		if (std::string_view(s) == std::string_view("\\\\?\\UNC\\"))
			return u8"\\\\?\\UNC\\";
#if defined(MPT_COMPILER_QUIRK_NO_CONSTEXPR_THROW)
		else
			return false ? nullptr : throw std::domain_error("invalid path literal");
#else
		throw std::domain_error("invalid path literal");
#endif
	}
};
#endif // C++20

template <>
struct literals<char16_t> {
	using char_type = char16_t;
	static MPT_CONSTEVAL char_type L(char c) {
		if (c == '\0')
			return u'\0';
		if (c == '\\')
			return u'\\';
		if (c == '/')
			return u'/';
		if (c == '.')
			return u'.';
		if (c == ':')
			return u':';
		if (c == '?')
			return u'?';
		if (c == '*')
			return u'*';
		if (c == '|')
			return u'|';
		if (c == '<')
			return u'<';
		if (c == '>')
			return u'>';
		if (c == '\"')
			return u'\"';
		if (c == '_')
			return u'_';
#if defined(MPT_COMPILER_QUIRK_NO_CONSTEXPR_THROW)
		else
			return false ? 0 : throw std::domain_error("invalid path literal");
#else
		throw std::domain_error("invalid path literal");
#endif
	}
	template <std::size_t N>
	static MPT_CONSTEVAL const char_type * L(const char (&s)[N]) {
		if (std::string_view(s) == std::string_view(""))
			return u"";
		if (std::string_view(s) == std::string_view("/"))
			return u"/";
		if (std::string_view(s) == std::string_view("."))
			return u".";
		if (std::string_view(s) == std::string_view("\\"))
			return u"\\";
		if (std::string_view(s) == std::string_view(".."))
			return u"..";
		if (std::string_view(s) == std::string_view("//"))
			return u"//";
		if (std::string_view(s) == std::string_view("./"))
			return u"./";
		if (std::string_view(s) == std::string_view(".\\"))
			return u".\\";
		if (std::string_view(s) == std::string_view("\\/"))
			return u"\\/";
		if (std::string_view(s) == std::string_view("/\\"))
			return u"/\\";
		if (std::string_view(s) == std::string_view("\\\\"))
			return u"\\\\";
		if (std::string_view(s) == std::string_view("\\\\?\\"))
			return u"\\\\?\\";
		if (std::string_view(s) == std::string_view("\\\\?\\UNC"))
			return u"\\\\?\\UNC";
		if (std::string_view(s) == std::string_view("\\\\?\\UNC\\"))
			return u"\\\\?\\UNC\\";
#if defined(MPT_COMPILER_QUIRK_NO_CONSTEXPR_THROW)
		else
			return false ? nullptr : throw std::domain_error("invalid path literal");
#else
		throw std::domain_error("invalid path literal");
#endif
	}
};

template <>
struct literals<char32_t> {
	using char_type = char32_t;
	static MPT_CONSTEVAL char_type L(char c) {
		if (c == '\0')
			return U'\0';
		if (c == '\\')
			return U'\\';
		if (c == '/')
			return U'/';
		if (c == '.')
			return U'.';
		if (c == ':')
			return U':';
		if (c == '?')
			return U'?';
		if (c == '*')
			return U'*';
		if (c == '|')
			return U'|';
		if (c == '<')
			return U'<';
		if (c == '>')
			return U'>';
		if (c == '\"')
			return U'\"';
		if (c == '_')
			return U'_';
#if defined(MPT_COMPILER_QUIRK_NO_CONSTEXPR_THROW)
		else
			return false ? 0 : throw std::domain_error("invalid path literal");
#else
		throw std::domain_error("invalid path literal");
#endif
	}
	template <std::size_t N>
	static MPT_CONSTEVAL const char_type * L(const char (&s)[N]) {
		if (std::string_view(s) == std::string_view(""))
			return U"";
		if (std::string_view(s) == std::string_view("/"))
			return U"/";
		if (std::string_view(s) == std::string_view("."))
			return U".";
		if (std::string_view(s) == std::string_view("\\"))
			return U"\\";
		if (std::string_view(s) == std::string_view(".."))
			return U"..";
		if (std::string_view(s) == std::string_view("//"))
			return U"//";
		if (std::string_view(s) == std::string_view("./"))
			return U"./";
		if (std::string_view(s) == std::string_view(".\\"))
			return U".\\";
		if (std::string_view(s) == std::string_view("\\/"))
			return U"\\/";
		if (std::string_view(s) == std::string_view("/\\"))
			return U"/\\";
		if (std::string_view(s) == std::string_view("\\\\"))
			return U"\\\\";
		if (std::string_view(s) == std::string_view("\\\\?\\"))
			return U"\\\\?\\";
		if (std::string_view(s) == std::string_view("\\\\?\\UNC"))
			return U"\\\\?\\UNC";
		if (std::string_view(s) == std::string_view("\\\\?\\UNC\\"))
			return U"\\\\?\\UNC\\";
#if defined(MPT_COMPILER_QUIRK_NO_CONSTEXPR_THROW)
		else
			return false ? nullptr : throw std::domain_error("invalid path literal");
#else
		throw std::domain_error("invalid path literal");
#endif
	}
};

} // namespace path_literals



enum class PathStyle {
	Posix,
	DOS_DJGPP,
	Windows9x,
	WindowsNT,
};

template <PathStyle EStyle>
struct PathStyleTag {
	static inline constexpr PathStyle path_style = EStyle;
};



template <typename TRawPath, typename PathStyleTag>
struct PathTraits {



	static inline constexpr PathStyle path_style = PathStyleTag::path_style;
	using raw_path_type = TRawPath;
	using char_type = typename raw_path_type::value_type;



	static bool IsPathSeparator(char_type c) {
		using namespace path_literals;
		bool result{};
		if constexpr ((path_style == PathStyle::WindowsNT) || (path_style == PathStyle::Windows9x) || (path_style == PathStyle::DOS_DJGPP)) {
			result = (c == L<char_type>('\\')) || (c == L<char_type>('/'));
		} else if constexpr (path_style == PathStyle::Posix) {
			result = (c == L<char_type>('/'));
		} else {
			//static_assert(false);
		}
		return result;
	}



	static char_type GetDefaultPathSeparator() {
		using namespace path_literals;
		char_type result{};
		if constexpr ((path_style == PathStyle::WindowsNT) || (path_style == PathStyle::Windows9x) || (path_style == PathStyle::DOS_DJGPP)) {
			result = L<char_type>('\\');
		} else if constexpr (path_style == PathStyle::Posix) {
			result = L<char_type>('/');
		} else {
			//static_assert(false);
		}
		return result;
	}



	static bool IsValidComponentChar(char_type c) {
		using namespace path_literals;
		bool result = true;
		if constexpr ((path_style == PathStyle::WindowsNT) || (path_style == PathStyle::Windows9x) || (path_style == PathStyle::DOS_DJGPP)) {
			if (c == L<char_type>('\\') || c == L<char_type>('\"') || c == L<char_type>('/') || c == L<char_type>(':') || c == L<char_type>('?') || c == L<char_type>('<') || c == L<char_type>('>') || c == L<char_type>('|') || c == L<char_type>('*')) {
				result = false;
			} else {
				result = true;
			}
		} else if constexpr (path_style == PathStyle::Posix) {
			result = (c != L<char_type>('/'));
		} else {
			// nothing
		}
		return result;
	}



	static char_type InvalidComponentCharReplacement() {
		using namespace path_literals;
		return L<char_type>('_');
	}



#if MPT_GCC_AT_LEAST(12, 0, 0) && MPT_GCC_BEFORE(13, 1, 0)
// Work-around <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105329> /
// <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105651>.
#pragma GCC push_options
#if defined(__OPTIMIZE__)
#pragma GCC optimize("O1")
#endif
// Work-around brain-damaged GCC warning 'void operator delete(void*, std::size_t)' called on a pointer to an unallocated object '"\\\000\\\000\000"'.
// Probably a duplicate of one of the many incarnations of <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=99098>.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif
	static void SplitPath(raw_path_type p, raw_path_type * prefix, raw_path_type * drive, raw_path_type * dir, raw_path_type * fbase, raw_path_type * fext) {

		using namespace path_literals;

		if (prefix) {
			*prefix = raw_path_type();
		}
		if (drive) {
			*drive = raw_path_type();
		}
		if (dir) {
			*dir = raw_path_type();
		}
		if (fbase) {
			*fbase = raw_path_type();
		}
		if (fext) {
			*fext = raw_path_type();
		}

		if constexpr ((path_style == PathStyle::WindowsNT) || (path_style == PathStyle::Windows9x) || (path_style == PathStyle::DOS_DJGPP)) {

			// We cannot use CRT splitpath here, because:
			//  * limited to _MAX_PATH or similar
			//  * no support for UNC paths
			//  * no support for \\?\ prefixed paths

			if constexpr (path_style == PathStyle::WindowsNT) {
				// remove \\?\\ prefix
				if (p.substr(0, 8) == L<char_type>("\\\\?\\UNC\\")) {
					if (prefix) {
						*prefix = L<char_type>("\\\\?\\UNC");
					}
					p = L<char_type>("\\\\") + p.substr(8);
				} else if (p.substr(0, 4) == L<char_type>("\\\\?\\")) {
					if (prefix) {
						*prefix = L<char_type>("\\\\?\\");
					}
					p = p.substr(4);
				}
			}

			MPT_MAYBE_CONSTANT_IF (((path_style == PathStyle::WindowsNT) || (path_style == PathStyle::Windows9x)) && (p.length() >= 2) && (p.substr(0, 2) == L<char_type>("\\\\") || p.substr(0, 2) == L<char_type>("\\/") || p.substr(0, 2) == L<char_type>("/\\") || p.substr(0, 2) == L<char_type>("//"))) {
				// UNC
				typename raw_path_type::size_type first_slash = p.substr(2).find_first_of(L<char_type>("\\/"));
				if (first_slash != raw_path_type::npos) {
					typename raw_path_type::size_type second_slash = p.substr(2 + first_slash + 1).find_first_of(L<char_type>("\\/"));
					if (second_slash != raw_path_type::npos) {
						if (drive) {
							*drive = p.substr(0, 2 + first_slash + 1 + second_slash);
						}
						p = p.substr(2 + first_slash + 1 + second_slash);
					} else {
						if (drive) {
							*drive = p;
						}
						p = raw_path_type();
					}
				} else {
					if (drive) {
						*drive = p;
					}
					p = raw_path_type();
				}
			} else {
				// local
				if (p.length() >= 2 && (p[1] == L<char_type>(':'))) {
					if (drive) {
						*drive = p.substr(0, 2);
					}
					p = p.substr(2);
				} else {
					if (drive) {
						*drive = raw_path_type();
					}
				}
			}
			typename raw_path_type::size_type last_slash = p.find_last_of(L<char_type>("\\/"));
			if (last_slash != raw_path_type::npos) {
				if (dir) {
					*dir = p.substr(0, last_slash + 1);
				}
				p = p.substr(last_slash + 1);
			} else {
				if (dir) {
					*dir = raw_path_type();
				}
			}
			typename raw_path_type::size_type last_dot = p.find_last_of(L<char_type>("."));
			if (last_dot == raw_path_type::npos) {
				if (fbase) {
					*fbase = p;
				}
				if (fext) {
					*fext = raw_path_type();
				}
			} else if (last_dot == 0) {
				if (fbase) {
					*fbase = p;
				}
				if (fext) {
					*fext = raw_path_type();
				}
			} else if (p == L<char_type>(".") || p == L<char_type>("..")) {
				if (fbase) {
					*fbase = p;
				}
				if (fext) {
					*fext = raw_path_type();
				}
			} else {
				if (fbase) {
					*fbase = p.substr(0, last_dot);
				}
				if (fext) {
					*fext = p.substr(last_dot);
				}
			}

		} else if constexpr (path_style == PathStyle::Posix) {

			typename raw_path_type::size_type last_slash = p.find_last_of(L<char_type>("/"));
			if (last_slash != raw_path_type::npos) {
				if (dir) {
					*dir = p.substr(0, last_slash + 1);
				}
				p = p.substr(last_slash + 1);
			} else {
				if (dir) {
					*dir = raw_path_type();
				}
			}
			typename raw_path_type::size_type last_dot = p.find_last_of(L<char_type>("."));
			if (last_dot == raw_path_type::npos) {
				if (fbase) {
					*fbase = p;
				}
				if (fext) {
					*fext = raw_path_type();
				}
			} else if (last_dot == 0) {
				if (fbase) {
					*fbase = p;
				}
				if (fext) {
					*fext = raw_path_type();
				}
			} else if (p == L<char_type>(".") || p == L<char_type>("..")) {
				if (fbase) {
					*fbase = p;
				}
				if (fext) {
					*fext = raw_path_type();
				}
			} else {
				if (fbase) {
					*fbase = p.substr(0, last_dot);
				}
				if (fext) {
					*fext = p.substr(last_dot);
				}
			}

		} else {

			//static_assert(false);
		}
	}
#if MPT_GCC_AT_LEAST(12, 0, 0) && MPT_GCC_BEFORE(13, 1, 0)
#pragma GCC diagnostic pop
#pragma GCC pop_options
#endif



#if MPT_GCC_AT_LEAST(12, 0, 0) && MPT_GCC_BEFORE(13, 1, 0)
// Work-around <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105329> /
// <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105651>.
#pragma GCC push_options
#if defined(__OPTIMIZE__)
#pragma GCC optimize("O1")
#endif
#endif
	// Convert a path to its simplified form, i.e. remove ".\" and "..\" entries
	// Note: We use our own implementation as PathCanonicalize is limited to MAX_PATH
	// and unlimited versions are only available on Windows 8 and later.
	// Furthermore, we also convert forward-slashes to backslashes and always remove trailing slashes.
	static raw_path_type Simplify(const raw_path_type & path) {

		using namespace path_literals;

		raw_path_type result{};

		if (path.empty()) {
			return result;
		}

		std::vector<raw_path_type> components;

		if constexpr ((path_style == PathStyle::WindowsNT) || (path_style == PathStyle::Windows9x) || (path_style == PathStyle::DOS_DJGPP)) {

			raw_path_type root;
			typename raw_path_type::size_type startPos = 0;
			if (path.size() >= 2 && path[1] == L<char_type>(':')) {
				// Drive letter
				root = path.substr(0, 2) + L<char_type>('\\');
				startPos = 2;
			} else MPT_MAYBE_CONSTANT_IF (((path_style == PathStyle::WindowsNT) || (path_style == PathStyle::Windows9x)) && (path.substr(0, 2) == L<char_type>("\\\\"))) {
				// Network share
				root = L<char_type>("\\\\");
				startPos = 2;
			} else if (path.substr(0, 2) == L<char_type>(".\\") || path.substr(0, 2) == L<char_type>("./")) {
				// Special case for relative paths
				root = L<char_type>(".\\");
				startPos = 2;
			} else if (path.size() >= 1 && (path[0] == L<char_type>('\\') || path[0] == L<char_type>('/'))) {
				// Special case for relative paths
				root = L<char_type>("\\");
				startPos = 1;
			}

			while (startPos < path.size()) {
				auto pos = path.find_first_of(L<char_type>("\\/"), startPos);
				if (pos == raw_path_type::npos) {
					pos = path.size();
				}
				raw_path_type dir = path.substr(startPos, pos - startPos);
				if (dir == L<char_type>("..")) {
					// Go back one directory
					if (!components.empty()) {
						components.pop_back();
					}
				} else if (dir == L<char_type>(".")) {
					// nop
				} else if (!dir.empty()) {
					components.push_back(std::move(dir));
				}
				startPos = pos + 1;
			}

			result = root;
			result.reserve(path.size());
			for (const auto & component : components) {
				result += component + L<char_type>("\\");
			}
			if (!components.empty()) {
				result.pop_back();
			}

		} else if constexpr (path_style == PathStyle::Posix) {

			raw_path_type root;
			typename raw_path_type::size_type startPos = 0;
			if (path.substr(0, 2) == L<char_type>("./")) {
				// Special case for relative paths
				root = L<char_type>("./");
				startPos = 2;
			} else if (path.size() >= 1 && (path[0] == L<char_type>('/'))) {
				// Special case for relative paths
				root = L<char_type>("/");
				startPos = 1;
			}

			while (startPos < path.size()) {
				auto pos = path.find_first_of(L<char_type>("/"), startPos);
				if (pos == raw_path_type::npos) {
					pos = path.size();
				}
				raw_path_type dir = path.substr(startPos, pos - startPos);
				if (dir == L<char_type>("..")) {
					// Go back one directory
					if (!components.empty()) {
						components.pop_back();
					}
				} else if (dir == L<char_type>(".")) {
					// nop
				} else if (!dir.empty()) {
					components.push_back(std::move(dir));
				}
				startPos = pos + 1;
			}

			result = root;
			result.reserve(path.size());
			for (const auto & component : components) {
				result += component + L<char_type>("/");
			}
			if (!components.empty()) {
				result.pop_back();
			}

		} else {

			//static_assert(false);
		}

		return result;
	}
#if MPT_GCC_AT_LEAST(12, 0, 0) && MPT_GCC_BEFORE(13, 1, 0)
#pragma GCC pop_options
#endif



#if MPT_GCC_AT_LEAST(12, 0, 0) && MPT_GCC_BEFORE(13, 1, 0)
// Work-around <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105329> /
// <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105651>.
#pragma GCC push_options
#if defined(__OPTIMIZE__)
#pragma GCC optimize("O1")
#endif
#endif
	static bool IsAbsolute(const raw_path_type & path) {
		using namespace path_literals;
		bool result{};
		if constexpr (path_style == PathStyle::WindowsNT) {
			if (path.substr(0, 8) == L<char_type>("\\\\?\\UNC\\")) {
				return true;
			}
			if (path.substr(0, 4) == L<char_type>("\\\\?\\")) {
				return true;
			}
			if (path.substr(0, 2) == L<char_type>("\\\\")) {
				return true; // UNC
			}
			if (path.substr(0, 2) == L<char_type>("//")) {
				return true; // UNC
			}
			result = (path.length()) >= 3 && (path[1] == L<char_type>(':')) && IsPathSeparator(path[2]);
		} else if constexpr (path_style == PathStyle::Windows9x) {
			if (path.substr(0, 2) == L<char_type>("\\\\")) {
				return true; // UNC
			}
			if (path.substr(0, 2) == L<char_type>("//")) {
				return true; // UNC
			}
			result = (path.length()) >= 3 && (path[1] == L<char_type>(':')) && IsPathSeparator(path[2]);
		} else if constexpr (path_style == PathStyle::DOS_DJGPP) {
			result = (path.length()) >= 3 && (path[1] == L<char_type>(':')) && IsPathSeparator(path[2]);
		} else if constexpr (path_style == PathStyle::Posix) {
			result = (path.length() >= 1) && IsPathSeparator(path[0]);
		} else {
			//static_assert(false);
		}
		return result;
	}
#if MPT_GCC_AT_LEAST(12, 0, 0) && MPT_GCC_BEFORE(13, 1, 0)
#pragma GCC pop_options
#endif
};



template <typename Traits, bool allow_transcode_locale = true>
class BasicPathString {

public:

	using path_traits = Traits;
	using raw_path_type = typename path_traits::raw_path_type;
	using char_type = typename raw_path_type::value_type;

private:

	raw_path_type path;

public:

	template <typename Tpath, typename std::enable_if<std::is_same<Tpath, raw_path_type>::value, bool>::type = true>
	BasicPathString(const Tpath & path_)
		: path(path_) {
		return;
	}

	template <typename Tpath, typename std::enable_if<std::is_same<Tpath, raw_path_type>::value, bool>::type = true>
	operator Tpath() const {
		return path;
	}

public:

	BasicPathString() = default;
	BasicPathString(const BasicPathString &) = default;
	BasicPathString(BasicPathString &&) noexcept = default;

	BasicPathString & assign(const BasicPathString & other) {
		path = other.path;
		return *this;
	}

	BasicPathString & assign(BasicPathString && other) noexcept {
		path = std::move(other.path);
		return *this;
	}

	BasicPathString & operator=(const BasicPathString & other) {
		return assign(other);
	}

	BasicPathString & operator=(BasicPathString && other) noexcept {
		return assign(std::move(other));
	}

	BasicPathString & append(const BasicPathString & other) {
		path.append(other.path);
		return *this;
	}

	BasicPathString & operator+=(const BasicPathString & other) {
		return append(other);
	}

	friend BasicPathString operator+(const BasicPathString & a, const BasicPathString & b) {
		return BasicPathString(a).append(b);
	}

	friend bool operator<(const BasicPathString & a, const BasicPathString & b) {
		return a.AsNative() < b.AsNative();
	}

	friend bool operator==(const BasicPathString & a, const BasicPathString & b) {
		return a.AsNative() == b.AsNative();
	}

	friend bool operator!=(const BasicPathString & a, const BasicPathString & b) {
		return a.AsNative() != b.AsNative();
	}

	bool empty() const {
		return path.empty();
	}

	std::size_t length() const {
		return path.size();
	}

	std::size_t Length() const {
		return path.size();
	}

public:

	raw_path_type AsNative() const {
		return path;
	}

	static BasicPathString FromNative(const raw_path_type & path) {
		return BasicPathString(path);
	}

	mpt::ustring ToUnicode() const {
		return mpt::transcode<mpt::ustring>(path);
	}

	static BasicPathString FromUnicode(const mpt::ustring & path) {
		return BasicPathString(mpt::transcode<raw_path_type>(path));
	}

	std::string ToUTF8() const {
		return mpt::transcode<std::string>(mpt::common_encoding::utf8, path);
	}

	static BasicPathString FromUTF8(const std::string & path) {
		return BasicPathString(mpt::transcode<raw_path_type>(mpt::common_encoding::utf8, path));
	}

#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)

	std::wstring ToWide() const {
		return mpt::transcode<std::wstring>(path);
	}

	static BasicPathString FromWide(const std::wstring & path) {
		return BasicPathString(mpt::transcode<raw_path_type>(path));
	}

#endif // !MPT_COMPILER_QUIRK_NO_WCHAR

	template <bool enable = allow_transcode_locale, typename std::enable_if<enable, bool>::type = true>
	std::string ToLocale() const {
		return mpt::transcode<std::string>(mpt::logical_encoding::locale, path);
	}

	template <bool enable = allow_transcode_locale, typename std::enable_if<enable, bool>::type = true>
	static BasicPathString FromLocale(const std::string & path) {
		return BasicPathString(mpt::transcode<raw_path_type>(mpt::logical_encoding::locale, path));
	}

#if MPT_DETECTED_MFC

	CString ToCString() const {
		return mpt::transcode<CString>(path);
	}

	static BasicPathString FromCString(const CString & path) {
		return BasicPathString(mpt::transcode<raw_path_type>(path));
	}

#endif // MPT_DETECTED_MFC

public:

	static bool IsPathSeparator(char_type c) {
		return path_traits::IsPathSeparator(c);
	}

	static char_type GetDefaultPathSeparator() {
		return path_traits::GetDefaultPathSeparator();
	}

	bool HasTrailingSlash() const {
		if (path.empty()) {
			return false;
		}
		char_type c = path[path.length() - 1];
		return IsPathSeparator(c);
	}

	BasicPathString AsSanitizedComponent() const {
		BasicPathString result = *this;
		for (auto & c : result.path) {
			if (!path_traits::IsValidComponentChar(c)) {
				c = path_traits::InvalidComponentCharReplacement();
			}
		}
		return result;
	}

	BasicPathString WithoutTrailingSlash() const {
		BasicPathString result = *this;
		while (result.HasTrailingSlash()) {
			if (result.Length() == 1) {
				return result;
			}
			result = BasicPathString(result.AsNative().substr(0, result.AsNative().length() - 1));
		}
		return result;
	}

	BasicPathString WithTrailingSlash() const {
		BasicPathString result = *this;
		if (!result.empty() && !result.HasTrailingSlash()) {
			result.path += GetDefaultPathSeparator();
		}
		return result;
	}

	void SplitPath(BasicPathString * prefix, BasicPathString * drive, BasicPathString * dir, BasicPathString * fbase, BasicPathString * fext) const {
		path_traits::SplitPath(path, prefix ? &prefix->path : nullptr, drive ? &drive->path : nullptr, dir ? &dir->path : nullptr, fbase ? &fbase->path : nullptr, fext ? &fext->path : nullptr);
	}

	// \\?\ or \\?\\UNC or empty
	BasicPathString GetPrefix() const {
		BasicPathString prefix;
		SplitPath(&prefix, nullptr, nullptr, nullptr, nullptr);
		return prefix;
	}

	// Drive letter + colon, e.g. "C:" or \\server\share
	BasicPathString GetDrive() const {
		BasicPathString drive;
		SplitPath(nullptr, &drive, nullptr, nullptr, nullptr);
		return drive;
	}

	// Directory, e.g. "\OpenMPT\"
	BasicPathString GetDirectory() const {
		BasicPathString dir;
		SplitPath(nullptr, nullptr, &dir, nullptr, nullptr);
		return dir;
	}

	// Drive + Dir, e.g. "C:\OpenMPT\"
	BasicPathString GetDirectoryWithDrive() const {
		BasicPathString drive, dir;
		SplitPath(nullptr, &drive, &dir, nullptr, nullptr);
		return drive + dir;
	}

	// File name without extension, e.g. "OpenMPT"
	BasicPathString GetFilenameBase() const {
		BasicPathString fname;
		SplitPath(nullptr, nullptr, nullptr, &fname, nullptr);
		return fname;
	}

	// Extension including dot, e.g. ".exe"
	BasicPathString GetFilenameExtension() const {
		BasicPathString ext;
		SplitPath(nullptr, nullptr, nullptr, nullptr, &ext);
		return ext;
	}

	// File name + extension, e.g. "OpenMPT.exe"
	BasicPathString GetFilename() const {
		BasicPathString name, ext;
		SplitPath(nullptr, nullptr, nullptr, &name, &ext);
		return name + ext;
	}

	// Return the same path string with a different (or appended) extension (including "."), e.g. "foo.bar",".txt" -> "foo.txt" or "C:\OpenMPT\foo",".txt" -> "C:\OpenMPT\foo.txt"
	BasicPathString ReplaceExtension(const BasicPathString & newExt) const {
		return GetDirectoryWithDrive() + GetFilenameBase() + newExt;
	}

	// Convert a path to its simplified form, i.e. remove ".\" and "..\" entries, similar to std::fs::path::lexically_normal
	BasicPathString Simplify() const {
		return BasicPathString::FromNative(path_traits::Simplify(path));
	}

	bool IsAbsolute() const {
		return path_traits::IsAbsolute(path);
	}

	bool is_absolute() const {
		return path_traits::IsAbsolute(path);
	}

	bool is_relative() const {
		return !path_traits::IsAbsolute(path);
	}
};



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_PATH_BASIC_PATH_HPP
