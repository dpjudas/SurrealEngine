/*
 * mod_specifications.cpp
 * ----------------------
 * Purpose: Mod specifications characterise the features of every editable module format in OpenMPT, such as the number of supported channels, samples, effects, etc...
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "mod_specifications.h"
#include "../common/misc_util.h"
#include <algorithm>

OPENMPT_NAMESPACE_BEGIN

namespace ModSpecs
{


constexpr CModSpecifications mptm_ =
{
	/*
	TODO: Proper, less arbitrarily chosen values here.
	NOTE: If changing limits, see whether:
			-savefile format and GUI methods can handle new values(might not be a small task :).
	 */
	MOD_TYPE_MPT,								// Internal MODTYPE value
	"mptm",										// File extension
	NOTE_MIN,									// Minimum note index
	NOTE_MIN + 119,								// Maximum note index
	4000,										// Pattern max.
	4000,										// Order max.
	MAX_SEQUENCES,								// Sequences max
	1,											// Channel min
	127,										// Channel max
	32,											// Min tempo
	1000,										// Max tempo
	1,											// Min Speed
	255,										// Max Speed
	1,											// Min pattern rows
	1024,										// Max pattern rows
	25,											// Max mod name length
	25,											// Max sample name length
	12,											// Max sample filename length
	25,											// Max instrument name length
	12,											// Max instrument filename length
	0,											// Max comment line length
	3999,										// SamplesMax
	255,										// instrumentMax
	MixLevels::v1_17RC3,						// defaultMixLevels
	SONG_LINEARSLIDES | SONG_EXFILTERRANGE | SONG_ITOLDEFFECTS | SONG_ITCOMPATGXX,	// Supported song flags
	200,										// Max MIDI mapping directives
	MAX_ENVPOINTS,								// Envelope point count
	true,										// Has notecut.
	true,										// Has noteoff.
	true,										// Has notefade.
	true,										// Has envelope release node
	true,										// Has song comments
	true,										// Has "+++" pattern
	true,										// Has "---" pattern
	true,										// Has restart position (order)
	true,										// Supports plugins
	true,										// Custom pattern time signatures
	true,										// Pattern names
	true,										// Has artist name
	true,										// Has default resampling
	true,										// Fixed point tempo
	" JFEGHLKRXODB?CQATI?SMNVW?UY?P?Z\\:#+*?????????????????????",	// Supported Effects
	" vpcdabuh??gfe?o",							// Supported Volume Column commands
};

constexpr CModSpecifications mod_ =
{
	MOD_TYPE_MOD,								// Internal MODTYPE value
	"mod",										// File extension
	25,											// Minimum note index
	108,										// Maximum note index
	128,										// Pattern max.
	128,										// Order max.
	1,											// Only one order list
	1,											// Channel min
	99,											// Channel max
	32,											// Min tempo
	255,										// Max tempo
	1,											// Min Speed
	31,											// Max Speed
	64,											// Min pattern rows
	64,											// Max pattern rows
	20,											// Max mod name length
	22,											// Max sample name length
	0,											// Max sample filename length
	0,											// Max instrument name length
	0,											// Max instrument filename length
	0,											// Max comment line length
	31,											// SamplesMax
	0,											// instrumentMax
	MixLevels::Compatible,						// defaultMixLevels
	SONG_PT_MODE | SONG_AMIGALIMITS | SONG_ISAMIGA | SONG_FORMAT_NO_VOLCOL,  // Supported song flags
	0,											// Max MIDI mapping directives
	0,											// No instrument envelopes
	false,										// No notecut.
	false,										// No noteoff.
	false,										// No notefade.
	false,										// No envelope release node
	false,										// No song comments
	false,										// Doesn't have "+++" pattern
	false,										// Doesn't have "---" pattern
	true,										// Has restart position (order)
	false,										// Doesn't support plugins
	false,										// No custom pattern time signatures
	false,										// No pattern names
	false,										// Doesn't have artist name
	false,										// Doesn't have default resampling
	false,										// Integer tempo
	" 0123456789ABCD?FF?E??????????????????????????????????????",	// Supported Effects
	" ???????????????",							// Supported Volume Column commands
};

