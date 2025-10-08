/*
 * MIDIMacros.h
 * ------------
 * Purpose: Helper functions / classes for MIDI Macro functionality.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "Snd_defs.h"
#include "openmpt/base/Endian.hpp"

OPENMPT_NAMESPACE_BEGIN

class IMixPlugin;

enum
{
	kGlobalMacros = 9,    // Number of global macros
	kSFxMacros    = 16,   // Number of parametered macros
	kZxxMacros    = 128,  // Number of fixed macros
	kMacroLength  = 32,   // Max number of chars per macro
};

// Parametered macro presets
enum ParameteredMacro
{
	kSFxUnused = 0,
	kSFxCutoff,     // Z00 - Z7F controls resonant filter cutoff
	kSFxReso,       // Z00 - Z7F controls resonant filter resonance
	kSFxFltMode,    // Z00 - Z7F controls resonant filter mode (lowpass / highpass)
	kSFxDryWet,     // Z00 - Z7F controls plugin Dry / Wet ratio
	kSFxPlugParam,  // Z00 - Z7F controls a plugin parameter
	kSFxCC,         // Z00 - Z7F controls MIDI CC
	kSFxChannelAT,  // Z00 - Z7F controls Channel Aftertouch
	kSFxPolyAT,     // Z00 - Z7F controls Poly Aftertouch
	kSFxPitch,      // Z00 - Z7F controls Pitch Bend
	kSFxProgChange, // Z00 - Z7F controls MIDI Program Change
	kSFxCustom,

	kSFxMax
};


// Fixed macro presets
enum FixedMacro
{
	kZxxUnused = 0,
	kZxxReso4Bit,    // Z80 - Z8F controls resonant filter resonance
	kZxxReso7Bit,    // Z80 - ZFF controls resonant filter resonance
	kZxxCutoff,      // Z80 - ZFF controls resonant filter cutoff
	kZxxFltMode,     // Z80 - ZFF controls resonant filter mode (lowpass / highpass)
	kZxxResoFltMode, // Z80 - Z9F controls resonance + filter mode
	kZxxChannelAT,   // Z80 - ZFF controls Channel Aftertouch
	kZxxPolyAT,      // Z80 - ZFF controls Poly Aftertouch
	kZxxPitch,       // Z80 - ZFF controls Pitch Bend
	kZxxProgChange,  // Z80 - ZFF controls MIDI Program Change
	kZxxCustom,

	kZxxMax
};


// Global macro types
enum GlobalMacro
{
	MIDIOUT_START = 0,
	MIDIOUT_STOP,
	MIDIOUT_TICK,
	MIDIOUT_NOTEON,
	MIDIOUT_NOTEOFF,
	MIDIOUT_VOLUME,
	MIDIOUT_PAN,
	MIDIOUT_BANKSEL,
	MIDIOUT_PROGRAM,
};


struct MIDIMacroConfigData
{
	struct Macro
	{
	public:
		Macro() = default;
		Macro(const Macro &other) = default;
		Macro &operator=(const Macro &other) = default;
		Macro &operator=(const std::string_view other) noexcept
		{
			const size_t copyLength = std::min({m_data.size() - 1u, other.size(), other.find('\0')});
			std::copy(other.begin(), other.begin() + copyLength, m_data.begin());
			m_data[copyLength] = '\0';
			Sanitize();
			return *this;
		}

		bool operator==(const Macro &other) const noexcept
		{
			return m_data == other.m_data;  // Don't care about data past null-terminator as operator= and Sanitize() ensure there is no data behind it.
		}
		bool operator!=(const Macro &other) const noexcept
		{
			return !(*this == other);
		}

		operator mpt::span<const char>() const noexcept
		{
			return {m_data.data(), Length()};
		}
		operator std::string_view() const noexcept
		{
			return {m_data.data(), Length()};
		}
		operator std::string() const
		{
			return {m_data.data(), Length()};
		}

		MPT_CONSTEXPR20_FUN size_t Length() const noexcept
		{
			return static_cast<size_t>(std::distance(m_data.begin(), std::find(m_data.begin(), m_data.end(), '\0')));
		}

		MPT_CONSTEXPR20_FUN void Clear() noexcept
		{
			m_data.fill('\0');
		}

		// Remove blanks and other unwanted characters from macro strings for internal usage.
		std::string NormalizedString() const;

		void Sanitize() noexcept;
		void UpgradeLegacyMacro() noexcept;

	private:
		std::array<char, kMacroLength> m_data;
	};

	std::array<Macro, kGlobalMacros> Global;
	std::array<Macro, kSFxMacros> SFx;  // Parametered macros for Z00...Z7F
	std::array<Macro, kZxxMacros> Zxx;  // Fixed macros Z80...ZFF

	constexpr Macro *begin() noexcept {return Global.data(); }
	constexpr const Macro *begin() const noexcept { return Global.data(); }
	constexpr Macro *end() noexcept { return Zxx.data() + Zxx.size(); }
	constexpr const Macro *end() const noexcept { return Zxx.data() + Zxx.size(); }
};

// This is directly written to files, so the size must be correct!
MPT_BINARY_STRUCT(MIDIMacroConfigData::Macro, 32)
MPT_BINARY_STRUCT(MIDIMacroConfigData, 4896)

class MIDIMacroConfig : public MIDIMacroConfigData
{

public:

	MIDIMacroConfig() { Reset(); }

	// Get macro type from a macro string
	ParameteredMacro GetParameteredMacroType(uint32 macroIndex) const;
	FixedMacro GetFixedMacroType() const;

	// Create a new macro
protected:
	void CreateParameteredMacro(Macro &parameteredMacro, ParameteredMacro macroType, int subType) const;
public:
	void CreateParameteredMacro(uint32 macroIndex, ParameteredMacro macroType, int subType = 0)
	{
		if(macroIndex < std::size(SFx))
			CreateParameteredMacro(SFx[macroIndex], macroType, subType);
	}
	std::string CreateParameteredMacro(ParameteredMacro macroType, int subType = 0) const;

protected:
	void CreateFixedMacro(std::array<Macro, kZxxMacros> &fixedMacros, FixedMacro macroType) const;
public:
	void CreateFixedMacro(FixedMacro macroType)
	{
		CreateFixedMacro(Zxx, macroType);
	}

	bool operator==(const MIDIMacroConfig &other) const;
	bool operator!=(const MIDIMacroConfig &other) const { return !(*this == other); }

#ifdef MODPLUG_TRACKER

	// Translate macro type or macro string to macro name
	CString GetParameteredMacroName(uint32 macroIndex, IMixPlugin *plugin = nullptr) const;
	CString GetParameteredMacroName(ParameteredMacro macroType) const;
	CString GetFixedMacroName(FixedMacro macroType) const;

	// Extract information from a parametered macro string.
	PlugParamIndex MacroToPlugParam(uint32 macroIndex) const;
	int MacroToMidiCC(uint32 macroIndex) const;

	// Check if any macro can automate a given plugin parameter
	int FindMacroForParam(PlugParamIndex param) const;

#endif // MODPLUG_TRACKER

	// Check if a given set of macros is the default IT macro set.
	bool IsMacroDefaultSetupUsed() const;

	// Reset MIDI macro config to default values.
	void Reset();

	// Clear all Zxx macros so that they do nothing.
	void ClearZxxMacros();

	// Sanitize all macro config strings.
	void Sanitize();

	// Fix old-format (not conforming to IT's MIDI macro definitions) MIDI config strings.
	void UpgradeMacros();
};

static_assert(sizeof(MIDIMacroConfig) == sizeof(MIDIMacroConfigData)); // this is directly written to files, so the size must be correct!


OPENMPT_NAMESPACE_END
