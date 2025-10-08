/*
 * OPL.cpp
 * -------
 * Purpose: Translate data coming from OpenMPT's mixer into OPL commands to be sent to the Opal emulator.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 *          Schism Tracker contributors (bisqwit, JosepMa, Malvineous, code relicensed from GPL to BSD with permission)
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"
#include "OPL.h"
#include "../common/misc_util.h"

#include <cstdint>
#if MPT_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include "opal.h"
#if MPT_COMPILER_GCC
#pragma GCC diagnostic pop
#endif

OPENMPT_NAMESPACE_BEGIN

OPL::OPL(uint32 sampleRate)
{
	Initialize(sampleRate);
}


OPL::OPL(IRegisterLogger &logger)
	: m_logger{&logger}
{
	Initialize(OPL_BASERATE);
}


OPL::~OPL()
{
	// This destructor is put here so that we can forward-declare the Opal emulator class.
}


void OPL::Initialize(uint32 sampleRate)
{
	if(m_opl == nullptr)
		m_opl = std::make_unique<Opal>(sampleRate);
	else
		m_opl->SetSampleRate(sampleRate);
	Reset();
}


void OPL::Mix(int32 *target, size_t count, uint32 volumeFactorQ16)
{
	if(!m_isActive)
		return;

	// This factor causes a sample voice to be more or less as loud as an OPL voice
	const int32 factor = Util::muldiv_unsigned(volumeFactorQ16, 6169, (1 << 16));
	while(count--)
	{
		int16 l, r;
		m_opl->Sample(&l, &r);
		target[0] += l * factor;
		target[1] += r * factor;
		target += 2;
	}
}


OPL::Register OPL::ChannelToRegister(uint8 oplCh)
{
	if(oplCh < 9)
		return oplCh;
	else
		return (oplCh - 9) | 0x100;
}


// Translate a channel's first operator address into a register
OPL::Register OPL::OperatorToRegister(uint8 oplCh)
{
	static constexpr uint8 OPLChannelToOperator[] = { 0, 1, 2, 8, 9, 10, 16, 17, 18 };
	if(oplCh < 9)
		return OPLChannelToOperator[oplCh];
	else
		return OPLChannelToOperator[oplCh - 9] | 0x100;
}


uint8 OPL::GetVoice(CHANNELINDEX c) const
{
	if((m_ChanToOPL[c] & OPL_CHANNEL_CUT) || m_ChanToOPL[c] == OPL_CHANNEL_INVALID)
		return OPL_CHANNEL_INVALID;
	return m_ChanToOPL[c] & OPL_CHANNEL_MASK;
}


uint8 OPL::AllocateVoice(CHANNELINDEX c)
{
	// Can we re-use a previous channel?
	if(auto oplCh = m_ChanToOPL[c]; oplCh != OPL_CHANNEL_INVALID)
	{
		if(!(m_ChanToOPL[c] & OPL_CHANNEL_CUT))
			return oplCh;
		// Check re-use hint
		oplCh &= OPL_CHANNEL_MASK;
		if(m_OPLtoChan[oplCh] == CHANNELINDEX_INVALID || m_OPLtoChan[oplCh] == c)
		{
			m_OPLtoChan[oplCh] = c;
			m_ChanToOPL[c] = oplCh;
			return oplCh;
		}
	}
	// Search for unused channel or channel with released note
	uint8 releasedChn = OPL_CHANNEL_INVALID, releasedCutChn = OPL_CHANNEL_INVALID;
	for(uint8 oplCh = 0; oplCh < OPL_CHANNELS; oplCh++)
	{
		if(m_OPLtoChan[oplCh] == CHANNELINDEX_INVALID)
		{
			m_OPLtoChan[oplCh] = c;
			m_ChanToOPL[c] = oplCh;
			return oplCh;
		} else if(!(m_KeyOnBlock[oplCh] & KEYON_BIT))
		{
			releasedChn = oplCh;
			if(m_ChanToOPL[m_OPLtoChan[oplCh]] & OPL_CHANNEL_CUT)
				releasedCutChn = oplCh;
		}
	}
	if(releasedChn != OPL_CHANNEL_INVALID)
	{
		// Prefer channel that has been marked as cut over channel that has just been released
		if(releasedCutChn != OPL_CHANNEL_INVALID)
			releasedChn = releasedCutChn;
		m_ChanToOPL[m_OPLtoChan[releasedChn]] = OPL_CHANNEL_INVALID;
		m_OPLtoChan[releasedChn] = c;
		m_ChanToOPL[c] = releasedChn;
	}
	return GetVoice(c);
}


void OPL::MoveChannel(CHANNELINDEX from, CHANNELINDEX to)
{
	uint8 oplCh = GetVoice(from);
	if(oplCh == OPL_CHANNEL_INVALID)
		return;
	m_OPLtoChan[oplCh] = to;
	m_ChanToOPL[from] = OPL_CHANNEL_INVALID;
	m_ChanToOPL[to] = oplCh;
	if(m_logger)
		m_logger->MoveChannel(from, to);
}


void OPL::NoteOff(CHANNELINDEX c)
{
	uint8 oplCh = GetVoice(c);
	if(oplCh == OPL_CHANNEL_INVALID || m_opl == nullptr)
		return;
	if(!(m_KeyOnBlock[oplCh] & KEYON_BIT))
		return;
	m_KeyOnBlock[oplCh] &= ~KEYON_BIT;
	Port(c, KEYON_BLOCK | ChannelToRegister(oplCh), m_KeyOnBlock[oplCh]);
}


void OPL::NoteCut(CHANNELINDEX c, bool unassign)
{
	uint8 oplCh = GetVoice(c);
	if(oplCh == OPL_CHANNEL_INVALID)
		return;
	NoteOff(c);
	Volume(c, 0, false);  // Note that a volume of 0 is not complete silence; the release portion of the sound will still be heard at -48dB
	if(unassign)
	{
		m_OPLtoChan[oplCh] = CHANNELINDEX_INVALID;
		m_ChanToOPL[c] |= OPL_CHANNEL_CUT;
	}
}


void OPL::Frequency(CHANNELINDEX c, uint32 milliHertz, bool keyOff, bool beatingOscillators)
{
	uint8 oplCh = GetVoice(c);
	if(oplCh == OPL_CHANNEL_INVALID || m_opl == nullptr)
		return;

	uint16 fnum = 1023;
	uint8 block = 7;
	if(milliHertz <= 6208431)
	{
		if(milliHertz > 3104215) block = 7;
		else if(milliHertz > 1552107) block = 6;
		else if(milliHertz > 776053) block = 5;
		else if(milliHertz > 388026) block = 4;
		else if(milliHertz > 194013) block = 3;
		else if(milliHertz > 97006) block = 2;
		else if(milliHertz > 48503) block = 1;
		else block = 0;

		fnum = static_cast<uint16>(Util::muldivr_unsigned(milliHertz, 1 << (20 - block), OPL_BASERATE * 1000));
		MPT_ASSERT(fnum < 1024);
	}

	// Evil CDFM hack! Composer 670 slightly detunes each note based on the OPL channel number modulo 4.
	// We allocate our OPL channels dynamically, which would result in slightly different beating characteristics,
	// but we can just take the pattern channel number instead, as the pattern channel layout is always identical.
	if(beatingOscillators)
		fnum = std::min(static_cast<uint16>(fnum + (c & 3)), uint16(1023));

	fnum |= (block << 10);

	OPL::Register channel = ChannelToRegister(oplCh);
	m_KeyOnBlock[oplCh] = static_cast<uint8>((keyOff ? 0 : KEYON_BIT) | (fnum >> 8));  // Key on bit + Octave (block) + F-number high 2 bits
	Port(c, FNUM_LOW    | channel, fnum & 0xFFu);                                      // F-Number low 8 bits
	Port(c, KEYON_BLOCK | channel, m_KeyOnBlock[oplCh]);

	m_isActive = true;
}


uint8 OPL::CalcVolume(uint8 trackerVol, uint8 kslVolume)
{
	if(trackerVol >= 63u)
		return kslVolume;
	if(trackerVol > 0)
		trackerVol++;
	return static_cast<uint8>((kslVolume & KSL_MASK) | (63u - ((63u - (kslVolume & TOTAL_LEVEL_MASK)) * trackerVol) / 64u));
}


void OPL::Volume(CHANNELINDEX c, uint8 vol, bool applyToModulator)
{
	uint8 oplCh = GetVoice(c);
	if(oplCh == OPL_CHANNEL_INVALID || m_opl == nullptr)
		return;

	const auto &patch = m_Patches[oplCh];
	const OPL::Register modulator = OperatorToRegister(oplCh), carrier = modulator + 3;
	if((patch[10] & CONNECTION_BIT) || applyToModulator)
	{
		// Set volume of both operators in additive mode
		Port(c, KSL_LEVEL + modulator, CalcVolume(vol, patch[2]));
	}
	if(!applyToModulator)
	{
		Port(c, KSL_LEVEL + carrier, CalcVolume(vol, patch[3]));
	}
}


int8 OPL::Pan(CHANNELINDEX c, int32 pan)
{
	uint8 oplCh = GetVoice(c);
	if(oplCh == OPL_CHANNEL_INVALID || m_opl == nullptr)
		return 0;

	const auto &patch = m_Patches[oplCh];
	uint8 fbConn = patch[10] & ~STEREO_BITS;
	// OPL3 only knows hard left, center and right, so we need to translate our
	// continuous panning range into one of those three states.
	// 0...84 = left, 85...170 = center, 171...256 = right
	if(pan <= 170)
		fbConn |= VOICE_TO_LEFT;
	if(pan >= 85)
		fbConn |= VOICE_TO_RIGHT;

	Port(c, FEEDBACK_CONNECTION | ChannelToRegister(oplCh), fbConn);
	return static_cast<int8>(((fbConn & VOICE_TO_LEFT) ? -1 : 0) + ((fbConn & VOICE_TO_RIGHT) ? 1 : 0));
}


void OPL::Patch(CHANNELINDEX c, const OPLPatch &patch)
{
	uint8 oplCh = AllocateVoice(c);
	if(oplCh == OPL_CHANNEL_INVALID || m_opl == nullptr)
		return;

	m_Patches[oplCh] = patch;

	const OPL::Register modulator = OperatorToRegister(oplCh), carrier = modulator + 3;
	for(uint8 op = 0; op < 2; op++)
	{
		const auto opReg = op ? carrier : modulator;
		Port(c, AM_VIB          | opReg, patch[0 + op]);
		Port(c, KSL_LEVEL       | opReg, patch[2 + op]);
		Port(c, ATTACK_DECAY    | opReg, patch[4 + op]);
		Port(c, SUSTAIN_RELEASE | opReg, patch[6 + op]);
		Port(c, WAVE_SELECT     | opReg, patch[8 + op]);
	}

	Port(c, FEEDBACK_CONNECTION | ChannelToRegister(oplCh), patch[10]);
}


void OPL::Reset()
{
	if(m_isActive)
	{
		for(CHANNELINDEX chn = 0; chn < MAX_CHANNELS; chn++)
		{
			NoteCut(chn);
		}
		m_isActive = false;
	}

	m_KeyOnBlock.fill(0);
	m_OPLtoChan.fill(CHANNELINDEX_INVALID);
	m_ChanToOPL.fill(OPL_CHANNEL_INVALID);

	Port(CHANNELINDEX_INVALID, 0x105, 1);  // Enable OPL3
	Port(CHANNELINDEX_INVALID, 0x104, 0);  // No 4-op voices
}


void OPL::Port(CHANNELINDEX c, OPL::Register reg, OPL::Value value)
{
	if(!m_logger)
		m_opl->Port(reg, value);
	else
		m_logger->Port(c, reg, value);
}


std::vector<OPL::Register> OPL::AllVoiceRegisters(uint8 oplCh)
{
	static constexpr uint8 opRegisters[] = {AM_VIB, KSL_LEVEL, ATTACK_DECAY, SUSTAIN_RELEASE, WAVE_SELECT};
	static constexpr uint8 chnRegisters[] = {FNUM_LOW, KEYON_BLOCK, FEEDBACK_CONNECTION};
	std::vector<OPL::Register> result;
	uint8 minVoice = 0, maxVoice = OPL_CHANNELS;
	if(oplCh < OPL_CHANNELS)
	{
		minVoice = oplCh;
		maxVoice = oplCh + 1;
	}
	result.reserve(13 * (maxVoice - minVoice));
	for(uint8 voice = minVoice; voice < maxVoice; voice++)
	{
		const Register opBaseReg = OperatorToRegister(voice);
		for(uint8 opReg : opRegisters)
		{
			for(uint8 op = 0; op <= 3; op += 3)
			{
				result.push_back(opReg | (opBaseReg + op));
				MPT_ASSERT(RegisterToVoice(result.back()) == voice);
			}
		}
		const Register chnBaseReg = ChannelToRegister(voice);
		for(uint8 chnReg : chnRegisters)
		{
			result.push_back(chnReg | chnBaseReg);
			MPT_ASSERT(RegisterToVoice(result.back()) == voice);
		}
	}
	return result;
}


uint8 OPL::RegisterToVoice(OPL::Register reg)
{
	const OPL::Register regLo = reg & 0xE0;
	const uint8 baseCh = (reg > 0xFF) ? 9 : 0;
	if(reg == TREMOLO_VIBRATO_DEPTH)
		return 0xFF;
	if(regLo >= FNUM_LOW && regLo <= FEEDBACK_CONNECTION)
		return baseCh + static_cast<uint8>(reg & 0x0F);
	if(regLo >= AM_VIB && regLo <= WAVE_SELECT)
		return static_cast<uint8>(baseCh + (reg & 0x07) % 3u + ((reg & 0x1F) >> 3) * 3);
	return 0xFF;
}


OPL::Register OPL::StripVoiceFromRegister(OPL::Register reg)
{
	const OPL::Register regLo = reg & 0xE0;
	if(reg == TREMOLO_VIBRATO_DEPTH)
		return reg;
	if(regLo >= FNUM_LOW && regLo <= FEEDBACK_CONNECTION)
		return (reg & 0xF0);
	if(regLo >= AM_VIB && regLo <= WAVE_SELECT)
		return static_cast<OPL::Register>(regLo + ((reg & 0x07) >= 3 ? 3 : 0));
	return reg;
}


OPENMPT_NAMESPACE_END