constexpr CModSpecifications xm_ =
{
	MOD_TYPE_XM,								// Internal MODTYPE value
	"xm",										// File extension
	13,											// Minimum note index
	108,										// Maximum note index
	256,										// Pattern max.
	255,										// Order max.
	1,											// Only one order list
	1,											// Channel min
	32,											// Channel max
	32,											// Min tempo
	1000,										// Max tempo
	1,											// Min Speed
	31,											// Max Speed
	1,											// Min pattern rows
	256,										// Max pattern rows
	20,											// Max mod name length
	22,											// Max sample name length
	0,											// Max sample filename length
	22,											// Max instrument name length
	0,											// Max instrument filename length
	0,											// Max comment line length
	128 * 16,									// SamplesMax (actually 16 per instrument)
	128,										// instrumentMax
	MixLevels::CompatibleFT2,					// defaultMixLevels
	SONG_LINEARSLIDES,							// Supported song flags
	0,											// Max MIDI mapping directives
	12,											// Envelope point count
	false,										// No notecut.
	true,										// Has noteoff.
	false,										// No notefade.
	false,										// No envelope release node
	false,										// No song comments
	false,										// Doesn't have "+++" pattern
	false,										// Doesn't have "---" pattern
	true,										// Has restart position (order)
	false,										// Doesn't support plugins
	false,										// No custom pattern time signatures
	false,										// No pattern names
	false,										// Doesn't have artist name
	false,										// Doesn't have default resampling
	false,										// Integer tempo
	" 0123456789ABCDRFFTE???GHK??XPL??????W????????????????????",	// Supported Effects
	" vpcdabuhlrg????",							// Supported Volume Column commands
};

// XM with MPT extensions
constexpr CModSpecifications xmEx_ =
{
	MOD_TYPE_XM,								// Internal MODTYPE value
	"xm",										// File extension
	13,											// Minimum note index
	108,										// Maximum note index
	256,										// Pattern max.
	255,										// Order max.
	1,											// Only one order list
	1,											// Channel min
	128,										// Channel max
	32,											// Min tempo
	1000,										// Max tempo
	1,											// Min Speed
	31,											// Max Speed
	1,											// Min pattern rows
	1024,										// Max pattern rows
	20,											// Max mod name length
	22,											// Max sample name length
	0,											// Max sample filename length
	22,											// Max instrument name length
	0,											// Max instrument filename length
	0,											// Max comment line length
	MAX_SAMPLES - 1,							// SamplesMax (actually 32 per instrument(256 * 32 = 8192), but limited to MAX_SAMPLES = 4000)
	255,										// instrumentMax
	MixLevels::CompatibleFT2,					// defaultMixLevels
	SONG_LINEARSLIDES | SONG_EXFILTERRANGE,		// Supported song flags
	200,										// Max MIDI mapping directives
	12,											// Envelope point count
	false,										// No notecut.
	true,										// Has noteoff.
	false,										// No notefade.
	false,										// No envelope release node
	true,										// Has song comments
	false,										// Doesn't have "+++" pattern
	false,										// Doesn't have "---" pattern
	true,										// Has restart position (order)
	true,										// Supports plugins
	false,										// No custom pattern time signatures
	true,										// Pattern names
	true,										// Has artist name
	false,										// Doesn't have default resampling
	false,										// Integer tempo
	" 0123456789ABCDRFFTE???GHK?YXPLZ\\?#??W????????????????????",	// Supported Effects
	" vpcdabuhlrg????",							// Supported Volume Column commands
};

constexpr CModSpecifications s3m_ =
{
	MOD_TYPE_S3M,								// Internal MODTYPE value
	"s3m",										// File extension
	13,											// Minimum note index
	108,										// Maximum note index
	100,										// Pattern max.
	255,										// Order max.
	1,											// Only one order list
	1,											// Channel min
	32,											// Channel max
	33,											// Min tempo
	255,										// Max tempo
	1,											// Min Speed
	255,										// Max Speed
	64,											// Min pattern rows
	64,											// Max pattern rows
	27,											// Max mod name length
	27,											// Max sample name length
	12,											// Max sample filename length
	0,											// Max instrument name length
	0,											// Max instrument filename length
	0,											// Max comment line length
	99,											// SamplesMax
	0,											// instrumentMax
	MixLevels::Compatible,						// defaultMixLevels
	SONG_FASTVOLSLIDES | SONG_AMIGALIMITS | SONG_S3MOLDVIBRATO,	// Supported song flags
	0,											// Max MIDI mapping directives
	0,											// No instrument envelopes
	true,										// Has notecut.
	false,										// No noteoff.
	false,										// No notefade.
	false,										// No envelope release node
	false,										// No song comments
	true,										// Has "+++" pattern
	true,										// Has "---" pattern
	false,										// Doesn't have restart position (order)
	false,										// Doesn't support plugins
	false,										// No custom pattern time signatures
	false,										// No pattern names
	false,										// Doesn't have artist name
	false,										// Doesn't have default resampling
	false,										// Integer tempo
	" JFEGHLKRXODB?CQATI?SMNVW?U?????????? ????????????????????",	// Supported Effects
	" vp?????????????",							// Supported Volume Column commands
};

