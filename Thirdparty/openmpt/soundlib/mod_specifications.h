/*
 * mod_specifications.h
 * --------------------
 * Purpose: Mod specifications characterise the features of every editable module format in OpenMPT, such as the number of supported channels, samples, effects, etc...
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "Snd_defs.h"
#include "modcommand.h"                       // ModCommand::
#include "../soundlib/SoundFilePlayConfig.h"  // mixlevel constants.


OPENMPT_NAMESPACE_BEGIN


struct CModSpecifications
{
	/// Returns modtype corresponding to given file extension. The extension string
	/// may begin with or without dot, e.g. both ".it" and "it" will be handled correctly.
	static MODTYPE ExtensionToType(std::string ext); // (encoded in ASCII)
	static MODTYPE ExtensionToType(mpt::ustring ext);

	// Return true if format supports given note.
	bool HasNote(ModCommand::NOTE note) const;
	bool HasVolCommand(ModCommand::VOLCMD volcmd) const;
	bool HasCommand(ModCommand::COMMAND cmd) const;
	// Return corresponding effect letter for this format
	char GetEffectLetter(ModCommand::COMMAND cmd) const;
	char GetVolEffectLetter(ModCommand::VOLCMD volcmd) const;
	static char GetGenericVolEffectLetter(ModCommand::VOLCMD volcmd);

	// NOTE: If changing order, update all initializations in .cpp file.
	MODTYPE internalType;       // Internal MODTYPE value
	const char *fileExtension;  // File extension without dot (encoded in ASCII).
	mpt::ustring GetFileExtension() const { return mpt::ToUnicode(mpt::Charset::ASCII, fileExtension); }
	mpt::ustring GetFileExtensionLower() const { return mpt::ToUnicode(mpt::Charset::ASCII, mpt::ToLowerCaseAscii(fileExtension)); }
	mpt::ustring GetFileExtensionUpper() const { return mpt::ToUnicode(mpt::Charset::ASCII, mpt::ToUpperCaseAscii(fileExtension)); }
	ModCommand::NOTE noteMin;   // Minimum note index (index starts from 1)
	ModCommand::NOTE noteMax;   // Maximum note index (index starts from 1)
	PATTERNINDEX patternsMax;
	ORDERINDEX ordersMax;
	SEQUENCEINDEX sequencesMax;
	CHANNELINDEX channelsMin;  // Minimum number of editable channels in pattern.
	CHANNELINDEX channelsMax;  // Maximum number of editable channels in pattern.
	uint32 tempoMinInt;
	uint32 tempoMaxInt;
	uint32 speedMin;  // Minimum ticks per frame
	uint32 speedMax;  // Maximum ticks per frame
	ROWINDEX patternRowsMin;
	ROWINDEX patternRowsMax;
	uint16 modNameLengthMax;         // Meaning 'usable letters', possible null character is not included.
	uint16 sampleNameLengthMax;      // Ditto
	uint16 sampleFilenameLengthMax;  // Ditto
	uint16 instrNameLengthMax;       // Ditto
	uint16 instrFilenameLengthMax;   // Ditto
	uint16 commentLineLengthMax;     // not including line break, 0 for unlimited
	SAMPLEINDEX samplesMax;          // Max number of samples == Highest possible sample index
	INSTRUMENTINDEX instrumentsMax;  // Max number of instruments == Highest possible instrument index
	MixLevels defaultMixLevels;      // Default mix levels that are used when creating a new file in this format
	FlagSet<SongFlags> songFlags;    // Supported song flags
	uint8 MIDIMappingDirectivesMax;  // Number of MIDI Mapping directives that the format can store (0 = none)
	uint8 envelopePointsMax;         // Maximum number of points of each envelope
	bool hasNoteCut;                 // True if format has note cut (^^).
	bool hasNoteOff;                 // True if format has note off (==).
	bool hasNoteFade;                // True if format has note fade (~~).
	bool hasReleaseNode;             // Envelope release node
	bool hasComments;                // True if format has a comments field
	bool hasIgnoreIndex;             // Does "+++" pattern exist?
	bool hasStopIndex;               // Does "---" pattern exist?
	bool hasRestartPos;              // Format has an automatic restart order position
	bool supportsPlugins;            // Format can store plugins
	bool hasPatternSignatures;       // Can patterns have a custom time signature?
	bool hasPatternNames;            // Can patterns have a name?
	bool hasArtistName;              // Can artist name be stored in file?
	bool hasDefaultResampling;       // Can default resampling be saved? (if not, it can still be modified in the GUI but won't set the module as modified)
	bool hasFractionalTempo;         // Are fractional tempos allowed?
	const char *commands;            // An array holding all commands this format supports; commands that are not supported are marked with "?"
	const char *volcommands;         // Ditto, but for volume column
	MPT_CONSTEXPRINLINE TEMPO GetTempoMin() const { return TEMPO(tempoMinInt, 0); }
	MPT_CONSTEXPRINLINE TEMPO GetTempoMax() const { return TEMPO(tempoMaxInt, 0); }
};


namespace ModSpecs
{
	extern const CModSpecifications & mptm;
	extern const CModSpecifications & mod;
	extern const CModSpecifications & s3m;
	extern const CModSpecifications & s3mEx;
	extern const CModSpecifications & xm;
	extern const CModSpecifications & xmEx;
	extern const CModSpecifications & it;
	extern const CModSpecifications & itEx;
	extern const std::array<const CModSpecifications *, 8> Collection;
}


OPENMPT_NAMESPACE_END
