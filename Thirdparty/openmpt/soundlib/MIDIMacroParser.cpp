/*
 * MIDIMacroParser.cpp
 * -------------------
 * Purpose: Class for parsing IT MIDI macro strings and splitting them into individual raw MIDI messages.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "MIDIMacroParser.h"
#include "MIDIEvents.h"
#include "Sndfile.h"
#include "plugins/PlugInterface.h"

OPENMPT_NAMESPACE_BEGIN


bool MIDIMacroParser::NextMessage(mpt::span<uint8> &message, bool outputRunningStatus)
{
	if(m_runningStatusPos < m_data.size())
	{
		m_data[m_runningStatusPos] = m_runningstatusOldData;
		m_runningStatusPos = uint32_max;
	}
	const uint32 outSize = static_cast<uint32>(m_data.size());
	while(m_sendPos < outSize)
	{
		uint32 sendLen = 0;
		if(m_data[m_sendPos] == 0xF0)
		{
			// SysEx start
			if((outSize - m_sendPos >= 4) && (m_data[m_sendPos + 1] == 0xF0 || m_data[m_sendPos + 1] == 0xF1))
			{
				// Internal macro (normal (F0F0) or extended (F0F1)), 4 bytes long
				sendLen = 4;
			} else
			{
				// SysEx message, find end of message
				sendLen = outSize - m_sendPos;
				for(uint32 i = m_sendPos + 1; i < outSize; i++)
				{
					if(m_data[i] == 0xF7)
					{
						// Found end of SysEx message
						sendLen = i - m_sendPos + 1;
						break;
					}
				}
			}
		} else if(!(m_data[m_sendPos] & 0x80))
		{
			// Missing status byte? Try inserting running status
			if(m_runningStatus != 0)
			{
				if(outputRunningStatus)
				{
					m_sendPos--;
					m_runningstatusOldData = m_data[m_sendPos];
					m_data[m_sendPos] = m_runningStatus;
					m_runningStatusPos = m_sendPos;
					continue;
				} else
				{
					sendLen = std::min(static_cast<uint32>(MIDIEvents::GetEventLength(m_runningStatus) - 1), outSize - m_sendPos);
				}
			} else
			{
				// No running status to re-use; skip this byte
				m_sendPos++;
				continue;
			}
		} else
		{
			// Other MIDI messages
			sendLen = std::min(static_cast<uint32>(MIDIEvents::GetEventLength(m_data[m_sendPos])), outSize - m_sendPos);
		}

		if(sendLen == 0)
			break;

		if(m_data[m_sendPos] < 0xF0)
			m_runningStatus = m_data[m_sendPos];
		
		message = m_data.subspan(m_sendPos, sendLen);
		m_sendPos += sendLen;
		return true;
	}
	message = {};
	return false;
}


MIDIMacroParser::MIDIMacroParser(const CSoundFile &sndFile, PlayState *playState, CHANNELINDEX nChn, bool isSmooth, const mpt::span<const char> macro, mpt::span<uint8> out, uint8 param, PLUGINDEX plugin)
	: m_data{out}
{
	// Need to be able to add potentially missing F7 (End Of SysEx)
	MPT_ASSERT(out.size() > macro.size());
	ModChannel *chn = (playState && nChn < playState->Chn.size()) ? &playState->Chn[nChn] : nullptr;
	const ModInstrument *pIns = chn ? chn->pModInstrument : nullptr;
	MPT_ASSERT(!isSmooth || chn);  // If we want to interpolate, we need access to the channel.

	const uint8 lastZxxParam = chn ? chn->lastZxxParam : 0xFF;  // always interpolate based on original value in case z appears multiple times in macro string
	uint8 updateZxxParam = 0xFF;                                // avoid updating lastZxxParam immediately if macro contains both internal and external MIDI message

	bool firstNibble = true;
	size_t outPos = 0;  // output buffer position, which also equals the number of complete bytes
	for(size_t pos = 0; pos < macro.size() && outPos < out.size(); pos++)
	{
		bool isNibble = false;  // did we parse a nibble or a byte value?
		uint8 data = 0;         // data that has just been parsed

		// Parse next macro byte... See Impulse Tracker's MIDI.TXT for detailed information on each possible character.
		if(macro[pos] >= '0' && macro[pos] <= '9')
		{
			isNibble = true;
			data = static_cast<uint8>(macro[pos] - '0');
		} else if(macro[pos] >= 'A' && macro[pos] <= 'F')
		{
			isNibble = true;
			data = static_cast<uint8>(macro[pos] - 'A' + 0x0A);
		} else if(macro[pos] == 'c')
		{
			// MIDI channel
			isNibble = true;
			data = 0xFF;
#ifndef NO_PLUGINS
			const PLUGINDEX plug = (plugin != 0 || !chn) ? plugin : sndFile.GetBestPlugin(*chn, nChn, PrioritiseChannel, EvenIfMuted);
			if(plug > 0 && plug <= MAX_MIXPLUGINS)
			{
				auto midiPlug = dynamic_cast<const IMidiPlugin *>(sndFile.m_MixPlugins[plug - 1u].pMixPlugin);
				if(midiPlug && chn)
					data = midiPlug->GetMidiChannel(*chn, nChn);
			}
#endif // NO_PLUGINS
			if(data == 0xFF)
			{
				// Fallback if no plugin was found
				if(pIns && chn)
					data = pIns->GetMIDIChannel(*chn, nChn);
				else
					data = 0;
			}
		} else if(macro[pos] == 'n')
		{
			// Last triggered note
			if(chn && ModCommand::IsNote(chn->nLastNote))
				data = chn->nLastNote - NOTE_MIN;
		} else if(macro[pos] == 'v')
		{
			// Velocity
			// This is "almost" how IT does it - apparently, IT seems to lag one row behind on global volume or channel volume changes.
			if(chn && playState)
			{
				const int swing = (sndFile.m_playBehaviour[kITSwingBehaviour] || sndFile.m_playBehaviour[kMPTOldSwingBehaviour]) ? chn->nVolSwing : 0;
				const int vol = Util::muldiv((chn->nVolume + swing) * playState->m_nGlobalVolume, chn->nGlobalVol * chn->nInsVol, 1 << 20);
				data = static_cast<uint8>(Clamp(vol / 2, 1, 127));
				//data = static_cast<uint8>(std::min((chn->nVolume * chn->nGlobalVol * playState->m_nGlobalVolume) >> (1 + 6 + 8), 127));
			}
		} else if(macro[pos] == 'u')
		{
			// Calculated volume
			// Same note as with velocity applies here, but apparently also for instrument / sample volumes?
			if(chn && playState)
			{
				const int vol = Util::muldiv(chn->nCalcVolume * playState->m_nGlobalVolume, chn->nGlobalVol * chn->nInsVol, 1 << 26);
				data = static_cast<uint8>(Clamp(vol / 2, 1, 127));
				//data = static_cast<uint8>(std::min((chn->nCalcVolume * chn->nGlobalVol * playState->m_nGlobalVolume) >> (7 + 6 + 8), 127));
			}
		} else if(macro[pos] == 'x')
		{
			// Pan set
			if(chn)
				data = static_cast<uint8>(std::min(static_cast<int>(chn->nPan / 2), 127));
		} else if(macro[pos] == 'y')
		{
			// Calculated pan
			if(chn)
				data = static_cast<uint8>(std::min(static_cast<int>(chn->nRealPan / 2), 127));
		} else if(macro[pos] == 'a')
		{
			// High byte of bank select
			if(pIns && pIns->wMidiBank)
			{
				data = static_cast<uint8>(((pIns->wMidiBank - 1) >> 7) & 0x7F);
			}
		} else if(macro[pos] == 'b')
		{
			// Low byte of bank select
			if(pIns && pIns->wMidiBank)
			{
				data = static_cast<uint8>((pIns->wMidiBank - 1) & 0x7F);
			}
		} else if(macro[pos] == 'o')
		{
			// Offset (ignoring high offset)
			if(chn)
				data = static_cast<uint8>((chn->oldOffset >> 8) & 0xFF);
		} else if(macro[pos] == 'h')
		{
			// Host channel number
			if(chn)
				data = static_cast<uint8>((nChn >= sndFile.GetNumChannels() ? (chn->nMasterChn - 1) : nChn) & 0x7F);
		} else if(macro[pos] == 'm')
		{
			// Loop direction (on sample channels - MIDI note on MIDI channels)
			if(chn)
				data = chn->dwFlags[CHN_PINGPONGFLAG] ? 1 : 0;
		} else if(macro[pos] == 'p')
		{
			// Program select
			if(pIns && pIns->nMidiProgram)
			{
				data = static_cast<uint8>((pIns->nMidiProgram - 1) & 0x7F);
			}
		} else if(macro[pos] == 'z')
		{
			// Zxx parameter
			data = param;
			if(isSmooth && playState && chn && chn->lastZxxParam < 0x80
				&& (outPos < 3 || out[outPos - 3] != 0xF0 || out[outPos - 2] < 0xF0))
			{
				// Interpolation for external MIDI messages - interpolation for internal messages
				// is handled separately to allow for more than 7-bit granularity where it's possible
				data = static_cast<uint8>(CSoundFile::CalculateSmoothParamChange(*playState, lastZxxParam, data));
				chn->lastZxxParam = data;
				updateZxxParam = 0x80;
			} else if(updateZxxParam == 0xFF)
			{
				updateZxxParam = data;
			}
		} else if(macro[pos] == 's')
		{
			// SysEx Checksum (not an original Impulse Tracker macro variable, but added for convenience)
			if(!firstNibble)  // From MIDI.TXT: '9n' is exactly the same as '09 n' or '9 n' -- so finish current byte first
			{
				outPos++;
				firstNibble = true;
			}

			auto startPos = outPos;
			while(startPos > 0 && out[--startPos] != 0xF0)
				;

			if(outPos - startPos < 3 || out[startPos] != 0xF0)
				continue;

			// If first byte of model number is 0, read one more
			uint8 checksumStart = out[startPos + 3] ? 5 : 6;
			if(outPos - startPos < checksumStart)
				continue;

			for(auto p = startPos + checksumStart; p != outPos; p++)
			{
				data += out[p];
			}
			data = (~data + 1) & 0x7F;
		} else
		{
			// Unrecognized byte (e.g. space char)
			continue;
		}

		// Append parsed data
		if(isNibble)  // parsed a nibble (constant or 'c' variable)
		{
			if(firstNibble)
			{
				out[outPos] = data;
			} else
			{
				out[outPos] = (out[outPos] << 4) | data;
				outPos++;
			}
			firstNibble = !firstNibble;
		} else  // parsed a byte (variable)
		{
			if(!firstNibble)  // From MIDI.TXT: '9n' is exactly the same as '09 n' or '9 n' -- so finish current byte first
			{
				outPos++;
			}
			out[outPos++] = data;
			firstNibble = true;
		}
	}
	// Finish current byte
	if(!firstNibble)
		outPos++;
	if(chn && updateZxxParam < 0x80)
		chn->lastZxxParam = updateZxxParam;

	// Add end of SysEx byte if necessary
	for(size_t i = 0; i < outPos; i++)
	{
		if(out[i] != 0xF0)
			continue;
		if(outPos - i >= 4 && (out[i + 1] == 0xF0 || out[i + 1] == 0xF1))
		{
			// Internal message
			i += 3;
		} else
		{
			// Real SysEx
			while(i < outPos && out[i] != 0xF7)
				i++;
			if(i == outPos && outPos < out.size())
				out[outPos++] = 0xF7;
		}
		
	}

	m_data = out.first(outPos);
}


MIDIMacroParser::~MIDIMacroParser()
{
	if(m_runningStatusPos < m_data.size())
		m_data[m_runningStatusPos] = m_runningstatusOldData;
}


OPENMPT_NAMESPACE_END