// S3M with MPT extensions
constexpr CModSpecifications s3mEx_ =
{
	MOD_TYPE_S3M,								// Internal MODTYPE value
	"s3m",										// File extension
	13,											// Minimum note index
	108,										// Maximum note index
	100,										// Pattern max.
	255,										// Order max.
	1,											// Only one order list
	1,											// Channel min
	32,											// Channel max
	33,											// Min tempo
	255,										// Max tempo
	1,											// Min Speed
	255,										// Max Speed
	64,											// Min pattern rows
	64,											// Max pattern rows
	27,											// Max mod name length
	27,											// Max sample name length
	12,											// Max sample filename length
	0,											// Max instrument name length
	0,											// Max instrument filename length
	0,											// Max comment line length
	99,											// SamplesMax
	0,											// instrumentMax
	MixLevels::Compatible,						// defaultMixLevels
	SONG_FASTVOLSLIDES | SONG_AMIGALIMITS,		// Supported song flags
	0,											// Max MIDI mapping directives
	0,											// No instrument envelopes
	true,										// Has notecut.
	false,										// No noteoff.
	false,										// No notefade.
	false,										// No envelope release node
	false,										// No song comments
	true,										// Has "+++" pattern
	true,										// Has "---" pattern
	false,										// Doesn't have restart position (order)
	false,										// Doesn't support plugins
	false,										// No custom pattern time signatures
	false,										// No pattern names
	false,										// Doesn't have artist name
	false,										// Doesn't have default resampling
	false,										// Integer tempo
	" JFEGHLKRXODB?CQATI?SMNVW?UY?P?Z????? ????????????????????",	// Supported Effects
	" vp?????????????",							// Supported Volume Column commands
};

constexpr CModSpecifications it_ =
{
	MOD_TYPE_IT,								// Internal MODTYPE value
	"it",										// File extension
	1,											// Minimum note index
	120,										// Maximum note index
	200,										// Pattern max.
	256,										// Order max.
	1,											// Only one order list
	1,											// Channel min
	64,											// Channel max
	32,											// Min tempo
	255,										// Max tempo
	1,											// Min Speed
	255,										// Max Speed
	32,											// Min pattern rows
	200,										// Max pattern rows
	25,											// Max mod name length
	25,											// Max sample name length
	12,											// Max sample filename length
	25,											// Max instrument name length
	12,											// Max instrument filename length
	75,											// Max comment line length
	99,											// SamplesMax
	99,											// instrumentMax
	MixLevels::Compatible,						// defaultMixLevels
	SONG_LINEARSLIDES | SONG_ITOLDEFFECTS | SONG_ITCOMPATGXX,	// Supported song flags
	0,											// Max MIDI mapping directives
	25,											// Envelope point count
	true,										// Has notecut.
	true,										// Has noteoff.
	true,										// Has notefade.
	false,										// No envelope release node
	true,										// Has song comments
	true,										// Has "+++" pattern
	true,										// Has "--" pattern
	false,										// Doesn't have restart position (order)
	false,										// Doesn't support plugins
	false,										// No custom pattern time signatures
	false,										// No pattern names
	false,										// Doesn't have artist name
	false,										// Doesn't have default resampling
	false,										// Integer tempo
	" JFEGHLKRXODB?CQATI?SMNVW?UY?P?Z????? ????????????????????",	// Supported Effects
	" vpcdab?h??gfe??",							// Supported Volume Column commands
};

