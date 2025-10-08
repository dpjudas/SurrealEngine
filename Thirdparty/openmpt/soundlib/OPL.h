/*
 * OPL.h
 * -----
 * Purpose: Translate data coming from OpenMPT's mixer into OPL commands to be sent to the Opal emulator.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "Snd_defs.h"

OPENMPT_NAMESPACE_BEGIN

class Opal;

class OPL
{
public:
	enum OPLRegisters : uint8
	{
		// Operators (combine with result of OperatorToRegister)
		AM_VIB          = 0x20,  // AM / VIB / EG / KSR / Multiple (0x20 to 0x35)
		KSL_LEVEL       = 0x40,  // KSL / Total level (0x40 to 0x55)
		ATTACK_DECAY    = 0x60,  // Attack rate / Decay rate (0x60 to 0x75)
		SUSTAIN_RELEASE = 0x80,  // Sustain level / Release rate (0x80 to 0x95)
		WAVE_SELECT     = 0xE0,  // Wave select (0xE0 to 0xF5)

		// Channels (combine with result of ChannelToRegister)
		FNUM_LOW            = 0xA0,  // F-number low bits (0xA0 to 0xA8)
		KEYON_BLOCK         = 0xB0,  // F-number high bits / Key on / Block (octave) (0xB0 to 0xB8)
		FEEDBACK_CONNECTION = 0xC0,  // Feedback / Connection (0xC0 to 0xC8)

		TREMOLO_VIBRATO_DEPTH = 0xBD,  // Tremolo Depth / Vibrato Depth / Percussion Mode / BD/SD/TT/CY/HH Key-On
	};

	enum OPLValues : uint8
	{
		// AM_VIB
		TREMOLO_ON       = 0x80,
		VIBRATO_ON       = 0x40,
		SUSTAIN_ON       = 0x20,
		KSR              = 0x10,  // Key scaling rate
		MULTIPLE_MASK    = 0x0F,  // Frequency multiplier

		// KSL_LEVEL
		KSL_MASK         = 0xC0,  // Envelope scaling bits
		TOTAL_LEVEL_MASK = 0x3F,  // Strength (volume) of OP

		// ATTACK_DECAY
		ATTACK_MASK      = 0xF0,
		DECAY_MASK       = 0x0F,

		// SUSTAIN_RELEASE
		SUSTAIN_MASK     = 0xF0,
		RELEASE_MASK     = 0x0F,

		// KEYON_BLOCK
		KEYON_BIT        = 0x20,

		// FEEDBACK_CONNECTION
		FEEDBACK_MASK    = 0x0E,  // Valid just for first OP of a voice
		CONNECTION_BIT   = 0x01,
		VOICE_TO_LEFT    = 0x10,
		VOICE_TO_RIGHT   = 0x20,
		STEREO_BITS      = VOICE_TO_LEFT | VOICE_TO_RIGHT,
	};

	using Register = uint16;
	using Value = uint8;

	class IRegisterLogger
	{
	public:
		virtual void Port(CHANNELINDEX c, Register reg, Value value) = 0;
		virtual void MoveChannel(CHANNELINDEX from, CHANNELINDEX to) = 0;
		virtual ~IRegisterLogger() {}
	};

	explicit OPL(uint32 sampleRate);
	explicit OPL(IRegisterLogger &logger);
	~OPL();

	void Initialize(uint32 sampleRate);
	void Mix(int32 *buffer, size_t count, uint32 volumeFactorQ16);

	void NoteOff(CHANNELINDEX c);
	void NoteCut(CHANNELINDEX c, bool unassign = true);
	void Frequency(CHANNELINDEX c, uint32 milliHertz, bool keyOff, bool beatingOscillators);
	void Volume(CHANNELINDEX c, uint8 vol, bool applyToModulator);
	int8 Pan(CHANNELINDEX c, int32 pan);
	void Patch(CHANNELINDEX c, const OPLPatch &patch);
	bool IsActive(CHANNELINDEX c) const { return GetVoice(c) != OPL_CHANNEL_INVALID; }
	void MoveChannel(CHANNELINDEX from, CHANNELINDEX to);
	void Reset();

	// A list of all registers for channels and operators if oplCh == 0xFF, otherwise all registers for the given channel and its operators
	static std::vector<Register> AllVoiceRegisters(uint8 oplCh = 0xFF);
	// Returns voice for given register, or 0xFF if it's not a voice-specific register
	static uint8 RegisterToVoice(Register reg);
	// Returns register without any voice offset, so as if it was triggering the first voice
	static Register StripVoiceFromRegister(Register reg);

protected:
	static Register ChannelToRegister(uint8 oplCh);
	static Register OperatorToRegister(uint8 oplCh);
	static uint8 CalcVolume(uint8 trackerVol, uint8 kslVolume);
	uint8 GetVoice(CHANNELINDEX c) const;
	uint8 AllocateVoice(CHANNELINDEX c);
	void Port(CHANNELINDEX c, Register reg, Value value);

	enum
	{
		OPL_CHANNELS = 18,       // 9 for OPL2 or 18 for OPL3
		OPL_CHANNEL_CUT = 0x80,  // Indicates that the channel has been cut and used as a hint to re-use the channel for the same tracker channel if possible
		OPL_CHANNEL_MASK = 0x7F,
		OPL_CHANNEL_INVALID = 0xFF,
		OPL_BASERATE = 49716,
	};

	std::unique_ptr<Opal> m_opl;
	IRegisterLogger *m_logger = nullptr;

	std::array<uint8, OPL_CHANNELS> m_KeyOnBlock;
	std::array<CHANNELINDEX, OPL_CHANNELS> m_OPLtoChan;
	std::array<uint8, MAX_CHANNELS> m_ChanToOPL;
	std::array<OPLPatch, OPL_CHANNELS> m_Patches;

	bool m_isActive = false;
};

OPENMPT_NAMESPACE_END
