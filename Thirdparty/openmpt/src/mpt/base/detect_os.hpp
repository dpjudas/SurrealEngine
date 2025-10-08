/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_DETECT_OS_HPP
#define MPT_BASE_DETECT_OS_HPP



#define MPT_WIN_MAKE_VERSION(major, minor, sp, build) ((major << 24) + (minor << 16) + (sp << 8) + (build << 0))

// clang-format off

#define MPT_WIN_WIN32S   MPT_WIN_MAKE_VERSION(0x03, 0x00, 0x00, 0x00)

#define MPT_WIN_WIN95    MPT_WIN_MAKE_VERSION(0x04, 0x00, 0x00, 0x00)
#define MPT_WIN_WIN98    MPT_WIN_MAKE_VERSION(0x04, 0x10, 0x00, 0x00)
#define MPT_WIN_WINME    MPT_WIN_MAKE_VERSION(0x04, 0x90, 0x00, 0x00)

#define MPT_WIN_NT3      MPT_WIN_MAKE_VERSION(0x03, 0x00, 0x00, 0x00)
#define MPT_WIN_NT4      MPT_WIN_MAKE_VERSION(0x04, 0x00, 0x00, 0x00)
#define MPT_WIN_2000     MPT_WIN_MAKE_VERSION(0x05, 0x00, 0x00, 0x00)
#define MPT_WIN_2000SP1  MPT_WIN_MAKE_VERSION(0x05, 0x00, 0x01, 0x00)
#define MPT_WIN_2000SP2  MPT_WIN_MAKE_VERSION(0x05, 0x00, 0x02, 0x00)
#define MPT_WIN_2000SP3  MPT_WIN_MAKE_VERSION(0x05, 0x00, 0x03, 0x00)
#define MPT_WIN_2000SP4  MPT_WIN_MAKE_VERSION(0x05, 0x00, 0x04, 0x00)
#define MPT_WIN_XP       MPT_WIN_MAKE_VERSION(0x05, 0x01, 0x00, 0x00)
#define MPT_WIN_XPSP1    MPT_WIN_MAKE_VERSION(0x05, 0x01, 0x01, 0x00)
#define MPT_WIN_XPSP2    MPT_WIN_MAKE_VERSION(0x05, 0x01, 0x02, 0x00)
#define MPT_WIN_XPSP3    MPT_WIN_MAKE_VERSION(0x05, 0x01, 0x03, 0x00)
#define MPT_WIN_XPSP4    MPT_WIN_MAKE_VERSION(0x05, 0x01, 0x04, 0x00) // unused
#define MPT_WIN_XP64     MPT_WIN_MAKE_VERSION(0x05, 0x02, 0x00, 0x00) // unused
#define MPT_WIN_XP64SP1  MPT_WIN_MAKE_VERSION(0x05, 0x02, 0x01, 0x00)
#define MPT_WIN_XP64SP2  MPT_WIN_MAKE_VERSION(0x05, 0x02, 0x02, 0x00)
#define MPT_WIN_XP64SP3  MPT_WIN_MAKE_VERSION(0x05, 0x02, 0x03, 0x00) // unused
#define MPT_WIN_XP64SP4  MPT_WIN_MAKE_VERSION(0x05, 0x02, 0x04, 0x00) // unused
#define MPT_WIN_VISTA    MPT_WIN_MAKE_VERSION(0x06, 0x00, 0x00, 0x00)
#define MPT_WIN_VISTASP1 MPT_WIN_MAKE_VERSION(0x06, 0x00, 0x01, 0x00)
#define MPT_WIN_VISTASP2 MPT_WIN_MAKE_VERSION(0x06, 0x00, 0x02, 0x00)
#define MPT_WIN_VISTASP3 MPT_WIN_MAKE_VERSION(0x06, 0x00, 0x03, 0x00) // unused
#define MPT_WIN_VISTASP4 MPT_WIN_MAKE_VERSION(0x06, 0x00, 0x04, 0x00) // unused
#define MPT_WIN_7        MPT_WIN_MAKE_VERSION(0x06, 0x01, 0x00, 0x00)
#define MPT_WIN_8        MPT_WIN_MAKE_VERSION(0x06, 0x02, 0x00, 0x00)
#define MPT_WIN_81       MPT_WIN_MAKE_VERSION(0x06, 0x03, 0x00, 0x00)

