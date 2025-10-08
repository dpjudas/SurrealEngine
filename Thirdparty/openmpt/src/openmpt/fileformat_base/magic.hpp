/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/macros.hpp"
#include "openmpt/base/Types.hpp"



OPENMPT_NAMESPACE_BEGIN



// Functions to create 4-byte and 2-byte magic byte identifiers in little-endian format
// Use this together with uint32le/uint16le file members.
MPT_CONSTEVAL uint32 MagicLE(const char (&id)[5])
{
	return static_cast<uint32>((static_cast<uint8>(id[3]) << 24) | (static_cast<uint8>(id[2]) << 16) | (static_cast<uint8>(id[1]) << 8) | static_cast<uint8>(id[0]));
}
MPT_CONSTEVAL uint16 MagicLE(const char (&id)[3])
{
	return static_cast<uint16>((static_cast<uint8>(id[1]) << 8) | static_cast<uint8>(id[0]));
}

// Functions to create 4-byte and 2-byte magic byte identifiers in big-endian format
// Use this together with uint32be/uint16be file members.
// Note: Historically, some magic bytes in MPT-specific fields are reversed (due to the use of multi-char literals).
// Such fields turned up reversed in files, so MagicBE is used to keep them readable in the code.
MPT_CONSTEVAL uint32 MagicBE(const char (&id)[5])
{
	return static_cast<uint32>((static_cast<uint8>(id[0]) << 24) | (static_cast<uint8>(id[1]) << 16) | (static_cast<uint8>(id[2]) << 8) | static_cast<uint8>(id[3]));
}
MPT_CONSTEVAL uint16 MagicBE(const char (&id)[3])
{
	return static_cast<uint16>((static_cast<uint8>(id[0]) << 8) | static_cast<uint8>(id[1]));
}



OPENMPT_NAMESPACE_END
