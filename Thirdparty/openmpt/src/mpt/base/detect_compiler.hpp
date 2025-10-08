/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_DETECT_COMPILER_HPP
#define MPT_BASE_DETECT_COMPILER_HPP



#define MPT_COMPILER_MAKE_VERSION2(version, sp)         ((version)*100 + (sp))
#define MPT_COMPILER_MAKE_VERSION3(major, minor, patch) ((major)*10000 + (minor)*100 + (patch))



#if defined(MPT_COMPILER_GENERIC)

#undef MPT_COMPILER_GENERIC
#define MPT_COMPILER_GENERIC 1

#elif defined(__clang__) && defined(_MSC_VER) && defined(__c2__)

#error "Clang/C2 is not supported. Please use Clang/LLVM for Windows instead."

#elif defined(__clang__)

#define MPT_COMPILER_CLANG                      1
#define MPT_COMPILER_CLANG_VERSION              MPT_COMPILER_MAKE_VERSION3(__clang_major__, __clang_minor__, __clang_patchlevel__)
#define MPT_CLANG_AT_LEAST(major, minor, patch) (MPT_COMPILER_CLANG_VERSION >= MPT_COMPILER_MAKE_VERSION3((major), (minor), (patch)))
#define MPT_CLANG_BEFORE(major, minor, patch)   (MPT_COMPILER_CLANG_VERSION < MPT_COMPILER_MAKE_VERSION3((major), (minor), (patch)))

#if MPT_CLANG_BEFORE(6, 0, 0)
#error "clang version 6 required"
#endif

#if defined(__clang_analyzer__)
#ifndef MPT_BUILD_ANALYZED
#define MPT_BUILD_ANALYZED
#endif
#endif

#elif defined(__GNUC__)

#define MPT_COMPILER_GCC                      1
#define MPT_COMPILER_GCC_VERSION              MPT_COMPILER_MAKE_VERSION3(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#define MPT_GCC_AT_LEAST(major, minor, patch) (MPT_COMPILER_GCC_VERSION >= MPT_COMPILER_MAKE_VERSION3((major), (minor), (patch)))
#define MPT_GCC_BEFORE(major, minor, patch)   (MPT_COMPILER_GCC_VERSION < MPT_COMPILER_MAKE_VERSION3((major), (minor), (patch)))

#if MPT_GCC_BEFORE(7, 1, 0)
#error "GCC version 7.1 required"
#endif

#elif defined(_MSC_VER)

#define MPT_COMPILER_MSVC 1
#if (_MSC_VER >= 1944)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 14)
#elif (_MSC_VER >= 1943)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 13)
#elif (_MSC_VER >= 1942)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 12)
#elif (_MSC_VER >= 1941)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 11)
#elif (_MSC_VER >= 1940)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 10)
#elif (_MSC_VER >= 1939)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 9)
#elif (_MSC_VER >= 1938)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 8)
#elif (_MSC_VER >= 1937)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 7)
#elif (_MSC_VER >= 1936)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 6)
#elif (_MSC_VER >= 1935)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 5)
#elif (_MSC_VER >= 1934)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 4)
#elif (_MSC_VER >= 1933)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 3)
#elif (_MSC_VER >= 1932)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 2)
#elif (_MSC_VER >= 1931)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 1)
#elif (_MSC_VER >= 1930)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2022, 0)
#elif (_MSC_VER >= 1929)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2019, 10)
#elif (_MSC_VER >= 1928)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2019, 8)
#elif (_MSC_VER >= 1927)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2019, 7)
#elif (_MSC_VER >= 1926)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2019, 6)
#elif (_MSC_VER >= 1925)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2019, 5)
#elif (_MSC_VER >= 1924)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2019, 4)
#elif (_MSC_VER >= 1923)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2019, 3)
#elif (_MSC_VER >= 1922)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2019, 2)
#elif (_MSC_VER >= 1921)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2019, 1)
#elif (_MSC_VER >= 1920)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2019, 0)
#elif (_MSC_VER >= 1916)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2017, 9)
#elif (_MSC_VER >= 1915)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2017, 8)
#elif (_MSC_VER >= 1914)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2017, 7)
#elif (_MSC_VER >= 1913)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2017, 6)
#elif (_MSC_VER >= 1912)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2017, 5)
#elif (_MSC_VER >= 1911)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2017, 3)
#elif (_MSC_VER >= 1910)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2017, 0)
#elif (_MSC_VER >= 1900) && defined(_MSVC_LANG)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2015, 3)
#elif (_MSC_VER >= 1900)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2015, 0)
#elif (_MSC_VER >= 1800)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2013, 0)
#elif (_MSC_VER >= 1700)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2012, 0)
#elif (_MSC_VER >= 1600)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2010, 0)
#elif (_MSC_VER >= 1500)
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2008, 0)
#else
#define MPT_COMPILER_MSVC_VERSION MPT_COMPILER_MAKE_VERSION2(2005, 0)
#endif
#define MPT_MSVC_AT_LEAST(version, sp) (MPT_COMPILER_MSVC_VERSION >= MPT_COMPILER_MAKE_VERSION2((version), (sp)))
#define MPT_MSVC_BEFORE(version, sp)   (MPT_COMPILER_MSVC_VERSION < MPT_COMPILER_MAKE_VERSION2((version), (sp)))

