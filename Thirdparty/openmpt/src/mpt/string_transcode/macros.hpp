/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_STRING_TRANSCODE_MACROS_HPP
#define MPT_STRING_TRANSCODE_MACROS_HPP



#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include <string>



// The MPT_UTF8_STRING allows specifying UTF8 char arrays.
// The resulting type is mpt::ustring and the construction might require runtime translation,
// i.e. it is NOT generally available at compile time.
// Use explicit UTF8 encoding,
// i.e. U+00FC (LATIN SMALL LETTER U WITH DIAERESIS) would be written as "\xC3\xBC".
#define MPT_UTF8_STRING(x) mpt::transcode<mpt::ustring>(mpt::common_encoding::utf8, std::string{x})



#endif // MPT_STRING_TRANSCODE_MACROS_HPP
