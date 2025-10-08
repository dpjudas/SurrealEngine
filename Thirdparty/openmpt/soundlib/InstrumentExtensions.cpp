/*
 * InstrumentExtensions.cpp
 * ------------------------
 * Purpose: Instrument properties I/O
 * Notes  : Welcome to the absolutely horrible abominations that are the "extended instrument properties"
 *          which are some of the earliest additions OpenMPT did to the IT / XM format. They are ugly,
 *          and the way they work even differs between IT/XM/ITI/XI and ITI/XI/ITP.
 *          Yes, the world would be a better place without this stuff.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

#ifndef MODPLUG_NO_FILESAVE
#include "mpt/io/base.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"

#include <functional>
#endif

OPENMPT_NAMESPACE_BEGIN

/*---------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------
MODULAR (in/out) ModInstrument :
-----------------------------------------------------------------------------------------------

* to update:
------------

- both following functions need to be updated when adding a new member in ModInstrument:
  - SaveExtendedInstrumentProperties
  - ReadInstrumentHeaderField

- see below for body declaration.


* members:
----------

- 32bit identification CODE tag (must be unique)
- 16bit content SIZE in byte(s)
- member field


* CODE tag naming convention:
-----------------------------

- have a look below in current tag dictionnary
- take the initial ones of the field name
- 4 characters code (not more, not less)
- must be filled with '.' characters if code has less than 4 characters
- for arrays, must include a '[' character following significant characters ('.' not significant!!!)
- use only characters used in full member name, ordered as they appear in it
- match character attribute (small, capital)

Example with "PanEnv.nLoopEnd" , "PitchEnv.nLoopEnd" & "VolEnv.Values[MAX_ENVPOINTS]" members:
- use 'PLE.' for PanEnv.nLoopEnd
- use 'PiLE' for PitchEnv.nLoopEnd
- use 'VE[.' for VolEnv.Values[MAX_ENVPOINTS]


* In use CODE tag dictionary (alphabetical order):
--------------------------------------------------

AERN RW PanEnv.nReleaseNode
AFLG R  PanEnv.dwFlags
CS.. RW nCutSwing
DCT. R  nDCT
dF.. R  dwFlags
DNA. R  nDNA
FM.. RW filterMode
fn[. R  filename[12]
FO.. RW nFadeOut
GV.. R  nGlobalVol
IFC. R  nIFC
IFR. R  nIFR
K[..    Keyboard[128]
MB.. RW wMidiBank
MC.. RW nMidiChannel
MiP. RW nMixPlug
MP.. RW nMidiProgram
MPWD RW MIDI Pitch Wheel Depth
n[.. R  name[32]
NM[. R  NoteMap[128]
NNA. R  nNNA
P... RW nPan
PE.. RW PanEnv.nNodes
PE[. RW PanEnv.Values[MAX_ENVPOINTS]
PERN RW PitchEnv.nReleaseNode
PFLG R  PitchEnv.dwFlags
PiE. RW PitchEnv.nNodes
PiE[ RW PitchEnv.Values[MAX_ENVPOINTS]
PiLE R  PitchEnv.nLoopEnd
PiLS R  PitchEnv.nLoopStart
PiP[ RW PitchEnv.Ticks[MAX_ENVPOINTS]
PiSB R  PitchEnv.nSustainStart
PiSE R  PitchEnv.nSustainEnd
PLE. R  PanEnv.nLoopEnd
PLS. R  PanEnv.nLoopStart
PP[. RW PanEnv.Ticks[MAX_ENVPOINTS]
PPC. R  nPPC
PPS. R  nPPS
PS.. R  nPanSwing
PSB. R  PanEnv.nSustainStart
PSE. R  PanEnv.nSustainEnd
PTTF RW pitchToTempoLock (fractional part)
PTTL RW pitchToTempoLock (integer part)
PVEH RW pluginVelocityHandling
PVOH RW pluginVolumeHandling
R... RW Resampling
RS.. RW nResSwing
VE.. RW VolEnv.nNodes
VE[. RW VolEnv.Values[MAX_ENVPOINTS]
VERN RW VolEnv.nReleaseNode
VFLG R  VolEnv.dwFlags
VLE. R  VolEnv.nLoopEnd
VLS. R  VolEnv.nLoopStart
VP[. RW VolEnv.Ticks[MAX_ENVPOINTS]
VR.. RW nVolRampUp
VS.. R  nVolSwing
VSB. R  VolEnv.nSustainStart
VSE. R  VolEnv.nSustainEnd

Note that many of these extensions were only relevant for ITP files, and thus there is no code for writing them, only reading.
Some of them used to be written but were never read ("K[.." sample map - it was only relevant for ITP files, but even there
it was always ignored, because sample indices may change when loading external instruments).


-----------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------*/