// Now, this is some very elaborate serious clusterfuck with basically no documentation.

// https://blog.yuo.be/2025/02/12/what-do-all-the-recent-ntddi-version-values-mean/
// https://learn.microsoft.com/en-gb/windows/win32/winprog/using-the-windows-headers#macros-for-conditional-declarations
// https://en.wikipedia.org/wiki/Windows_10_version_history
// https://en.wikipedia.org/wiki/Windows_11_version_history
// https://www.windowslatest.com/2021/09/03/windows-11-build-22449-is-now-available-with-new-loading-animation/

//      Release          NTDDI-version                                   NTDDI-constant  Release     Build.Sub  Codename      Semester     Semester Semester Marketing-Name    min-SDK-Version WDK_NTDDI_VERSION
//                                                                                            .Version                                     Codename  Date

#define MPT_WIN_10_PRE   MPT_WIN_MAKE_VERSION(0x06, 0x04, 0x00, 0x00) //                                                                                     Threshold            -            -
#define MPT_WIN_10       MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x00) // NTDDI_WIN10      Win10.1507 10240      Threshold                            1507    -                    10.0.19041.0 NTDDI_WIN10_VB
#define MPT_WIN_10_1511  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x01) // NTDDI_WIN10_TH2  Win10.1511 10586      Threshold 2                          1511    November Update      10.0.19041.0 NTDDI_WIN10_VB
#define MPT_WIN_10_1607  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x02) // NTDDI_WIN10_RS1  Win10.1607 14393      Redstone                             1607    Anniversary Update   10.0.19041.0 NTDDI_WIN10_VB
#define MPT_WIN_10_1703  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x03) // NTDDI_WIN10_RS2  Win10.1703 15063      Redstone 2                           1703    Creators Update      10.0.19041.0 NTDDI_WIN10_VB
#define MPT_WIN_10_1709  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x04) // NTDDI_WIN10_RS3  Win10.1709 16299      Redstone 3                           1709    Fall Creators Update 10.0.19041.0 NTDDI_WIN10_VB
#define MPT_WIN_10_1803  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x05) // NTDDI_WIN10_RS4  Win10.1803 17134      Redstone 4                           1803    April 2018 Update    10.0.19041.0 NTDDI_WIN10_VB
#define MPT_WIN_10_1809  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x06) // NTDDI_WIN10_RS5  Win10.1809 17763      Redstone 5                           1809    October 2018 Update  10.0.19041.0 NTDDI_WIN10_VB
#define MPT_WIN_10_1903  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x07) // NTDDI_WIN10_19H1 Win10.1903 18362      Titanium R1                Titanium  19H1    May 2019 Update      10.0.19041.0 NTDDI_WIN10_VB
//      MPT_WIN_10_1909  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x07) //                  Win10.1909 18363      Titanium R2                Vanadium  19H2    November 2019 Update 10.0.19041.0 NTDDI_WIN10_VB
#define MPT_WIN_10_2004  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x08) // NTDDI_WIN10_VB   Win10.2004 19041      Vibranium R1               Vibranium 20H1    May 2020 Update      10.0.19041.0 NTDDI_WIN10_VB
//      MPT_WIN_10_20H2  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x08) //                  Win10.20H2 19042      Vibranium R2               Manganese 20H2    October 2020 Update  10.0.19041.0 NTDDI_WIN10_VB
//      MPT_WIN_10_21H1  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x08) //                  Win10.21H1 19043      Vibranium R3               Iron      21H1    May 2021 Update      10.0.19041.0 NTDDI_WIN10_VB
//      MPT_WIN_10_21H2  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x08) //                  Win10.21H2 19044      Vibranium R4               Cobalt    21H2    November 2021 Update 10.0.19041.0 NTDDI_WIN10_VB
//      MPT_WIN_10_22H2  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x08) //                  Win10.22H2 19045      Vibranium R5               Nickel    22H2    2022 Update          10.0.19041.0 NTDDI_WIN10_VB

