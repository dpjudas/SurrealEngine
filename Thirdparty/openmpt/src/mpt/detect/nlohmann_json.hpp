/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_DETECT_NLOHMANN_JSON_HPP
#define MPT_DETECT_NLOHMANN_JSON_HPP

#if defined(MPT_WITH_NLOHMANN_JSON)
#if !defined(CPPCHECK)
#if !__has_include(<nlohmann/json.hpp>)
#error "MPT_WITH_NLOHMANN_JSON defined but <nlohmann/json.hpp> not found."
#endif
#endif
#define MPT_DETECTED_NLOHMANN_JSON 1
#else
#if defined(CPPCHECK)
#define MPT_DETECTED_NLOHMANN_JSON 1
#else
#if __has_include(<nlohmann/json.hpp>)
#define MPT_DETECTED_NLOHMANN_JSON 1
#else
#define MPT_DETECTED_NLOHMANN_JSON 0
#endif
#endif
#endif

#endif // MPT_DETECT_NLOHMANN_JSON_HPP