#ifndef MODPLUG_NO_FILESAVE


// We want constexpr ModInstrument{} due to bad code generation with temporary objects mostly in MSVC,
// however most stdlib implementations of C++20 fail to provide constexpr std::vector in C++20 mode,
// which is required for the envelopes. Thus we only activate that for C++23.
// For libopenmpt, this code path is only required for test suite,
// and inefficient code generation does not really matter.
#if MPT_CXX_AT_LEAST(23) || (MPT_CXX_AT_LEAST(20) && !defined(MPT_LIBCXX_QUIRK_NO_CXX20_CONSTEXPR_CONTAINER))
#define MODINSTRUMENT_DEFAULT MPT_FORCE_CONSTEXPR_VALUE(ModInstrument{})
#elif defined(LIBOPENMPT_BUILD)
#define MODINSTRUMENT_DEFAULT ModInstrument{}
#else
#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif  // MPT_COMPILER_CLANG
static MPT_CONSTEXPR20_CONTAINER_VAR ModInstrument ModInstrumentDefault;
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif  // MPT_COMPILER_CLANG
#define MODINSTRUMENT_DEFAULT ModInstrumentDefault
#endif


template <auto Member>
inline bool IsPropertyNonDefault(const ModInstrument &ins) { return MODINSTRUMENT_DEFAULT.*Member != ins.*Member; }

template <auto Member>
constexpr uint16 PropertySize() noexcept { return sizeof(ModInstrument{}.*Member); }

template <auto Member, typename PropertyNeededFunc, auto PropertySizeFunc>
struct PropertyWriterBase
{
	PropertyNeededFunc IsPropertyNeeded;
	static constexpr auto Size = PropertySizeFunc;

	PropertyWriterBase(PropertyNeededFunc propertyNeededFunc = IsPropertyNonDefault<Member>)
		: IsPropertyNeeded{std::move(propertyNeededFunc)}
	{ }
};

template <auto Member, typename PropertyNeededFunc = decltype(&IsPropertyNonDefault<Member>), auto PropertySizeFunc = PropertySize<Member>>
struct PropertyWriterInt : PropertyWriterBase<Member, PropertyNeededFunc, PropertySizeFunc>
{
	using PropertyWriterBase<Member, PropertyNeededFunc, PropertySizeFunc>::PropertyWriterBase;
	static void Write(std::ostream &file, const ModInstrument &ins) { mpt::IO::WriteIntLE(file, ins.*Member); }
};

template <auto Member, typename PropertyNeededFunc = decltype(&IsPropertyNonDefault<Member>), auto PropertySizeFunc = PropertySize<Member>>
struct PropertyWriterEnum : PropertyWriterBase<Member, PropertyNeededFunc, PropertySizeFunc>
{
	using PropertyWriterBase<Member, PropertyNeededFunc, PropertySizeFunc>::PropertyWriterBase;
	static void Write(std::ostream &file, const ModInstrument &ins)
	{
		const auto value = ins.*Member;
		static_assert(std::is_enum_v<decltype(value)>);
		mpt::IO::WriteIntLE(file, mpt::to_underlying(value));
	}
};