//                       MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x09) // NTDDI_WIN10_MN         20H2                                       Manganese 20H2                         10.0.22621.? NTDDI_WIN10_NI
//                       MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x0a) // NTDDI_WIN10_FE         21H1                                       Iron      21H1                         10.0.22621.? NTDDI_WIN10_NI
#define MPT_WIN_11       MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x0b) // NTDDI_WIN10_CO   Win11.21H2 22000      Sun Valley    Cobalt       Cobalt    21H2    -                    10.0.22621.? NTDDI_WIN10_NI
#define MPT_WIN_11_22H2  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x0c) // NTDDI_WIN10_NI   Win11.22H2 22621      Sun Valley 2  Nickel R1    Nickel    22H2    2022 Update          10.0.22621.? NTDDI_WIN10_NI
//                       MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x0c) //                  Win11.22H2 22621.675  Sun Valley 2  Nickel R1    Nickel    22H2    2022 Update Moment 1 10.0.22621.? NTDDI_WIN10_NI
//                       MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x0c) //                  Win11.22H2 22621.1344 Sun Valley 2  Nickel R1    Nickel    22H2    2022 Update Moment 2 10.0.22621.? NTDDI_WIN10_NI
//                       MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x0c) //                  Win11.22H2 22621.1778 Sun Valley 2  Nickel R1    Nickel    22H2    2022 Update Moment 3 10.0.22621.? NTDDI_WIN10_NI
//                       MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x0c) //                  Win11.22H2 22621.2361 Sun Valley 2  Nickel R1    Nickel    22H2    2022 Update Moment 4 10.0.22621.? NTDDI_WIN10_NI
//                       MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x0c) //                  Win11.22H2 22621.3235 Sun Valley 2  Nickel R1    Nickel    22H2    2022 Update Moment 5 10.0.22621.? NTDDI_WIN10_NI
//      MPT_WIN_11_23H2  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x0c) //                  Win11.23H2 22631      Sun Valley 3  Nickel R2    Zinc      23H2    2023 Update          10.0.22621.? NTDDI_WIN10_NI
//                       MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x0c) //                  Win11.23H2 22631.3235 Sun Valley 3  Nickel R2    Zinc      23H2    2023 Update Moment 5 10.0.22621.? NTDDI_WIN10_NI
//                       MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x0d) // NTDDI_WIN10_CU         23H1                                       Copper    23H1                         10.0.26100.? NTDDI_WIN11_GE
//                       MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x0e) // NTDDI_WIN11_ZN         23H2                                       Zinc      23H2                         10.0.26100.? NTDDI_WIN11_GE
//                       MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x0f) // NTDDI_WIN11_GA         24H1                                       Gallium   24H1                         10.0.26100.? NTDDI_WIN11_GE
#define MPT_WIN_11_24H2  MPT_WIN_MAKE_VERSION(0x0a, 0x00, 0x00, 0x10) // NTDDI_WIN11_GE   Win11.24H2 26100      Hudson Valley Germanium R1 Germanium 24H2    2024 Update          10.0.26100.? NTDDI_WIN11_GE

// MPT_WIN_API_DESKTOP     : Windows 8/10 Desktop Application (Win32)
// MPT_WIN_API_UNIVERSAL   : Windows 10 Store App / Universal App
// MPT_WIN_API_STORE_PC    : Windows 8 Store Desktop App
// MPT_WIN_API_STORE_PHONE : Windows 8 Store Phone App

