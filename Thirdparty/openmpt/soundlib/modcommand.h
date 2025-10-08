/*
 * modcommand.h
 * ------------
 * Purpose: ModCommand declarations and helpers. One ModCommand corresponds to one pattern cell.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"
#include "Snd_defs.h"
#include "mpt/base/algorithm.hpp"

OPENMPT_NAMESPACE_BEGIN

class CSoundFile;

// Note definitions
enum : uint8 // ModCommand::NOTE
{
	NOTE_NONE        = 0,    // Empty note cell
	NOTE_MIN         = 1,    // Minimum note value
	NOTE_MAX         = 128,  // Maximum note value
	NOTE_MIDDLEC     = (5 * 12 + NOTE_MIN),
	NOTE_KEYOFF      = 0xFF, // === (Note Off, releases envelope / fades samples, stops plugin note)
	NOTE_NOTECUT     = 0xFE, // ^^^ (Cuts sample / stops all plugin notes)
	NOTE_FADE        = 0xFD, // ~~~ (Fades samples, stops plugin note)
	NOTE_PC          = 0xFC, // Param Control 'note'. Changes param value on first tick.
	NOTE_PCS         = 0xFB, // Param Control (Smooth) 'note'. Interpolates param value during the whole row.
	NOTE_MIN_SPECIAL = NOTE_PCS,
	NOTE_MAX_SPECIAL = NOTE_KEYOFF,
};


// Volume Column commands
enum VolumeCommand : uint8
{
	VOLCMD_NONE           = 0,
	VOLCMD_VOLUME         = 1,
	VOLCMD_PANNING        = 2,
	VOLCMD_VOLSLIDEUP     = 3,
	VOLCMD_VOLSLIDEDOWN   = 4,
	VOLCMD_FINEVOLUP      = 5,
	VOLCMD_FINEVOLDOWN    = 6,
	VOLCMD_VIBRATOSPEED   = 7,
	VOLCMD_VIBRATODEPTH   = 8,
	VOLCMD_PANSLIDELEFT   = 9,
	VOLCMD_PANSLIDERIGHT  = 10,
	VOLCMD_TONEPORTAMENTO = 11,
	VOLCMD_PORTAUP        = 12,
	VOLCMD_PORTADOWN      = 13,
	VOLCMD_PLAYCONTROL    = 14,
	VOLCMD_OFFSET         = 15,
	MAX_VOLCMDS
};


// Effect column commands
enum EffectCommand : uint8
{
	CMD_NONE                = 0,
	CMD_ARPEGGIO            = 1,
	CMD_PORTAMENTOUP        = 2,
	CMD_PORTAMENTODOWN      = 3,
	CMD_TONEPORTAMENTO      = 4,
	CMD_VIBRATO             = 5,
	CMD_TONEPORTAVOL        = 6,
	CMD_VIBRATOVOL          = 7,
	CMD_TREMOLO             = 8,
	CMD_PANNING8            = 9,
	CMD_OFFSET              = 10,
	CMD_VOLUMESLIDE         = 11,
	CMD_POSITIONJUMP        = 12,
	CMD_VOLUME              = 13,
	CMD_PATTERNBREAK        = 14,
	CMD_RETRIG              = 15,
	CMD_SPEED               = 16,
	CMD_TEMPO               = 17,
	CMD_TREMOR              = 18,
	CMD_MODCMDEX            = 19,
	CMD_S3MCMDEX            = 20,
	CMD_CHANNELVOLUME       = 21,
	CMD_CHANNELVOLSLIDE     = 22,
	CMD_GLOBALVOLUME        = 23,
	CMD_GLOBALVOLSLIDE      = 24,
	CMD_KEYOFF              = 25,
	CMD_FINEVIBRATO         = 26,
	CMD_PANBRELLO           = 27,
	CMD_XFINEPORTAUPDOWN    = 28,
	CMD_PANNINGSLIDE        = 29,
	CMD_SETENVPOSITION      = 30,
	CMD_MIDI                = 31,
	CMD_SMOOTHMIDI          = 32,
	CMD_DELAYCUT            = 33,
	CMD_XPARAM              = 34,
	CMD_FINETUNE            = 35,
	CMD_FINETUNE_SMOOTH     = 36,
	CMD_DUMMY               = 37,
	CMD_NOTESLIDEUP         = 38, // IMF Gxy / PTM Jxy (Slide y notes up every x ticks)
	CMD_NOTESLIDEDOWN       = 39, // IMF Hxy / PTM Kxy (Slide y notes down every x ticks)
	CMD_NOTESLIDEUPRETRIG   = 40, // PTM Lxy (Slide y notes up every x ticks + retrigger note)
	CMD_NOTESLIDEDOWNRETRIG = 41, // PTM Mxy (Slide y notes down every x ticks + retrigger note)
	CMD_REVERSEOFFSET       = 42, // PTM Nxx Revert sample + offset
	CMD_DBMECHO             = 43, // DBM enable/disable echo
	CMD_OFFSETPERCENTAGE    = 44, // PLM Percentage Offset
	CMD_DIGIREVERSESAMPLE   = 45, // DIGI reverse sample
	CMD_VOLUME8             = 46, // 8-bit volume
	CMD_HMN_MEGA_ARP        = 47, // His Master's Noise "mega-arp"
	CMD_MED_SYNTH_JUMP      = 48, // MED synth jump / MIDI panning
	CMD_AUTO_VOLUMESLIDE    = 49,
	CMD_AUTO_PORTAUP        = 50,
	CMD_AUTO_PORTADOWN      = 51,
	CMD_AUTO_PORTAUP_FINE   = 52,
	CMD_AUTO_PORTADOWN_FINE = 53,
	CMD_AUTO_PORTAMENTO_FC  = 54, // Future Composer
	CMD_TONEPORTA_DURATION  = 55, // Parameter = how many rows the slide should last
	CMD_VOLUMEDOWN_DURATION = 56, // Parameter = how many rows the slide should last
	CMD_VOLUMEDOWN_ETX      = 57, // EasyTrax fade-out (parameter = speed, independent of song tempo)
	MAX_EFFECTS
};


enum class EffectType : uint8
{
	Normal   = 0,
	Global   = 1,
	Volume   = 2,
	Panning  = 3,
	Pitch    = 4,
	NumTypes = 5
};


class ModCommand
{
public:
	using NOTE = uint8;
	using INSTR = uint8;
	using VOL = uint8;
	using VOLCMD = VolumeCommand;
	using COMMAND = EffectCommand;
	using PARAM = uint8;

	// Defines the maximum value for column data when interpreted as 2-byte value
	// (for example volcmd and vol). The valid value range is [0, maxColumnValue].
	static constexpr int maxColumnValue = 999;

	bool operator==(const ModCommand &mc) const
	{
		return (note == mc.note)
			&& (instr == mc.instr)
			&& (volcmd == mc.volcmd)
			&& (command == mc.command)
			&& ((volcmd == VOLCMD_NONE && !IsPcNote()) || vol == mc.vol)
			&& ((command == CMD_NONE && !IsPcNote()) || param == mc.param);
	}
	bool operator!=(const ModCommand& mc) const { return !(*this == mc); }

	MPT_FORCEINLINE void SetVolumeCommand(const VolumeCommand c, const VOL v) { volcmd = c; vol = v; }
	MPT_FORCEINLINE void SetVolumeCommand(const std::pair<VolumeCommand, VOL> cmd) { volcmd = cmd.first; vol = cmd.second; }
	MPT_FORCEINLINE void SetVolumeCommand(const ModCommand &other) { volcmd = other.volcmd; vol = other. vol; }
	MPT_FORCEINLINE void SetEffectCommand(const EffectCommand c, const PARAM p) { command = c; param = p; }
	MPT_FORCEINLINE void SetEffectCommand(const std::pair<EffectCommand, PARAM> cmd) { command = cmd.first; param = cmd.second; }
	MPT_FORCEINLINE void SetEffectCommand(const ModCommand &other) { command = other.command; param = other.param; }

	void Set(NOTE n, INSTR ins, uint16 volcol, uint16 effectcol) { note = n; instr = ins; SetValueVolCol(volcol); SetValueEffectCol(effectcol); }

	uint16 GetValueVolCol() const { return GetValueVolCol(volcmd, vol); }
	static uint16 GetValueVolCol(uint8 volcmd, uint8 vol) { return static_cast<uint16>(volcmd << 8) + vol; }
	void SetValueVolCol(const uint16 val) { volcmd = static_cast<VOLCMD>(val >> 8); vol = static_cast<uint8>(val & 0xFF); }

	uint16 GetValueEffectCol() const { return GetValueEffectCol(command, param); }
	static uint16 GetValueEffectCol(uint8 command, uint8 param) { return static_cast<uint16>(command << 8) + param; }
	void SetValueEffectCol(const uint16 val) { command = static_cast<COMMAND>(val >> 8); param = static_cast<uint8>(val & 0xFF); }

	// Clears modcommand.
	void Clear() { memset(this, 0, sizeof(ModCommand)); }

	// Returns true if modcommand is empty, false otherwise.
	bool IsEmpty() const
	{
		return (note == NOTE_NONE && instr == 0 && volcmd == VOLCMD_NONE && command == CMD_NONE);
	}

	// Returns true if instrument column represents plugin index.
	bool IsInstrPlug() const { return IsPcNote(); }

	// Returns true if and only if note is NOTE_PC or NOTE_PCS.
	bool IsPcNote() const { return IsPcNote(note); }
	static bool IsPcNote(NOTE note) { return note == NOTE_PC || note == NOTE_PCS; }

	// Returns true if and only if note is a valid musical note.
	bool IsNote() const { return mpt::is_in_range(note, NOTE_MIN, NOTE_MAX); }
	static bool IsNote(NOTE note) { return mpt::is_in_range(note, NOTE_MIN, NOTE_MAX); }
	// Returns true if and only if note is a valid special note.
	bool IsSpecialNote() const { return mpt::is_in_range(note, NOTE_MIN_SPECIAL, NOTE_MAX_SPECIAL); }
	static bool IsSpecialNote(NOTE note) { return mpt::is_in_range(note, NOTE_MIN_SPECIAL, NOTE_MAX_SPECIAL); }
	// Returns true if and only if note is a valid musical note or the note entry is empty.
	bool IsNoteOrEmpty() const { return note == NOTE_NONE || IsNote(); }
	static bool IsNoteOrEmpty(NOTE note) { return note == NOTE_NONE || IsNote(note); }
	// Returns true if any of the commands in this cell trigger a tone portamento.
	bool IsTonePortamento() const { return command == CMD_TONEPORTAMENTO || command == CMD_TONEPORTAVOL || command == CMD_TONEPORTA_DURATION || volcmd == VOLCMD_TONEPORTAMENTO; }
	// Returns true if any commands in this cell trigger any sort of pitch slide / portamento.
	bool IsAnyPitchSlide() const;
	// Returns true if the cell contains a sliding or otherwise continuous effect command.
	bool IsContinousCommand(const CSoundFile &sndFile) const;
	bool IsContinousVolColCommand() const;
	// Returns true if the cell contains a sliding command with separate up/down nibbles.
	bool IsSlideUpDownCommand() const;
	// Returns true if the cell contains an effect command that may affect the global state of the module.
	bool IsGlobalCommand() const { return IsGlobalCommand(command, param); }
	static bool IsGlobalCommand(COMMAND command, PARAM param);
	// Returns true if the cell contains an effect command whose parameter is divided into two nibbles
	bool CommandHasTwoNibbles() const { return CommandHasTwoNibbles(command); }
	static bool CommandHasTwoNibbles(COMMAND command);
	// Returns true if the command is a regular volume slide
	bool IsNormalVolumeSlide() const { return command == CMD_VOLUMESLIDE || command == CMD_VIBRATOVOL || command == CMD_TONEPORTAVOL; }

	// Returns true if the note is inside the Amiga frequency range
	bool IsAmigaNote() const { return IsAmigaNote(note); }
	static bool IsAmigaNote(NOTE note) { return !IsNote(note) || (note >= NOTE_MIDDLEC - 12 && note < NOTE_MIDDLEC + 24); }

	static EffectType GetEffectType(COMMAND cmd);
	EffectType GetEffectType() const { return GetEffectType(command); }
	static EffectType GetVolumeEffectType(VOLCMD volcmd);
	EffectType GetVolumeEffectType() const { return GetVolumeEffectType(volcmd); }

	// Convert a complete ModCommand item from one format to another
	void Convert(MODTYPE fromType, MODTYPE toType, const CSoundFile &sndFile);
	// Convert MOD/XM Exx to S3M/IT Sxx
	void ExtendedMODtoS3MEffect();
	// Convert S3M/IT Sxx to MOD/XM Exx
	void ExtendedS3MtoMODEffect();

	// "Importance" of every FX command. Table is used for importing from formats with multiple effect columns
	// and is approximately the same as in SchismTracker.
	static size_t GetEffectWeight(COMMAND cmd);
	// Try to convert a an effect into a volume column effect. Returns converted effect on success.
	[[nodiscard]] static std::pair<VolumeCommand, VOL> ConvertToVolCommand(const EffectCommand effect, PARAM param, bool force);
	// Takes two "normal" effect commands and converts them to volume column + effect column commands. Returns the dropped command + param (CMD_NONE if nothing had to be dropped).
	std::pair<EffectCommand, PARAM> FillInTwoCommands(EffectCommand effect1, uint8 param1, EffectCommand effect2, uint8 param2, bool allowLowResOffset = false);
	// Try to combine two commands into one. Returns true on success and the combined command is placed in eff1 / param1.
	static bool CombineEffects(EffectCommand &eff1, uint8 &param1, EffectCommand &eff2, uint8 &param2);

public:
	uint8 note = NOTE_NONE;
	uint8 instr = 0;
	VolumeCommand volcmd = VOLCMD_NONE;
	EffectCommand command = CMD_NONE;
	uint8 vol = 0;
	uint8 param = 0;
};

OPENMPT_NAMESPACE_END