struct PropertyWriterReleaseNode
{
	bool IsPropertyNeeded(const ModInstrument &ins) const noexcept { return MODINSTRUMENT_DEFAULT.GetEnvelope(type).nReleaseNode != ins.GetEnvelope(type).nReleaseNode; }
	static constexpr uint16 Size() noexcept { return sizeof(InstrumentEnvelope{}.nReleaseNode); }
	void Write(std::ostream &file, const ModInstrument &ins) const { mpt::IO::WriteIntLE(file, ins.GetEnvelope(type).nReleaseNode); }
	const EnvelopeType type;
};

struct PropertyWriterEnvelopeBase
{
	PropertyWriterEnvelopeBase(uint32 nodes, EnvelopeType type) : nodes{nodes}, type{type} {}
	static bool IsPropertyNeeded(const ModInstrument &) noexcept
	{
		return true;
	}
	const uint32 nodes;
	const EnvelopeType type;
};

struct PropertyWriterEnvelopeSize : PropertyWriterEnvelopeBase
{
	using PropertyWriterEnvelopeBase::PropertyWriterEnvelopeBase;
	static constexpr uint16 Size() noexcept { return sizeof(uint32le); }
	void Write(std::ostream &file, const ModInstrument &ins) const { mpt::IO::WriteIntLE<uint32>(file, ins.GetEnvelope(type).size()); }
};

struct PropertyWriterEnvelopeTicks : PropertyWriterEnvelopeBase
{
	using PropertyWriterEnvelopeBase::PropertyWriterEnvelopeBase;
	uint16 Size() const noexcept { return static_cast<uint16>(sizeof(uint16le) * nodes); }
	void Write(std::ostream &file, const ModInstrument &ins) const
	{
		const auto &env = ins.GetEnvelope(type);
		const uint32 maxNodes = std::min(nodes, static_cast<uint32>(env.size()));
		for(uint32 i = 0; i < maxNodes; ++i)
		{
			mpt::IO::WriteIntLE(file, static_cast<uint16>(env[i].tick));
		}
		// Not every instrument's envelope will be the same length. fill up with zeros.
		uint16le padding{};
		for(uint32 i = maxNodes; i < nodes; ++i)
		{
			mpt::IO::Write(file, padding);
		}
	}
};

struct PropertyWriterEnvelopeValues : PropertyWriterEnvelopeBase
{
	using PropertyWriterEnvelopeBase::PropertyWriterEnvelopeBase;
	uint16 Size() const noexcept { return static_cast<uint16>(sizeof(uint8) * nodes); }
	void Write(std::ostream &file, const ModInstrument &ins) const
	{
		const auto &env = ins.GetEnvelope(type);
		const uint32 maxNodes = std::min(nodes, static_cast<uint32>(env.size()));
		for(uint32 i = 0; i < maxNodes; ++i)
		{
			mpt::IO::WriteIntLE(file, static_cast<uint8>(env[i].value));
		}
		// Not every instrument's envelope will be the same length. fill up with zeros.
		uint8 padding{};
		for(uint32 i = maxNodes; i < nodes; ++i)
		{
			mpt::IO::Write(file, padding);
		}
	}
};

struct PropertyWriterPitchTempoLock
{
	static constexpr auto IsPropertyNeeded = IsPropertyNonDefault<&ModInstrument::pitchToTempoLock>;
	static constexpr uint16 Size() noexcept { return sizeof(uint16le); }
	PropertyWriterPitchTempoLock(bool intPart) : m_intPart{intPart} {}
	void Write(std::ostream &file, const ModInstrument &ins)
	{
		mpt::IO::WriteIntLE(file, static_cast<uint16>(m_intPart ? ins.pitchToTempoLock.GetInt() : ins.pitchToTempoLock.GetFract()));
	}

	const bool m_intPart;
};