// clang-format on



// The order of the checks matters!
#if defined(__DJGPP__)
#define MPT_OS_DJGPP 1


#elif defined(__EMSCRIPTEN__)
#define MPT_OS_EMSCRIPTEN 1
#if !defined(__EMSCRIPTEN_major__) || !defined(__EMSCRIPTEN_minor__) || !defined(__EMSCRIPTEN_tiny__)
#include <emscripten/version.h>
#endif
#if defined(__EMSCRIPTEN_major__) && defined(__EMSCRIPTEN_minor__)
#if (__EMSCRIPTEN_major__ > 3)
// ok
#elif (__EMSCRIPTEN_major__ == 3) && (__EMSCRIPTEN_minor__ > 1)
// ok
#elif (__EMSCRIPTEN_major__ == 3) && (__EMSCRIPTEN_minor__ == 1) && (__EMSCRIPTEN_tiny__ >= 51)
// ok
#else
#error "Emscripten >= 3.1.51 is required."
#endif
#endif


#elif defined(__CYGWIN__)
#define MPT_OS_CYGWIN 1


#elif defined(_WIN32)
#define MPT_OS_WINDOWS 1
#if !defined(_WIN32_WINDOWS) && !defined(WINVER)
// include modern SDK version header if not targeting Win9x
#include <sdkddkver.h>
#ifdef _WIN32_WINNT_NT4
static_assert((_WIN32_WINNT_NT4 << 16) == MPT_WIN_NT4);
#endif
#ifdef _WIN32_WINNT_WIN2K
static_assert((_WIN32_WINNT_WIN2K << 16) == MPT_WIN_2000);
#endif
#ifdef _WIN32_WINNT_WINXP
static_assert((_WIN32_WINNT_WINXP << 16) == MPT_WIN_XP);
#endif
#ifdef _WIN32_WINNT_WS03
static_assert((_WIN32_WINNT_WS03 << 16) == MPT_WIN_XP64);
#endif
#ifdef _WIN32_WINNT_VISTA
static_assert((_WIN32_WINNT_VISTA << 16) == MPT_WIN_VISTA);
#endif
#ifdef _WIN32_WINNT_WIN7
static_assert((_WIN32_WINNT_WIN7 << 16) == MPT_WIN_7);
#endif
#ifdef _WIN32_WINNT_WIN8
static_assert((_WIN32_WINNT_WIN8 << 16) == MPT_WIN_8);
#endif
#ifdef _WIN32_WINNT_WINBLUE
static_assert((_WIN32_WINNT_WINBLUE << 16) == MPT_WIN_81);
#endif
#ifdef _WIN32_WINNT_WIN10
static_assert((_WIN32_WINNT_WIN10 << 16) == MPT_WIN_10);
#endif
#ifdef NTDDI_WIN4
static_assert(NTDDI_WIN4 == MPT_WIN_NT4);
#endif
#ifdef NTDDI_WIN2K
static_assert(NTDDI_WIN2K == MPT_WIN_2000);
#endif
#ifdef NTDDI_WIN2KSP1
static_assert(NTDDI_WIN2KSP1 == MPT_WIN_2000SP1);
#endif
#ifdef NTDDI_WIN2KSP2
static_assert(NTDDI_WIN2KSP2 == MPT_WIN_2000SP2);
#endif
#ifdef NTDDI_WIN2KSP3
static_assert(NTDDI_WIN2KSP3 == MPT_WIN_2000SP3);
#endif
#ifdef NTDDI_WIN2KSP4
static_assert(NTDDI_WIN2KSP4 == MPT_WIN_2000SP4);
#endif
#ifdef NTDDI_WINXP
static_assert(NTDDI_WINXP == MPT_WIN_XP);
#endif
#ifdef NTDDI_WINXPSP1
static_assert(NTDDI_WINXPSP1 == MPT_WIN_XPSP1);
#endif
#ifdef NTDDI_WINXPSP2
static_assert(NTDDI_WINXPSP2 == MPT_WIN_XPSP2);
#endif
#ifdef NTDDI_WINXPSP3
static_assert(NTDDI_WINXPSP3 == MPT_WIN_XPSP3);
#endif
#ifdef NTDDI_WINXPSP4
static_assert(NTDDI_WINXPSP4 == MPT_WIN_XPSP4);
#endif
#ifdef NTDDI_WS03
static_assert(NTDDI_WS03 == MPT_WIN_XP64);
#endif
#ifdef NTDDI_WS03SP1
static_assert(NTDDI_WS03SP1 == MPT_WIN_XP64SP1);
#endif
#ifdef NTDDI_WS03SP2
static_assert(NTDDI_WS03SP2 == MPT_WIN_XP64SP2);
#endif
#ifdef NTDDI_WS03SP3
static_assert(NTDDI_WS03SP3 == MPT_WIN_XP64SP3);
#endif
#ifdef NTDDI_WS03SP4
static_assert(NTDDI_WS03SP4 == MPT_WIN_XP64SP4);
#endif
#ifdef NTDDI_VISTA
static_assert(NTDDI_VISTA == MPT_WIN_VISTA);
#endif
#ifdef NTDDI_VISTASP1
static_assert(NTDDI_VISTASP1 == MPT_WIN_VISTASP1);
#endif
#ifdef NTDDI_VISTASP2
static_assert(NTDDI_VISTASP2 == MPT_WIN_VISTASP2);
#endif
#ifdef NTDDI_VISTASP3
static_assert(NTDDI_VISTASP3 == MPT_WIN_VISTASP3);
#endif
#ifdef NTDDI_VISTASP4
static_assert(NTDDI_VISTASP4 == MPT_WIN_VISTASP4);
#endif
#ifdef NTDDI_WIN7
static_assert(NTDDI_WIN7 == MPT_WIN_7);
#endif
#ifdef NTDDI_WIN8
static_assert(NTDDI_WIN8 == MPT_WIN_8);
#endif
#ifdef NTDDI_WINBLUE
static_assert(NTDDI_WINBLUE == MPT_WIN_81);
#endif
#ifdef NTDDI_WIN10
static_assert(NTDDI_WIN10 == MPT_WIN_10);
#endif
#ifdef NTDDI_WIN10_TH2
static_assert(NTDDI_WIN10_TH2 == MPT_WIN_10_1511);
#endif
#ifdef NTDDI_WIN10_RS1
static_assert(NTDDI_WIN10_RS1 == MPT_WIN_10_1607);
#endif
#ifdef NTDDI_WIN10_RS2
static_assert(NTDDI_WIN10_RS2 == MPT_WIN_10_1703);
#endif
#ifdef NTDDI_WIN10_RS3
static_assert(NTDDI_WIN10_RS3 == MPT_WIN_10_1709);
#endif
#ifdef NTDDI_WIN10_RS4
static_assert(NTDDI_WIN10_RS4 == MPT_WIN_10_1803);
#endif
#ifdef NTDDI_WIN10_RS5
static_assert(NTDDI_WIN10_RS5 == MPT_WIN_10_1809);
#endif
#ifdef NTDDI_WIN10_19H1
static_assert(NTDDI_WIN10_19H1 == MPT_WIN_10_1903);
#endif
//#ifdef NTDDI_WIN10_19H1
//static_assert(NTDDI_WIN10_19H1 == MPT_WIN_10_1909);
//#endif
#ifdef NTDDI_WIN10_VB
static_assert(NTDDI_WIN10_VB == MPT_WIN_10_2004);
#endif
//#ifdef NTDDI_WIN10_VB
//static_assert(NTDDI_WIN10_VB == MPT_WIN_10_20H2);
//#endif
//#ifdef NTDDI_WIN10_VB
//static_assert(NTDDI_WIN10_VB == MPT_WIN_10_21H1);
//#endif
//#ifdef NTDDI_WIN10_VB
//static_assert(NTDDI_WIN10_VB == MPT_WIN_10_21H2);
//#endif
//#ifdef NTDDI_WIN10_VB
//static_assert(NTDDI_WIN10_VB == MPT_WIN_10_22H2);
//#endif
//#ifdef NTDDI_WIN10_MN
//static_assert(NTDDI_WIN10_MN == ???);
//#endif
//#ifdef NTDDI_WIN10_FE
//static_assert(NTDDI_WIN10_FE == ???);
//#endif
#ifdef NTDDI_WIN10_CO
static_assert(NTDDI_WIN10_CO == MPT_WIN_11);
#endif
#ifdef NTDDI_WIN10_NI
static_assert(NTDDI_WIN10_NI == MPT_WIN_11_22H2);
#endif
//#ifdef NTDDI_WIN10_NI
//static_assert(NTDDI_WIN10_NI == MPT_WIN_11_23H2);
//#endif
//#ifdef NTDDI_WIN10_CU
//static_assert(NTDDI_WIN10_CU == ???);
//#endif
//#ifdef NTDDI_WIN10_ZN
//static_assert(NTDDI_WIN10_ZN == ???);
//#endif
//#ifdef NTDDI_WIN11_GA
//static_assert(NTDDI_WIN11_GA == ???);
//#endif
#ifdef NTDDI_WIN11_GE
static_assert(NTDDI_WIN11_GE == MPT_WIN_11_24H2);
#endif
#endif
#if defined(WINAPI_FAMILY)
#include <winapifamily.h>
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define MPT_OS_WINDOWS_WINRT 0
#else
#define MPT_OS_WINDOWS_WINRT 1
#endif
#define MPT_WIN_API_DESKTOP     WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define MPT_WIN_API_UNIVERSAL   WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PC_APP)
#define MPT_WIN_API_STORE_PC    WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#define MPT_WIN_API_STORE_PHONE WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE_APP)
#else // !WINAPI_FAMILY
#define MPT_OS_WINDOWS_WINRT    0
#define MPT_WIN_API_DESKTOP     1
#define MPT_WIN_API_UNIVERSAL   0
#define MPT_WIN_API_STORE_PC    0
#define MPT_WIN_API_STORE_PHONE 0
#endif // WINAPI_FAMILY
#if defined(NTDDI_VERSION) || defined(_WIN32_WINNT)
#define MPT_OS_WINDOWS_WINNT 1
#define MPT_OS_WINDOWS_WIN9X 0
#define MPT_OS_WINDOWS_WIN32 0
#if defined(NTDDI_VERSION)
#define MPT_WIN_VERSION NTDDI_VERSION
#else
#define MPT_WIN_VERSION (_WIN32_WINNT << 16)
#endif
#elif defined(_WIN32_WINDOWS)
#define MPT_OS_WINDOWS_WINNT 0
#define MPT_OS_WINDOWS_WIN9X 1
#define MPT_OS_WINDOWS_WIN32 0
#define MPT_WIN_VERSION      (_WIN32_WINDOWS << 16)
#elif defined(WINVER)
#define MPT_OS_WINDOWS_WINNT 0
#define MPT_OS_WINDOWS_WIN9X 0
#define MPT_OS_WINDOWS_WIN32 1
#define MPT_WIN_VERSION      (WINVER << 16)
#else
// assume modern
#define MPT_OS_WINDOWS_WINNT 1
#define MPT_OS_WINDOWS_WIN9X 0
#define MPT_OS_WINDOWS_WIN32 0
#define MPT_WIN_VERSION      MPT_WIN_NT4
#endif
#define MPT_WINRT_AT_LEAST(v) (MPT_OS_WINDOWS_WINRT && MPT_OS_WINDOWS_WINNT && (MPT_WIN_VERSION >= (v)))
#define MPT_WINRT_BEFORE(v)   (MPT_OS_WINDOWS_WINRT && MPT_OS_WINDOWS_WINNT && (MPT_WIN_VERSION < (v)))
#define MPT_WINNT_AT_LEAST(v) (MPT_OS_WINDOWS_WINNT && (MPT_WIN_VERSION >= (v)))
#define MPT_WINNT_BEFORE(v)   (MPT_OS_WINDOWS_WINNT && (MPT_WIN_VERSION < (v)))
#define MPT_WIN9X_AT_LEAST(v) ((MPT_OS_WINDOWS_WINNT || MPT_OS_WINDOWS_WIN9X) && (MPT_WIN_VERSION >= (v)))
#define MPT_WIN9X_BEFORE(v)   ((MPT_OS_WINDOWS_WINNT || MPT_OS_WINDOWS_WIN9X) && (MPT_WIN_VERSION < (v)))
#define MPT_WIN32_AT_LEAST(v) ((MPT_OS_WINDOWS_WINNT || MPT_OS_WINDOWS_WIN9X || MPT_OS_WINDOWS_WIN32) && (MPT_WIN_VERSION >= (v)))
#define MPT_WIN32_BEFORE(v)   ((MPT_OS_WINDOWS_WINNT || MPT_OS_WINDOWS_WIN9X || MPT_OS_WINDOWS_WIN32) && (MPT_WIN_VERSION < (v)))
#if MPT_OS_WINDOWS_WINRT
#define MPT_WIN_AT_LEAST(v) MPT_WINRT_AT_LEAST(v)
#define MPT_WIN_BEFORE(v)   MPT_WINRT_BEFORE(v)
#elif MPT_OS_WINDOWS_WINNT
#define MPT_WIN_AT_LEAST(v) MPT_WINNT_AT_LEAST(v)
#define MPT_WIN_BEFORE(v)   MPT_WINNT_BEFORE(v)
#elif MPT_OS_WINDOWS_WIN9X
#define MPT_WIN_AT_LEAST(v) MPT_WIN9X_AT_LEAST(v)
#define MPT_WIN_BEFORE(v)   MPT_WIN9X_BEFORE(v)
#elif MPT_OS_WINDOWS_WIN32
#define MPT_WIN_AT_LEAST(v) MPT_WIN32_AT_LEAST(v)
#define MPT_WIN_BEFORE(v)   MPT_WIN32_BEFORE(v)
#else
#define MPT_WIN_AT_LEAST(v) 0
#define MPT_WIN_BEFORE(v)   1
#endif
#if (MPT_OS_WINDOWS_WINNT || MPT_OS_WINDOWS_WINRT) && defined(WDK_NTDDI_VERSION)
static_assert(WDK_NTDDI_VERSION >= MPT_WIN_VERSION);
#endif


