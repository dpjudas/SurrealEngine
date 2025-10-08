/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_FORMAT_MESSAGE_MACROS_HPP
#define MPT_FORMAT_MESSAGE_MACROS_HPP



#include "mpt/base/detect.hpp"
#include "mpt/detect/mfc.hpp"
#include "mpt/format/default_formatter.hpp"
#include "mpt/format/message.hpp"



#define MPT_AFORMAT_MESSAGE(f) mpt::format_message<mpt::default_formatter, mpt::parse_format_string_argument_count(f)>(f)

#if !defined(MPT_COMPILER_QUIRK_NO_WCHAR)
#define MPT_WFORMAT_MESSAGE(f) mpt::format_message_typed<mpt::default_formatter, mpt::parse_format_string_argument_count(L##f), std::wstring>(L##f)
#endif // !MPT_COMPILER_QUIRK_NO_WCHAR

#define MPT_UFORMAT_MESSAGE(f) mpt::format_message_typed<mpt::default_formatter, mpt::parse_format_string_argument_count(MPT_ULITERAL(f)), mpt::ustring>(MPT_ULITERAL(f))

#define MPT_LFORMAT_MESSAGE(f) mpt::format_message_typed<mpt::default_formatter, mpt::parse_format_string_argument_count(f), mpt::lstring>(f)

#if MPT_OS_WINDOWS
#define MPT_TFORMAT_MESSAGE(f) mpt::format_message_typed<mpt::default_formatter, mpt::parse_format_string_argument_count(TEXT(f)), mpt::tstring>(TEXT(f))
#endif // MPT_OS_WINDOWS

#if MPT_DETECTED_MFC
#define MPT_CWFORMAT_MESSAGE(f) mpt::format_message_typed<mpt::default_formatter, mpt::parse_format_string_argument_count(L##f), CStringW>(L##f)
#define MPT_CAFORMAT_MESSAGE(f) mpt::format_message_typed<mpt::default_formatter, mpt::parse_format_string_argument_count(f), CStringA>(f)
#define MPT_CFORMAT_MESSAGE(f)  mpt::format_message_typed<mpt::default_formatter, mpt::parse_format_string_argument_count(TEXT(f)), CString>(TEXT(f))
#endif // MPT_DETECTED_MFC



#endif // MPT_FORMAT_MESSAGE_MACROS_HPP