template <typename PropertyWriter>
static void WriteProperty(std::ostream &f, uint32 code, mpt::span<const ModInstrument *const> instruments, PropertyWriter property)
{
	bool writeProperty = false;
	for(const ModInstrument *ins : instruments)
	{
		if(ins != nullptr && property.IsPropertyNeeded(*ins))
		{
			writeProperty = true;
			break;
		}
	}
	if(!writeProperty)
		return;
	mpt::IO::WriteIntLE<uint32>(f, code);
	mpt::IO::WriteIntLE<uint16>(f, property.Size());
	for(const ModInstrument *ins : instruments)
	{
		property.Write(f, ins ? *ins : MODINSTRUMENT_DEFAULT);
	}
}


void CSoundFile::SaveExtendedInstrumentProperties(INSTRUMENTINDEX instr, MODTYPE forceType, std::ostream &f) const
{
	const bool allInstruments = (instr < 1 || instr > GetNumInstruments());
	const auto instruments = mpt::as_span(Instruments).subspan(allInstruments ? 1 : instr, allInstruments ? GetNumInstruments() : 1);
	SaveExtendedInstrumentProperties(instruments, forceType, f, allInstruments);
}

void CSoundFile::SaveExtendedInstrumentProperties(mpt::span<const ModInstrument * const> instruments, MODTYPE forceType, std::ostream &f, bool allInstruments)
{
	uint32 code = MagicBE("MPTX");  // write extension header code
	mpt::IO::WriteIntLE<uint32>(f, code);

	WriteProperty(f, MagicBE("VR.."), instruments, PropertyWriterInt<&ModInstrument::nVolRampUp>{});
	WriteProperty(f, MagicBE("MiP."), instruments, PropertyWriterInt<&ModInstrument::nMixPlug>{});
	WriteProperty(f, MagicBE("R..."), instruments, PropertyWriterEnum<&ModInstrument::resampling>{});
	WriteProperty(f, MagicBE("PVEH"), instruments, PropertyWriterEnum<&ModInstrument::pluginVelocityHandling>{});
	WriteProperty(f, MagicBE("PVOH"), instruments, PropertyWriterEnum<&ModInstrument::pluginVolumeHandling>{});

	if(!(forceType & MOD_TYPE_XM))
	{
		// XM instrument headers already stores full-precision fade-out
		WriteProperty(f, MagicBE("FO.."), instruments, PropertyWriterInt<&ModInstrument::nFadeOut>{[](const ModInstrument &ins) { return (ins.nFadeOut % 32u) || ins.nFadeOut > 8192; }});
		// XM instrument headers already have support for this
		// Note: For ITI we always want to write this property, hence the allInstruments check
		int32 prevPWD = allInstruments ? int32_min : int32_max;
		WriteProperty(f, MagicBE("MPWD"), instruments, PropertyWriterInt<&ModInstrument::midiPWD, std::function<bool(const ModInstrument &)>>{[&prevPWD](const ModInstrument& ins)
		{
			if((prevPWD != int32_min && ins.midiPWD != prevPWD) || (ins.midiPWD < 0))
				return true;
			prevPWD = ins.midiPWD;
			return false;
		}});
		// We never supported these as hacks in XM (luckily!)
		WriteProperty(f, MagicBE("P..."), instruments, PropertyWriterInt<&ModInstrument::nPan>{[](const ModInstrument &ins) { return ins.dwFlags[INS_SETPANNING] && (ins.nPan % 4u); }});
		WriteProperty(f, MagicBE("CS.."), instruments, PropertyWriterInt<&ModInstrument::nCutSwing>{});
		WriteProperty(f, MagicBE("RS.."), instruments, PropertyWriterInt<&ModInstrument::nResSwing>{});
		WriteProperty(f, MagicBE("FM.."), instruments, PropertyWriterEnum<&ModInstrument::filterMode>{});
		WriteProperty(f, MagicBE("PTTL"), instruments, PropertyWriterPitchTempoLock{true});
		WriteProperty(f, MagicLE("PTTF"), instruments, PropertyWriterPitchTempoLock{false});
	} else
	{
		WriteProperty(f, MagicBE("MC.."), instruments, PropertyWriterInt<&ModInstrument::nMidiChannel>{[](const ModInstrument &ins) { return ins.nMidiChannel == MidiMappedChannel; }});
		// Can be saved in XM, but it's not possible to NOT save a MIDI program if a MIDI channel is set
		WriteProperty(f, MagicBE("MP.."), instruments, PropertyWriterInt<&ModInstrument::nMidiProgram>{[](const ModInstrument &ins) { return ins.HasValidMIDIChannel() == (ins.nMidiProgram == 0); }});
		WriteProperty(f, MagicBE("MB.."), instruments, PropertyWriterInt<&ModInstrument::wMidiBank>{});
	}

	if(forceType & MOD_TYPE_MPT)
	{
		uint32 maxNodes[3] = { 0, 0, 0 };
		for(const ModInstrument *ins : instruments)
		{
			if(ins == nullptr)
				continue;
			maxNodes[0] = std::max(maxNodes[0], ins->VolEnv.size());
			maxNodes[1] = std::max(maxNodes[1], ins->PanEnv.size());
			maxNodes[2] = std::max(maxNodes[2], ins->PitchEnv.size());
		}
		// write full envelope information for MPTM files (more env points)
		if(maxNodes[0] > 25)
		{
			WriteProperty(f, MagicBE("VE.."), instruments, PropertyWriterEnvelopeSize{maxNodes[0], ENV_VOLUME});
			WriteProperty(f, MagicBE("VP[."), instruments, PropertyWriterEnvelopeTicks{maxNodes[0], ENV_VOLUME});
			WriteProperty(f, MagicBE("VE[."), instruments, PropertyWriterEnvelopeValues{maxNodes[0], ENV_VOLUME});
		}
		if(maxNodes[1] > 25)
		{
			WriteProperty(f, MagicBE("PE.."), instruments, PropertyWriterEnvelopeSize{maxNodes[1], ENV_PANNING});
			WriteProperty(f, MagicBE("PP[."), instruments, PropertyWriterEnvelopeTicks{maxNodes[1], ENV_PANNING});
			WriteProperty(f, MagicBE("PE[."), instruments, PropertyWriterEnvelopeValues{maxNodes[1], ENV_PANNING});
		}
		if(maxNodes[2] > 25)
		{
			WriteProperty(f, MagicBE("PiE."), instruments, PropertyWriterEnvelopeSize{maxNodes[2], ENV_PITCH});
			WriteProperty(f, MagicBE("PiP["), instruments, PropertyWriterEnvelopeTicks{maxNodes[2], ENV_PITCH});
			WriteProperty(f, MagicBE("PiE["), instruments, PropertyWriterEnvelopeValues{maxNodes[2], ENV_PITCH});
		}
		WriteProperty(f, MagicBE("VERN"), instruments, PropertyWriterReleaseNode{ENV_VOLUME});
		WriteProperty(f, MagicBE("AERN"), instruments, PropertyWriterReleaseNode{ENV_PANNING});
		WriteProperty(f, MagicBE("PERN"), instruments, PropertyWriterReleaseNode{ENV_PITCH});
	}
}