constexpr CModSpecifications itEx_ =
{
	MOD_TYPE_IT,								// Internal MODTYPE value
	"it",										// File extension
	1,											// Minimum note index
	120,										// Maximum note index
	240,										// Pattern max.
	256,										// Order max.
	1,											// Only one order list
	1,											// Channel min
	127,										// Channel max
	32,											// Min tempo
	512,										// Max tempo
	1,											// Min Speed
	255,										// Max Speed
	1,											// Min pattern rows
	1024,										// Max pattern rows
	25,											// Max mod name length
	25,											// Max sample name length
	12,											// Max sample filename length
	25,											// Max instrument name length
	12,											// Max instrument filename length
	75,											// Max comment line length
	3999,										// SamplesMax
	255,										// instrumentMax
	MixLevels::Compatible,						// defaultMixLevels
	SONG_LINEARSLIDES | SONG_EXFILTERRANGE | SONG_ITOLDEFFECTS | SONG_ITCOMPATGXX,	// Supported song flags
	200,										// Max MIDI mapping directives
	25,											// Envelope point count
	true,										// Has notecut.
	true,										// Has noteoff.
	true,										// Has notefade.
	false,										// No envelope release node
	true,										// Has song comments
	true,										// Has "+++" pattern
	true,										// Has "---" pattern
	false,										// Doesn't have restart position (order)
	true,										// Supports plugins
	false,										// No custom pattern time signatures
	true,										// Pattern names
	true,										// Has artist name
	false,										// Doesn't have default resampling
	false,										// Integer tempo
	" JFEGHLKRXODB?CQATI?SMNVW?UY?P?Z\\?#?? ????????????????????",	// Supported Effects
	" vpcdab?h??gfe??",							// Supported Volume Column commands
};

const std::array<const CModSpecifications *, 8> Collection = { &mptm_, &mod_, &s3m_, &s3mEx_, &xm_, &xmEx_, &it_, &itEx_ };

const CModSpecifications &mptm = mptm_;
const CModSpecifications &mod = mod_;
const CModSpecifications &s3m = s3m_;
const CModSpecifications &s3mEx = s3mEx_;
const CModSpecifications &xm = xm_;
const CModSpecifications &xmEx = xmEx_;
const CModSpecifications &it = it_;
const CModSpecifications &itEx = itEx_;

} // namespace ModSpecs


MODTYPE CModSpecifications::ExtensionToType(std::string ext)
{
	if(ext.empty())
	{
		return MOD_TYPE_NONE;
	} else if(ext[0] == '.')
	{
		ext.erase(0, 1);
	}
	ext = mpt::ToLowerCaseAscii(ext);
	for(const auto &spec : ModSpecs::Collection)
	{
		if(ext == spec->fileExtension)
		{
			return spec->internalType;
		}
	}
	return MOD_TYPE_NONE;
}


MODTYPE CModSpecifications::ExtensionToType(mpt::ustring ext)
{
	return ExtensionToType(mpt::ToCharset(mpt::Charset::ASCII, ext));
}


bool CModSpecifications::HasNote(ModCommand::NOTE note) const
{
	if(note >= noteMin && note <= noteMax)
		return true;
	else if(ModCommand::IsSpecialNote(note))
	{
		if(note == NOTE_NOTECUT)
			return hasNoteCut;
		else if(note == NOTE_KEYOFF)
			return hasNoteOff;
		else if(note == NOTE_FADE)
			return hasNoteFade;
		else
			return (internalType == MOD_TYPE_MPT);
	} else if(note == NOTE_NONE)
		return true;
	return false;
}


bool CModSpecifications::HasVolCommand(ModCommand::VOLCMD volcmd) const
{
	if(volcmd >= MAX_VOLCMDS) return false;
	return volcommands[volcmd] != '?';
}


bool CModSpecifications::HasCommand(ModCommand::COMMAND cmd) const
{
	if(cmd >= MAX_EFFECTS) return false;
	return commands[cmd] != '?';
}


char CModSpecifications::GetVolEffectLetter(ModCommand::VOLCMD volcmd) const
{
	if(volcmd >= MAX_VOLCMDS)
		return '?';
	return volcommands[volcmd];
}


char CModSpecifications::GetGenericVolEffectLetter(ModCommand::VOLCMD volcmd)
{
	// Note: Remove this function if volume effect letter display is ever going to differ between formats, and update users to GetVolEffectLetter instead.
	static constexpr char VolCommands[] = " vpcdabuhlrgfe:o";
	static_assert(std::size(VolCommands) == MAX_VOLCMDS + 1);
	if(volcmd >= MAX_VOLCMDS)
		return '?';
	return VolCommands[volcmd];
}


char CModSpecifications::GetEffectLetter(ModCommand::COMMAND cmd) const
{
	if(cmd >= MAX_EFFECTS)
		return '?';
	return commands[cmd];
}


OPENMPT_NAMESPACE_END