#if MPT_MSVC_BEFORE(2017, 9)
#error "MSVC version 2017 15.9 required"
#endif

#if defined(_PREFAST_)
#ifndef MPT_BUILD_ANALYZED
#define MPT_BUILD_ANALYZED
#endif
#endif

#else

#define MPT_COMPILER_GENERIC 1

#endif



#ifndef MPT_COMPILER_GENERIC
#define MPT_COMPILER_GENERIC 0
#endif
#ifndef MPT_COMPILER_CLANG
#define MPT_COMPILER_CLANG                      0
#define MPT_CLANG_AT_LEAST(major, minor, patch) 0
#define MPT_CLANG_BEFORE(major, minor, patch)   0
#endif
#ifndef MPT_COMPILER_GCC
#define MPT_COMPILER_GCC                      0
#define MPT_GCC_AT_LEAST(major, minor, patch) 0
#define MPT_GCC_BEFORE(major, minor, patch)   0
#endif
#ifndef MPT_COMPILER_MSVC
#define MPT_COMPILER_MSVC              0
#define MPT_MSVC_AT_LEAST(version, sp) 0
#define MPT_MSVC_BEFORE(version, sp)   0
#endif



#if MPT_COMPILER_GENERIC

#if (__cplusplus >= 202302)
#define MPT_CXX 23
#elif (__cplusplus >= 202002)
#define MPT_CXX 20
#elif (__cplusplus >= 201703)
#define MPT_CXX 17
#endif

#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG

#if (__cplusplus >= 202302)
#define MPT_CXX 23
#elif (__cplusplus >= 202002)
#if defined(__APPLE__) && MPT_CLANG_BEFORE(13, 0, 0)
// XCode 12.5 has a really weird mix of Clang and libc++. Just black-list C++20 support for XCode <= 12.
#define MPT_CXX 17
#else
#define MPT_CXX 20
#endif
#elif (__cplusplus >= 201703)
#define MPT_CXX 17
#endif

#elif MPT_COMPILER_MSVC

#if MPT_MSVC_AT_LEAST(2015, 3)
#if (_MSVC_LANG >= 202302)
#define MPT_CXX 23
#elif (_MSVC_LANG >= 202002)
#define MPT_CXX 20
#elif (_MSVC_LANG >= 201703)
#define MPT_CXX 17
#endif
#endif

#endif

// default to C++17
#ifndef MPT_CXX
#define MPT_CXX 17
#endif

// MPT_CXX is stricter than just using __cplusplus directly.
// We will only claim a language version as supported IFF all core language and
// library fatures that we need are actually supported AND working correctly
// (to our needs).

#define MPT_CXX_AT_LEAST(version) (MPT_CXX >= (version))
#define MPT_CXX_BEFORE(version)   (MPT_CXX < (version))



// detect compiler quirks

#if MPT_COMPILER_CLANG
#if defined(__APPLE__)
#define MPT_COMPILER_QUIRK_APPLE_CLANG
#endif
#endif



// detect compiler setting quirks

#if MPT_COMPILER_GCC
#if (MPT_GCC_AT_LEAST(14, 0, 0) && MPT_GCC_BEFORE(14, 2, 0)) || (MPT_GCC_AT_LEAST(13, 0, 0) && MPT_GCC_BEFORE(13, 4, 0)) || (MPT_GCC_AT_LEAST(12, 0, 0) && MPT_GCC_BEFORE(12, 5, 0)) || MPT_GCC_BEFORE(12, 0, 0)
// GCC 14 causes severe miscompilation of inline functions on MinGW.
// See <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=115049>.
// Current investigation suggests a general problem with -fipa-ra on non-ELF
// platforms.
// As far as we understand the issue, it could possibly also manifest with
// other inter-procedure-optimizations and with older GCC versions.
// Fixed in GCC 15
// (<https://gcc.gnu.org/git/?p=gcc.git;h=5080840d8fbf25a321dd27543a1462d393d338bc>),
// GCC 14.2
// (<https://gcc.gnu.org/git/?p=gcc.git;h=747c4b58573ea00419f64293a61537eb69f43307>).
// GCC 13.4
// (<https://gcc.gnu.org/git/?p=gcc.git;h=953bf37690d22de956d75c6aef7a9690ad55b9a7>).
// and GCC 12.5
// (<https://gcc.gnu.org/git/?p=gcc.git;h=2c5f48a43f26223cb8603b826d7c0d52cdbcfb46>).
#if !defined(__ELF__)
#define MPT_COMPILER_SETTING_QUIRK_GCC_BROKEN_IPA
#endif
#endif
#endif



#endif // MPT_BASE_DETECT_COMPILER_HPP