#undef MODINSTRUMENT_DEFAULT


#endif // !MODPLUG_NO_FILESAVE


// Convert instrument flags which were read from 'dF..' extension to proper internal representation.
static void ConvertInstrumentFlags(ModInstrument &ins, uint32 flags)
{
	ins.VolEnv.dwFlags.set(ENV_ENABLED, (flags & 0x0001) != 0);
	ins.VolEnv.dwFlags.set(ENV_SUSTAIN, (flags & 0x0002) != 0);
	ins.VolEnv.dwFlags.set(ENV_LOOP, (flags & 0x0004) != 0);
	ins.VolEnv.dwFlags.set(ENV_CARRY, (flags & 0x0800) != 0);

	ins.PanEnv.dwFlags.set(ENV_ENABLED, (flags & 0x0008) != 0);
	ins.PanEnv.dwFlags.set(ENV_SUSTAIN, (flags & 0x0010) != 0);
	ins.PanEnv.dwFlags.set(ENV_LOOP, (flags & 0x0020) != 0);
	ins.PanEnv.dwFlags.set(ENV_CARRY, (flags & 0x1000) != 0);

	ins.PitchEnv.dwFlags.set(ENV_ENABLED, (flags & 0x0040) != 0);
	ins.PitchEnv.dwFlags.set(ENV_SUSTAIN, (flags & 0x0080) != 0);
	ins.PitchEnv.dwFlags.set(ENV_LOOP, (flags & 0x0100) != 0);
	ins.PitchEnv.dwFlags.set(ENV_CARRY, (flags & 0x2000) != 0);
	ins.PitchEnv.dwFlags.set(ENV_FILTER, (flags & 0x0400) != 0);

	ins.dwFlags.set(INS_SETPANNING, (flags & 0x0200) != 0);
	ins.dwFlags.set(INS_MUTE, (flags & 0x4000) != 0);
}


