#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

#ifdef WIN32
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <gdiplus.h>
#undef min
#undef max
#undef PlaySound
#undef DrawText
#endif