#elif defined(__APPLE__)
#define MPT_OS_MACOSX_OR_IOS 1
#include <TargetConditionals.h>
#if defined(TARGET_OS_OSX)
#if (TARGET_OS_OSX != 0)
#include <AvailabilityMacros.h>
#endif
#endif
//#if TARGET_IPHONE_SIMULATOR
//#elif TARGET_OS_IPHONE
//#elif TARGET_OS_MAC
//#else
//#endif


#elif defined(__HAIKU__)
#define MPT_OS_HAIKU 1


#elif defined(__ANDROID__) || defined(ANDROID)
#define MPT_OS_ANDROID 1


#elif defined(__linux__)
#define MPT_OS_LINUX 1


#elif defined(__DragonFly__)
#define MPT_OS_DRAGONFLYBSD 1


#elif defined(__FreeBSD__)
#define MPT_OS_FREEBSD 1


#elif defined(__OpenBSD__)
#define MPT_OS_OPENBSD 1


#elif defined(__NetBSD__)
#define MPT_OS_NETBSD 1


#elif defined(__unix__)
#define MPT_OS_GENERIC_UNIX 1


#else
#define MPT_OS_UNKNOWN 1


#endif


#ifndef MPT_OS_DJGPP
#define MPT_OS_DJGPP 0
#endif
#ifndef MPT_OS_EMSCRIPTEN
#define MPT_OS_EMSCRIPTEN 0
#endif
#ifndef MPT_OS_CYGWIN
#define MPT_OS_CYGWIN 0
#endif
#ifndef MPT_OS_WINDOWS
#define MPT_OS_WINDOWS 0
#endif
#ifndef MPT_OS_WINDOWS_WINRT
#define MPT_OS_WINDOWS_WINRT 0
#endif
#ifndef MPT_OS_WINDOWS_WINNT
#define MPT_OS_WINDOWS_WINNT 0
#endif
#ifndef MPT_OS_WINDOWS_WIN9X
#define MPT_OS_WINDOWS_WIN9X 0
#endif
#ifndef MPT_OS_WINDOWS_WIN32
#define MPT_OS_WINDOWS_WIN32 0
#endif
#ifndef MPT_WINRT_AT_LEAST
#define MPT_WINRT_AT_LEAST(v) 0
#endif
#ifndef MPT_WINRT_BEFORE
#define MPT_WINRT_BEFORE(v) 0
#endif
#ifndef MPT_WINNT_AT_LEAST
#define MPT_WINNT_AT_LEAST(v) 0
#endif
#ifndef MPT_WINNT_BEFORE
#define MPT_WINNT_BEFORE(v) 0
#endif
#ifndef MPT_WIN9X_AT_LEAST
#define MPT_WIN9X_AT_LEAST(v) 0
#endif
#ifndef MPT_WIN9X_BEFORE
#define MPT_WIN9X_BEFORE(v) 0
#endif
#ifndef MPT_WIN32_AT_LEAST
#define MPT_WIN32_AT_LEAST(v) 0
#endif
#ifndef MPT_WIN32_BEFORE
#define MPT_WIN32_BEFORE(v) 0
#endif
#ifndef MPT_WIN_AT_LEAST
#define MPT_WIN_AT_LEAST(v) 0
#endif
#ifndef MPT_WIN_BEFORE
#define MPT_WIN_BEFORE(v) 0
#endif
#ifndef MPT_WIN_API_DESKTOP
#define MPT_WIN_API_DESKTOP 0
#endif
#ifndef MPT_WIN_API_UNIVERSAL
#define MPT_WIN_API_UNIVERSAL 0
#endif
#ifndef MPT_WIN_API_STORE_PC
#define MPT_WIN_API_STORE_PC 0
#endif
#ifndef MPT_WIN_API_STORE_PHONE
#define MPT_WIN_API_STORE_PHONE 0
#endif
#ifndef MPT_OS_MACOSX_OR_IOS
#define MPT_OS_MACOSX_OR_IOS 0
#endif
#ifndef MPT_OS_HAIKU
#define MPT_OS_HAIKU 0
#endif
#ifndef MPT_OS_ANDROID
#define MPT_OS_ANDROID 0
#endif
#ifndef MPT_OS_LINUX
#define MPT_OS_LINUX 0
#endif
#ifndef MPT_OS_DRAGONFLYBSD
#define MPT_OS_DRAGONFLYBSD 0
#endif
#ifndef MPT_OS_FREEBSD
#define MPT_OS_FREEBSD 0
#endif
#ifndef MPT_OS_OPENBSD
#define MPT_OS_OPENBSD 0
#endif
#ifndef MPT_OS_NETBSD
#define MPT_OS_NETBSD 0
#endif
#ifndef MPT_OS_GENERIC_UNIX
#define MPT_OS_GENERIC_UNIX 0
#endif
#ifndef MPT_OS_UNKNOWN
#define MPT_OS_UNKNOWN 0
#endif



#define MPT_MODE_KERNEL 0



#endif // MPT_BASE_DETECT_OS_HPP