// Convert VFLG / PFLG / AFLG
static void ConvertEnvelopeFlags(ModInstrument &instr, uint32 flags, EnvelopeType envType)
{
	InstrumentEnvelope &env = instr.GetEnvelope(envType);
	env.dwFlags.set(ENV_ENABLED, (flags & 0x01) != 0);
	env.dwFlags.set(ENV_LOOP, (flags & 0x02) != 0);
	env.dwFlags.set(ENV_SUSTAIN, (flags & 0x04) != 0);
	env.dwFlags.set(ENV_CARRY, (flags & 0x08) != 0);
	env.dwFlags.set(ENV_FILTER, (envType == ENV_PITCH) && (flags & 0x10) != 0);
}


static void ReadInstrumentHeaderField(ModInstrument &ins, uint32 fcode, FileReader &file)
{
	const size_t size = static_cast<size_t>(file.GetLength());

	// Note: Various int / enum members have changed their size over the past.
	// Hence we use ReadSizedIntLE everywhere to allow reading both truncated and oversized values.
	constexpr auto ReadInt = [](FileReader &file, auto size, auto &member)
	{
		using T = std::remove_reference_t<decltype(member)>;
		member = file.ReadSizedIntLE<T>(size);
	};
	constexpr auto ReadEnum = [](FileReader &file, auto size, auto &member)
	{
		using T = std::remove_reference_t<decltype(member)>;
		static_assert(std::is_enum_v<T>);
		member = static_cast<T>(file.ReadSizedIntLE<std::underlying_type_t<T>>(size));
	};
	constexpr auto ReadEnvelopeTicks = [](FileReader &file, auto size, InstrumentEnvelope &env)
	{
		const uint32 points = std::min(env.size(), static_cast<uint32>(size / 2));
		for(uint32 i = 0; i < points; i++)
		{
			env[i].tick = file.ReadUint16LE();
		}
	};
	constexpr auto ReadEnvelopeValues = [](FileReader &file, auto size, InstrumentEnvelope &env)
	{
		const uint32 points = std::min(env.size(), static_cast<uint32>(size));
		for(uint32 i = 0; i < points; i++)
		{
			env[i].value = file.ReadUint8();
		}
	};

	// Members which can be found in this table but not in the write table are only required in the legacy ITP format.
	switch(fcode)
	{
	case MagicBE("FO.."): ReadInt(file, size, ins.nFadeOut); break;
	case MagicBE("GV.."): ReadInt(file, size, ins.nGlobalVol); break;
	case MagicBE("P..."): ReadInt(file, size, ins.nPan); break;
	case MagicBE("VLS."): ReadInt(file, size, ins.VolEnv.nLoopStart); break;
	case MagicBE("VLE."): ReadInt(file, size, ins.VolEnv.nLoopEnd); break;
	case MagicBE("VSB."): ReadInt(file, size, ins.VolEnv.nSustainStart); break;
	case MagicBE("VSE."): ReadInt(file, size, ins.VolEnv.nSustainEnd); break;
	case MagicBE("PLS."): ReadInt(file, size, ins.PanEnv.nLoopStart); break;
	case MagicBE("PLE."): ReadInt(file, size, ins.PanEnv.nLoopEnd); break;
	case MagicBE("PSB."): ReadInt(file, size, ins.PanEnv.nSustainStart); break;
	case MagicBE("PSE."): ReadInt(file, size, ins.PanEnv.nSustainEnd); break;
	case MagicBE("PiLS"): ReadInt(file, size, ins.PitchEnv.nLoopStart); break;
	case MagicBE("PiLE"): ReadInt(file, size, ins.PitchEnv.nLoopEnd); break;
	case MagicBE("PiSB"): ReadInt(file, size, ins.PitchEnv.nSustainStart); break;
	case MagicBE("PiSE"): ReadInt(file, size, ins.PitchEnv.nSustainEnd); break;
	case MagicBE("NNA."): ReadEnum(file, size, ins.nNNA); break;
	case MagicBE("DCT."): ReadEnum(file, size, ins.nDCT); break;
	case MagicBE("DNA."): ReadEnum(file, size, ins.nDNA); break;
	case MagicBE("PS.."): ReadInt(file, size, ins.nPanSwing); break;
	case MagicBE("VS.."): ReadInt(file, size, ins.nVolSwing); break;
	case MagicBE("IFC."): ReadInt(file, size, ins.nIFC); break;
	case MagicBE("IFR."): ReadInt(file, size, ins.nIFR); break;
	case MagicBE("MB.."): ReadInt(file, size, ins.wMidiBank); break;
	case MagicBE("MP.."): ReadInt(file, size, ins.nMidiProgram); break;
	case MagicBE("MC.."): ReadInt(file, size, ins.nMidiChannel); break;
	case MagicBE("PPS."): ReadInt(file, size, ins.nPPS); break;
	case MagicBE("PPC."): ReadInt(file, size, ins.nPPC); break;
	case MagicBE("VP[."): ReadEnvelopeTicks(file, size, ins.VolEnv); break;
	case MagicBE("PP[."): ReadEnvelopeTicks(file, size, ins.PanEnv); break;
	case MagicBE("PiP["): ReadEnvelopeTicks(file, size, ins.PitchEnv); break;
	case MagicBE("VE[."): ReadEnvelopeValues(file, size, ins.VolEnv); break;
	case MagicBE("PE[."): ReadEnvelopeValues(file, size, ins.PanEnv); break;
	case MagicBE("PiE["): ReadEnvelopeValues(file, size, ins.PitchEnv); break;
	case MagicBE("MiP."): ReadInt(file, size, ins.nMixPlug); break;
	case MagicBE("VR.."): ReadInt(file, size, ins.nVolRampUp); break;
	case MagicBE("CS.."): ReadInt(file, size, ins.nCutSwing); break;
	case MagicBE("RS.."): ReadInt(file, size, ins.nResSwing); break;
	case MagicBE("FM.."): ReadEnum(file, size, ins.filterMode); break;
	case MagicBE("PVEH"): ReadEnum(file, size, ins.pluginVelocityHandling); break;
	case MagicBE("PVOH"): ReadEnum(file, size, ins.pluginVolumeHandling); break;
	case MagicBE("PERN"): ReadInt(file, size, ins.PitchEnv.nReleaseNode); break;
	case MagicBE("AERN"): ReadInt(file, size, ins.PanEnv.nReleaseNode); break;
	case MagicBE("VERN"): ReadInt(file, size, ins.VolEnv.nReleaseNode); break;
	case MagicBE("MPWD"): ReadInt(file, size, ins.midiPWD); break;
	case MagicBE("dF.."):
		ConvertInstrumentFlags(ins, file.ReadSizedIntLE<uint32>(size));
		break;
	case MagicBE("VFLG"):
		ConvertEnvelopeFlags(ins, file.ReadSizedIntLE<uint32>(size), ENV_VOLUME);
		break;
	case MagicBE("AFLG"):
		ConvertEnvelopeFlags(ins, file.ReadSizedIntLE<uint32>(size), ENV_PANNING);
		break;
	case MagicBE("PFLG"):
		ConvertEnvelopeFlags(ins, file.ReadSizedIntLE<uint32>(size), ENV_PITCH);
		break;
	case MagicBE("NM[."):
		for(std::size_t i = 0; i < std::min(size, ins.NoteMap.size()); i++)
		{
			ins.NoteMap[i] = file.ReadUint8();
		}
		break;
	case MagicBE("n[.."):
		{
			char name[32] = "";
			file.ReadString<mpt::String::maybeNullTerminated>(name, size);
			ins.name = name;
		}
		break;
	case MagicBE("fn[."):
		{
			char filename[32] = "";
			file.ReadString<mpt::String::maybeNullTerminated>(filename, size);
			ins.filename = filename;
		}
		break;
	case MagicBE("R..."):
		// Resampling has been written as various sizes including uint16 and uint32 in the past
		if(uint32 resampling = file.ReadSizedIntLE<uint32>(size); Resampling::IsKnownMode(resampling))
			ins.resampling = static_cast<ResamplingMode>(resampling);
		break;
	case MagicBE("PTTL"):
		// Integer part of pitch/tempo lock
		ins.pitchToTempoLock.Set(file.ReadSizedIntLE<uint16>(size), ins.pitchToTempoLock.GetFract());
		break;
	case MagicLE("PTTF"):
		// Fractional part of pitch/tempo lock
		ins.pitchToTempoLock.Set(ins.pitchToTempoLock.GetInt(), file.ReadSizedIntLE<uint16>(size));
		break;
	case MagicBE("VE.."):
		ins.VolEnv.resize(std::min(uint32(MAX_ENVPOINTS), file.ReadSizedIntLE<uint32>(size)));
		break;
	case MagicBE("PE.."):
		ins.PanEnv.resize(std::min(uint32(MAX_ENVPOINTS), file.ReadSizedIntLE<uint32>(size)));
		break;
	case MagicBE("PiE."):
		ins.PitchEnv.resize(std::min(uint32(MAX_ENVPOINTS), file.ReadSizedIntLE<uint32>(size)));
		break;
	}
}


