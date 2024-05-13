#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Exception.h"

#ifdef WIN32
#define WIN32_MEAN_AND_LEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>
#include <gdiplus.h>
#undef min
#undef max
#undef PlaySound
#undef DrawText
#endif

#if (defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))) || ((defined(__i386__) || defined(__x86_64__) || defined(__e2k__)) && defined(__SSE2__))
#else
#define NOSSE
#endif
