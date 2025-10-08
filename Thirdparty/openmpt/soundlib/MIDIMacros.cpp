/*
 * MIDIMacros.cpp
 * --------------
 * Purpose: Helper functions / classes for MIDI Macro functionality.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "MIDIMacros.h"
#include "../soundlib/MIDIEvents.h"

#ifdef MODPLUG_TRACKER
#include "Sndfile.h"
#include "plugins/PlugInterface.h"
#endif // MODPLUG_TRACKER

OPENMPT_NAMESPACE_BEGIN

ParameteredMacro MIDIMacroConfig::GetParameteredMacroType(uint32 macroIndex) const
{
	const std::string macro = SFx[macroIndex].NormalizedString();

	for(uint32 i = 0; i < kSFxMax; i++)
	{
		ParameteredMacro sfx = static_cast<ParameteredMacro>(i);
		if(sfx != kSFxCustom)
		{
			if(macro == CreateParameteredMacro(sfx))
				return sfx;
		}
	}

	// Special macros with additional "parameter":
	if(macro.size() == 5 && macro.compare(CreateParameteredMacro(kSFxCC, MIDIEvents::MIDICC_start)) >= 0 && macro.compare(CreateParameteredMacro(kSFxCC, MIDIEvents::MIDICC_end)) <= 0)
		return kSFxCC;
	if(macro.size() == 7 && macro.compare(CreateParameteredMacro(kSFxPlugParam, 0)) >= 0 && macro.compare(CreateParameteredMacro(kSFxPlugParam, 0x17F)) <= 0)
		return kSFxPlugParam; 

	return kSFxCustom;  // custom / unknown
}


// Retrieve Zxx (Z80-ZFF) type from current macro configuration
FixedMacro MIDIMacroConfig::GetFixedMacroType() const
{
	// Compare with all possible preset patterns
	for(uint32 i = 0; i < kZxxMax; i++)
	{
		FixedMacro zxx = static_cast<FixedMacro>(i);
		if(zxx != kZxxCustom)
		{
			// Prepare macro pattern to compare
			decltype(Zxx) fixedMacros{};
			CreateFixedMacro(fixedMacros, zxx);
			if(fixedMacros == Zxx)
				return zxx;
		}
	}
	return kZxxCustom; // Custom setup
}


void MIDIMacroConfig::CreateParameteredMacro(Macro &parameteredMacro, ParameteredMacro macroType, int subType) const
{
	switch(macroType)
	{
	case kSFxUnused:     parameteredMacro = ""; break;
	case kSFxCutoff:     parameteredMacro = "F0F000z"; break;
	case kSFxReso:       parameteredMacro = "F0F001z"; break;
	case kSFxFltMode:    parameteredMacro = "F0F002z"; break;
	case kSFxDryWet:     parameteredMacro = "F0F003z"; break;
	case kSFxCC:         parameteredMacro = MPT_AFORMAT("Bc{}z")(mpt::afmt::HEX0<2>(subType & 0x7F)); break;
	case kSFxPlugParam:  parameteredMacro = MPT_AFORMAT("F0F{}z")(mpt::afmt::HEX0<3>(std::min(subType, 0x17F) + 0x80)); break;
	case kSFxChannelAT:  parameteredMacro = "Dcz"; break;
	case kSFxPolyAT:     parameteredMacro = "Acnz"; break;
	case kSFxPitch:      parameteredMacro = "Ec00z"; break;
	case kSFxProgChange: parameteredMacro = "Ccz"; break;
	case kSFxCustom:
	default:
		MPT_ASSERT_NOTREACHED();
		break;
	}
}


std::string MIDIMacroConfig::CreateParameteredMacro(ParameteredMacro macroType, int subType) const
{
	Macro parameteredMacro{};
	CreateParameteredMacro(parameteredMacro, macroType, subType);
	return parameteredMacro;
}


// Create Zxx (Z80 - ZFF) from preset
void MIDIMacroConfig::CreateFixedMacro(std::array<Macro, kZxxMacros> &fixedMacros, FixedMacro macroType) const
{
	for(uint32 i = 0; i < kZxxMacros; i++)
	{
		uint32 param = i;
		switch(macroType)
		{
		case kZxxUnused:
			fixedMacros[i] = "";
			break;
		case kZxxReso4Bit:
			param = i * 8;
			if(i < 16)
				fixedMacros[i] = MPT_AFORMAT("F0F001{}")(mpt::afmt::HEX0<2>(param));
			else
				fixedMacros[i] = "";
			break;
		case kZxxReso7Bit:
			fixedMacros[i] = MPT_AFORMAT("F0F001{}")(mpt::afmt::HEX0<2>(param));
			break;
		case kZxxCutoff:
			fixedMacros[i] = MPT_AFORMAT("F0F000{}")(mpt::afmt::HEX0<2>(param));
			break;
		case kZxxFltMode:
			fixedMacros[i] = MPT_AFORMAT("F0F002{}")(mpt::afmt::HEX0<2>(param));
			break;
		case kZxxResoFltMode:
			param = (i & 0x0F) * 8;
			if(i < 16)
				fixedMacros[i] = MPT_AFORMAT("F0F001{}")(mpt::afmt::HEX0<2>(param));
			else if(i < 32)
				fixedMacros[i] = MPT_AFORMAT("F0F002{}")(mpt::afmt::HEX0<2>(param));
			else
				fixedMacros[i] = "";
			break;
		case kZxxChannelAT:
			fixedMacros[i] = MPT_AFORMAT("Dc{}")(mpt::afmt::HEX0<2>(param));
			break;
		case kZxxPolyAT:
			fixedMacros[i] = MPT_AFORMAT("Acn{}")(mpt::afmt::HEX0<2>(param));
			break;
		case kZxxPitch:
			fixedMacros[i] = MPT_AFORMAT("Ec00{}")(mpt::afmt::HEX0<2>(param));
			break;
		case kZxxProgChange:
			fixedMacros[i] = MPT_AFORMAT("Cc{}")(mpt::afmt::HEX0<2>(param));
			break;
		case kZxxCustom:
		default:
			MPT_ASSERT_NOTREACHED();
			continue;
		}
	}
}


bool MIDIMacroConfig::operator== (const MIDIMacroConfig &other) const
{
	return std::equal(begin(), end(), other.begin());
}


#ifdef MODPLUG_TRACKER

// Returns macro description including plugin parameter / MIDI CC information
CString MIDIMacroConfig::GetParameteredMacroName(uint32 macroIndex, IMixPlugin *plugin) const
{
	const ParameteredMacro macroType = GetParameteredMacroType(macroIndex);

	switch(macroType)
	{
	case kSFxPlugParam:
		{
			const int param = MacroToPlugParam(macroIndex);
			CString formattedName;
			formattedName.Format(_T("Param %d"), param);
#ifndef NO_PLUGINS
			if(plugin != nullptr)
			{
				CString paramName = plugin->GetParamName(param);
				if(!paramName.IsEmpty())
				{
					formattedName += _T(" (") + paramName + _T(")");
				}
			} else
#else
			MPT_UNREFERENCED_PARAMETER(plugin);
#endif // NO_PLUGINS
			{
				formattedName += _T(" (N/A)");
			}
			return formattedName;
		}

	case kSFxCC:
		{
			CString formattedCC;
			formattedCC.Format(_T("MIDI CC %d"), MacroToMidiCC(macroIndex));
			return formattedCC;
		}

	default:
		return GetParameteredMacroName(macroType);
	}
}


// Returns generic macro description.
CString MIDIMacroConfig::GetParameteredMacroName(ParameteredMacro macroType) const
{
	switch(macroType)
	{
	case kSFxUnused:     return _T("Unused");
	case kSFxCutoff:     return _T("Set Filter Cutoff");
	case kSFxReso:       return _T("Set Filter Resonance");
	case kSFxFltMode:    return _T("Set Filter Mode");
	case kSFxDryWet:     return _T("Set Plugin Dry/Wet Ratio");
	case kSFxPlugParam:  return _T("Control Plugin Parameter...");
	case kSFxCC:         return _T("MIDI CC...");
	case kSFxChannelAT:  return _T("Channel Aftertouch");
	case kSFxPolyAT:     return _T("Polyphonic Aftertouch");
	case kSFxPitch:      return _T("Pitch Bend");
	case kSFxProgChange: return _T("MIDI Program Change");
	case kSFxCustom:
	default:             return _T("Custom");
	}
}


// Returns generic macro description.
CString MIDIMacroConfig::GetFixedMacroName(FixedMacro macroType) const
{
	switch(macroType)
	{
	case kZxxUnused:      return _T("Unused");
	case kZxxReso4Bit:    return _T("Z80 - Z8F controls Resonant Filter Resonance");
	case kZxxReso7Bit:    return _T("Z80 - ZFF controls Resonant Filter Resonance");
	case kZxxCutoff:      return _T("Z80 - ZFF controls Resonant Filter Cutoff");
	case kZxxFltMode:     return _T("Z80 - ZFF controls Resonant Filter Mode");
	case kZxxResoFltMode: return _T("Z80 - Z9F controls Resonance + Filter Mode");
	case kZxxChannelAT:   return _T("Z80 - ZFF controls Channel Aftertouch");
	case kZxxPolyAT:      return _T("Z80 - ZFF controls Polyphonic Aftertouch");
	case kZxxPitch:       return _T("Z80 - ZFF controls Pitch Bend");
	case kZxxProgChange:  return _T("Z80 - ZFF controls MIDI Program Change");
	case kZxxCustom:
	default:              return _T("Custom");
	}
}


PlugParamIndex MIDIMacroConfig::MacroToPlugParam(uint32 macroIndex) const
{
	const std::string macro = SFx[macroIndex].NormalizedString();

	PlugParamIndex code = 0;
	const char *param = macro.c_str();
	param += 4;
	if ((param[0] >= '0') && (param[0] <= '9')) code = (param[0] - '0') << 4; else
		if ((param[0] >= 'A') && (param[0] <= 'F')) code = (param[0] - 'A' + 0x0A) << 4;
	if ((param[1] >= '0') && (param[1] <= '9')) code += (param[1] - '0'); else
		if ((param[1] >= 'A') && (param[1] <= 'F')) code += (param[1] - 'A' + 0x0A);

	if (macro.size() >= 4 && macro[3] == '0')
		return (code - 128);
	else
		return (code + 128);
}


int MIDIMacroConfig::MacroToMidiCC(uint32 macroIndex) const
{
	const std::string macro = SFx[macroIndex].NormalizedString();

	int code = 0;
	const char *param = macro.c_str();
	param += 2;
	if ((param[0] >= '0') && (param[0] <= '9')) code = (param[0] - '0') << 4; else
		if ((param[0] >= 'A') && (param[0] <= 'F')) code = (param[0] - 'A' + 0x0A) << 4;
	if ((param[1] >= '0') && (param[1] <= '9')) code += (param[1] - '0'); else
		if ((param[1] >= 'A') && (param[1] <= 'F')) code += (param[1] - 'A' + 0x0A);

	return code;
}


int MIDIMacroConfig::FindMacroForParam(PlugParamIndex param) const
{
	for(int macroIndex = 0; macroIndex < kSFxMacros; macroIndex++)
	{
		if(GetParameteredMacroType(macroIndex) == kSFxPlugParam && MacroToPlugParam(macroIndex) == param)
		{
			return macroIndex;
		}
	}
	return -1;
}

#endif // MODPLUG_TRACKER


// Check if the MIDI Macro configuration used is the default one,
// i.e. the configuration that is assumed when loading a file that has no macros embedded.
bool MIDIMacroConfig::IsMacroDefaultSetupUsed() const
{
	return *this == MIDIMacroConfig{};
}


// Reset MIDI macro config to default values.
void MIDIMacroConfig::Reset()
{
	std::fill(begin(), end(), Macro{});

	Global[MIDIOUT_START] = "FF";
	Global[MIDIOUT_STOP] = "FC";
	Global[MIDIOUT_NOTEON] = "9c n v";
	Global[MIDIOUT_NOTEOFF] = "9c n 0";
	Global[MIDIOUT_PROGRAM] = "Cc p";
	// SF0: Z00-Z7F controls cutoff
	CreateParameteredMacro(0, kSFxCutoff);
	// Z80-Z8F controls resonance
	CreateFixedMacro(kZxxReso4Bit);
}


// Clear all Zxx macros so that they do nothing.
void MIDIMacroConfig::ClearZxxMacros()
{
	std::fill(SFx.begin(), SFx.end(), Macro{});
	std::fill(Zxx.begin(), Zxx.end(), Macro{});
}


// Sanitize all macro config strings.
void MIDIMacroConfig::Sanitize()
{
	for(auto &macro : *this)
	{
		macro.Sanitize();
	}
}


// Fix old-format (not conforming to IT's MIDI macro definitions) MIDI config strings.
void MIDIMacroConfig::UpgradeMacros()
{
	for(auto &macro : SFx)
	{
		macro.UpgradeLegacyMacro();
	}
	for(auto &macro : Zxx)
	{
		macro.UpgradeLegacyMacro();
	}
}


// Normalize by removing blanks and other unwanted characters from macro strings for internal usage.
std::string MIDIMacroConfig::Macro::NormalizedString() const
{
	std::string sanitizedMacro = *this;

	std::string::size_type pos = 0;
	while((pos = sanitizedMacro.find_first_not_of("0123456789ABCDEFabchmnopsuvxyz", pos)) != std::string::npos)
	{
		sanitizedMacro.erase(pos, 1);
	}

	return sanitizedMacro;
}


void MIDIMacroConfig::Macro::Sanitize() noexcept
{
	m_data.back() = '\0';
	const auto length = Length();
	std::fill(m_data.begin() + length, m_data.end(), '\0');
	for(size_t i = 0; i < length; i++)
	{
		if(m_data[i] < 32 || m_data[i] >= 127)
			m_data[i] = ' ';
	}
}


void MIDIMacroConfig::Macro::UpgradeLegacyMacro() noexcept
{
	for(auto &c : m_data)
	{
		if(c >= 'a' && c <= 'f')  // Both A-F and a-f were treated as hex constants
		{
			c = c - 'a' + 'A';
		} else if(c == 'K' || c == 'k')  // Channel was K or k
		{
			c = 'c';
		} else if(c == 'X' || c == 'x' || c == 'Y' || c == 'y')  // Those were pointless
		{
			c = 'z';
		}
	}
}


OPENMPT_NAMESPACE_END