// For ITP and internal usage
void CSoundFile::ReadExtendedInstrumentProperty(mpt::span<ModInstrument *> instruments, const uint32 code, FileReader &file)
{
	uint16 size = file.ReadUint16LE();
	for(ModInstrument *ins : instruments)
	{
		FileReader chunk = file.ReadChunk(size);
		if(ins && chunk.GetLength() == size)
			ReadInstrumentHeaderField(*ins, code, chunk);
	}
}


// For IT / XM / MO3 / ITI / XI
bool CSoundFile::LoadExtendedInstrumentProperties(mpt::span<ModInstrument *> instruments, FileReader &file)
{
	if(!file.ReadMagic("XTPM"))  // 'MPTX'
		return false;

	while(file.CanRead(6))
	{
		uint32 code = file.ReadUint32LE();

		if(code == MagicBE("MPTS")                          // Reached song extensions, break out of this loop
		   || code == MagicLE("228\x04")                    // Reached MPTM extensions (in case there are no song extensions)
		   || (code & 0x80808080) || !(code & 0x60606060))  // Non-ASCII chunk ID
		{
			file.SkipBack(4);
			break;
		}

		ReadExtendedInstrumentProperty(instruments, code, file);
	}
	return true;
}


OPENMPT_NAMESPACE_END
