/*
 * UpdateModule.cpp
 * ----------------
 * Purpose: Compensate for playback bugs of previous OpenMPT versions during import
 *          by rewriting patterns / samples / instruments or enabling / disabling specific compatibility flags
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Sndfile.h"
#include "plugins/PluginManager.h"
#include "../common/mptStringBuffer.h"
#include "../common/version.h"


OPENMPT_NAMESPACE_BEGIN


struct UpgradePatternData
{
	UpgradePatternData(CSoundFile &sf)
		: sndFile(sf)
		, compatPlay(sf.m_playBehaviour[MSF_COMPATIBLE_PLAY]) { }

	void operator() (ModCommand &m)
	{
		const CHANNELINDEX curChn = chn;
		chn++;
		if(chn >= sndFile.GetNumChannels())
		{
			chn = 0;
		}

		if(m.IsPcNote())
		{
			return;
		}
		const auto version = sndFile.m_dwLastSavedWithVersion;
		const auto modType = sndFile.GetType();

		if(modType == MOD_TYPE_S3M)
		{
			// Out-of-range global volume commands should be ignored in S3M. Fixed in OpenMPT 1.19 (r831).
			// So for tracks made with older versions of OpenMPT, we limit invalid global volume commands.
			if(version < MPT_V("1.19.00.00") && m.command == CMD_GLOBALVOLUME)
			{
				LimitMax(m.param, ModCommand::PARAM(64));
			}
		}

		else if(modType & (MOD_TYPE_IT | MOD_TYPE_MPT))
		{
			if(version < MPT_V("1.17.03.02") ||
				(!compatPlay && version < MPT_V("1.20.00.00")))
			{
				if(m.command == CMD_GLOBALVOLUME)
				{
					// Out-of-range global volume commands should be ignored in IT.
					// OpenMPT 1.17.03.02 fixed this in compatible mode, OpenMPT 1.20 fixes it in normal mode as well.
					// So for tracks made with older versions than OpenMPT 1.17.03.02 or tracks made with 1.17.03.02 <= version < 1.20, we limit invalid global volume commands.
					LimitMax(m.param, ModCommand::PARAM(128));
				}

				// SC0 and SD0 should be interpreted as SC1 and SD1 in IT files.
				// OpenMPT 1.17.03.02 fixed this in compatible mode, OpenMPT 1.20 fixes it in normal mode as well.
				else if(m.command == CMD_S3MCMDEX)
				{
					if(m.param == 0xC0)
					{
						m.command = CMD_NONE;
						m.note = NOTE_NOTECUT;
					} else if(m.param == 0xD0)
					{
						m.command = CMD_NONE;
					}
				}
			}

			// In the IT format, slide commands with both nibbles set should be ignored.
			// For note volume slides, OpenMPT 1.18 fixes this in compatible mode, OpenMPT 1.20 fixes this in normal mode as well.
			const bool noteVolSlide =
				(version < MPT_V("1.18.00.00") ||
				(!compatPlay && version < MPT_V("1.20.00.00")))
				&&
				(m.command == CMD_VOLUMESLIDE || m.command == CMD_VIBRATOVOL || m.command == CMD_TONEPORTAVOL || m.command == CMD_PANNINGSLIDE);

			// OpenMPT 1.20 also fixes this for global volume and channel volume slides.
			const bool chanVolSlide =
				(version < MPT_V("1.20.00.00"))
				&&
				(m.command == CMD_GLOBALVOLSLIDE || m.command == CMD_CHANNELVOLSLIDE);

			if(noteVolSlide || chanVolSlide)
			{
				if((m.param & 0x0F) != 0x00 && (m.param & 0x0F) != 0x0F && (m.param & 0xF0) != 0x00 && (m.param & 0xF0) != 0xF0)
				{
					if(m.command == CMD_GLOBALVOLSLIDE)
						m.param &= 0xF0;
					else
						m.param &= 0x0F;
				}
			}

			if(version < MPT_V("1.22.01.04")
				&& version != MPT_V("1.22.00.00"))	// Ignore compatibility export
			{
				// OpenMPT 1.22.01.04 fixes illegal (out of range) instrument numbers; they should do nothing. In previous versions, they stopped the playing sample.
				if(sndFile.GetNumInstruments() && m.instr > sndFile.GetNumInstruments() && !compatPlay)
				{
					m.volcmd = VOLCMD_VOLUME;
					m.vol = 0;
				}
			}

			// Command I11 accidentally behaved the same as command I00 with compatible IT tremor and old effects disabled
			if(m.command == CMD_TREMOR && m.param == 0x11 && version < MPT_V("1.29.12.02") && sndFile.m_playBehaviour[kITTremor] && !sndFile.m_SongFlags[SONG_ITOLDEFFECTS])
			{
				m.param = 0;
			}
		}

		else if(modType == MOD_TYPE_XM)
		{
			// Something made be believe that out-of-range global volume commands are ignored in XM
			// just like they are ignored in IT, but apparently they are not. Aaaaaargh!
			if(((version >= MPT_V("1.17.03.02") && compatPlay) || (version >= MPT_V("1.20.00.00")))
				&& version < MPT_V("1.24.02.02")
				&& m.command == CMD_GLOBALVOLUME
				&& m.param > 64)
			{
				m.command = CMD_NONE;
			}

			if(version < MPT_V("1.19.00.00")
				|| (!compatPlay && version < MPT_V("1.20.00.00")))
			{
				if(m.command == CMD_OFFSET && m.volcmd == VOLCMD_TONEPORTAMENTO)
				{
					// If there are both a portamento and an offset effect, the portamento should be preferred in XM files.
					// OpenMPT 1.19 fixed this in compatible mode, OpenMPT 1.20 fixes it in normal mode as well.
					m.command = CMD_NONE;
				}
			}

			if(version < MPT_V("1.20.01.10")
				&& m.volcmd == VOLCMD_TONEPORTAMENTO && m.command == CMD_TONEPORTAMENTO
				&& (m.vol != 0 || compatPlay) && m.param != 0)
			{
				// Mx and 3xx on the same row does weird things in FT2: 3xx is completely ignored and the Mx parameter is doubled. Fixed in revision 1312 / OpenMPT 1.20.01.10
				// Previously the values were just added up, so let's fix this!
				m.volcmd = VOLCMD_NONE;
				const uint16 param = static_cast<uint16>(m.param) + static_cast<uint16>(m.vol << 4);
				m.param = mpt::saturate_cast<ModCommand::PARAM>(param);
			}

			if(version < MPT_V("1.22.07.09")
				&& m.command == CMD_SPEED && m.param == 0)
			{
				// OpenMPT can emulate FT2's F00 behaviour now.
				m.command = CMD_NONE;
			}
		}

		if(version < MPT_V("1.20.00.00"))
		{
			// Pattern Delay fixes

			const bool fixS6x = (m.command == CMD_S3MCMDEX && (m.param & 0xF0) == 0x60);
			// We also fix X6x commands in hacked XM files, since they are treated identically to the S6x command in IT/S3M files.
			// We don't treat them in files made with OpenMPT 1.18+ that have compatible play enabled, though, since they are ignored there anyway.
			const bool fixX6x = (m.command == CMD_XFINEPORTAUPDOWN && (m.param & 0xF0) == 0x60
				&& (!(compatPlay && modType == MOD_TYPE_XM) || version < MPT_V("1.18.00.00")));

			if(fixS6x || fixX6x)
			{
				// OpenMPT 1.20 fixes multiple fine pattern delays on the same row. Previously, only the last command was considered,
				// but all commands should be added up. Since Scream Tracker 3 itself doesn't support S6x, we also use Impulse Tracker's behaviour here,
				// since we can assume that most S3Ms that make use of S6x were composed with Impulse Tracker.
				for(ModCommand *fixCmd = (&m) - curChn; fixCmd < &m; fixCmd++)
				{
					if((fixCmd->command == CMD_S3MCMDEX || fixCmd->command == CMD_XFINEPORTAUPDOWN) && (fixCmd->param & 0xF0) == 0x60)
					{
						fixCmd->command = CMD_NONE;
					}
				}
			}

			if(m.command == CMD_S3MCMDEX && (m.param & 0xF0) == 0xE0)
			{
				// OpenMPT 1.20 fixes multiple pattern delays on the same row. Previously, only the *last* command was considered,
				// but Scream Tracker 3 and Impulse Tracker only consider the *first* command.
				for(ModCommand *fixCmd = (&m) - curChn; fixCmd < &m; fixCmd++)
				{
					if(fixCmd->command == CMD_S3MCMDEX && (fixCmd->param & 0xF0) == 0xE0)
					{
						fixCmd->command = CMD_NONE;
					}
				}
			}
		}

		if(m.volcmd == VOLCMD_VIBRATODEPTH
			&& version < MPT_V("1.27.00.37")
			&& version != MPT_V("1.27.00.00"))
		{
			// Fix handling of double vibrato commands - previously only one of them was applied at a time
			if(m.command == CMD_VIBRATOVOL && m.vol > 0)
			{
				m.command = CMD_VOLUMESLIDE;
			} else if((m.command == CMD_VIBRATO || m.command == CMD_FINEVIBRATO) && (m.param & 0x0F) == 0)
			{
				m.command = CMD_VIBRATO;
				m.param |= (m.vol & 0x0F);
				m.volcmd = VOLCMD_NONE;
			} else if(m.command == CMD_VIBRATO || m.command == CMD_VIBRATOVOL || m.command == CMD_FINEVIBRATO)
			{
				m.volcmd = VOLCMD_NONE;
			}
		}

		// Volume column offset in IT/XM is bad, mkay?
		if(modType != MOD_TYPE_MPT && m.volcmd == VOLCMD_OFFSET && m.command == CMD_NONE)
		{
			m.command = CMD_OFFSET;
			m.param = m.vol << 3;
			m.volcmd = VOLCMD_NONE;
		}

		// Previously CMD_OFFSET simply overrode VOLCMD_OFFSET, now they work together as a combined command
		if(m.volcmd == VOLCMD_OFFSET && m.command == CMD_OFFSET && version < MPT_V("1.30.00.14"))
		{
			if(m.param != 0 || m.vol == 0)
				m.volcmd = VOLCMD_NONE;
			else
				m.command = CMD_NONE;
		}
	}

	const CSoundFile &sndFile;
	CHANNELINDEX chn = 0;
	const bool compatPlay;
};


void CSoundFile::UpgradeModule()
{
	if(m_dwLastSavedWithVersion < MPT_V("1.17.02.46") && m_dwLastSavedWithVersion != MPT_V("1.17.00.00"))
	{
		// Compatible playback mode didn't exist in earlier versions, so definitely disable it.
		m_playBehaviour.reset(MSF_COMPATIBLE_PLAY);
	}

	const bool compatModeIT = m_playBehaviour[MSF_COMPATIBLE_PLAY] && (GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT));
	const bool compatModeXM = m_playBehaviour[MSF_COMPATIBLE_PLAY] && GetType() == MOD_TYPE_XM;

	if(m_dwLastSavedWithVersion < MPT_V("1.20.00.00"))
	{
		for(INSTRUMENTINDEX i = 1; i <= GetNumInstruments(); i++)
		{
			ModInstrument *ins = Instruments[i];
			if(!ins)
				continue;
			// Previously, volume swing values ranged from 0 to 64. They should reach from 0 to 100 instead.
			ins->nVolSwing = static_cast<uint8>(std::min(static_cast<uint32>(ins->nVolSwing * 100 / 64), uint32(100)));

			if(!compatModeIT || m_dwLastSavedWithVersion < MPT_V("1.18.00.00"))
			{
				// Previously, Pitch/Pan Separation was only half depth (plot twist: it was actually only quarter depth).
				// This was corrected in compatible mode in OpenMPT 1.18, and in OpenMPT 1.20 it is corrected in normal mode as well.
				ins->nPPS = static_cast<int8>((ins->nPPS + (ins->nPPS >= 0 ? 1 : -1)) / 2);
			}

			if(!compatModeIT || m_dwLastSavedWithVersion < MPT_V("1.17.03.02"))
			{
				// IT compatibility 24. Short envelope loops
				// Previously, the pitch / filter envelope loop handling was broken, the loop was shortened by a tick (like in XM).
				// This was corrected in compatible mode in OpenMPT 1.17.03.02, and in OpenMPT 1.20 it is corrected in normal mode as well.
				ins->GetEnvelope(ENV_PITCH).Convert(MOD_TYPE_XM, GetType());
			}

			if(m_dwLastSavedWithVersion >= MPT_V("1.17.00.00") && m_dwLastSavedWithVersion < MPT_V("1.17.02.50"))
			{
				// If there are any plugins that can receive volume commands, enable volume bug emulation.
				if(ins->nMixPlug && ins->HasValidMIDIChannel())
				{
					m_playBehaviour.set(kMIDICCBugEmulation);
				}
			}

			if(m_dwLastSavedWithVersion < MPT_V("1.17.02.50") && (ins->nVolSwing | ins->nPanSwing | ins->nCutSwing | ins->nResSwing))
			{
				// If there are any instruments with random variation, enable the old random variation behaviour.
				m_playBehaviour.set(kMPTOldSwingBehaviour);
				break;
			}
		}

		if((GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT)) && (m_dwLastSavedWithVersion < MPT_V("1.17.03.02") || !compatModeIT))
		{
			// In the IT format, a sweep value of 0 shouldn't apply vibrato at all. Previously, a value of 0 was treated as "no sweep".
			// In OpenMPT 1.17.03.02, this was corrected in compatible mode, in OpenMPT 1.20 it is corrected in normal mode as well,
			// so we have to fix the setting while loading.
			for(SAMPLEINDEX i = 1; i <= GetNumSamples(); i++)
			{
				if(Samples[i].nVibSweep == 0 && (Samples[i].nVibDepth | Samples[i].nVibRate))
				{
					Samples[i].nVibSweep = 255;
				}
			}
		}

		// Fix old nasty broken (non-standard) MIDI configs in files.
		m_MidiCfg.UpgradeMacros();
	}

	if(m_dwLastSavedWithVersion < MPT_V("1.20.02.10")
		&& m_dwLastSavedWithVersion != MPT_V("1.20.00.00")
		&& (GetType() & (MOD_TYPE_XM | MOD_TYPE_IT | MOD_TYPE_MPT)))
	{
		bool instrPlugs = false;
		// Old pitch wheel commands were closest to sample pitch bend commands if the PWD is 13.
		for(INSTRUMENTINDEX i = 1; i <= GetNumInstruments(); i++)
		{
			if(Instruments[i] != nullptr && Instruments[i]->nMidiChannel != MidiNoChannel)
			{
				Instruments[i]->midiPWD = 13;
				instrPlugs = true;
			}
		}
		if(instrPlugs)
		{
			m_playBehaviour.set(kOldMIDIPitchBends);
		}
	}

	if(m_dwLastSavedWithVersion < MPT_V("1.22.03.12")
		&& m_dwLastSavedWithVersion != MPT_V("1.22.00.00")
		&& (GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT))
		&& (m_playBehaviour[MSF_COMPATIBLE_PLAY] || m_playBehaviour[kMPTOldSwingBehaviour]))
	{
		// The "correct" pan swing implementation did nothing if the instrument also had a pan envelope.
		// If there's a pan envelope, disable pan swing for such modules.
		for(INSTRUMENTINDEX i = 1; i <= GetNumInstruments(); i++)
		{
			if(Instruments[i] != nullptr && Instruments[i]->nPanSwing != 0 && Instruments[i]->PanEnv.dwFlags[ENV_ENABLED])
			{
				Instruments[i]->nPanSwing = 0;
			}
		}
	}

	bool hasAnyPlugins = false;
	if(GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT | MOD_TYPE_XM))
	{
		for(auto &plugin : m_MixPlugins)
		{
			if(plugin.IsValidPlugin())
			{
				hasAnyPlugins = true;
				break;
			}
		}
	}

#ifndef NO_PLUGINS
	if(m_dwLastSavedWithVersion < MPT_V("1.22.07.01") && hasAnyPlugins)
	{
		// Convert ANSI plugin path names to UTF-8 (irrelevant in probably 99% of all cases anyway, I think I've never seen a VST plugin with a non-ASCII file name)
		for(auto &plugin : m_MixPlugins)
		{
#if defined(MODPLUG_TRACKER)
			const std::string name = mpt::ToCharset(mpt::Charset::UTF8, mpt::Charset::Locale, plugin.Info.szLibraryName);
#else
			const std::string name = mpt::ToCharset(mpt::Charset::UTF8, mpt::Charset::Windows1252, plugin.Info.szLibraryName);
#endif
			plugin.Info.szLibraryName = name;
		}
	}
#endif // NO_PLUGINS

	// Starting from OpenMPT 1.22.07.19, FT2-style panning was applied in compatible mix mode.
	// Starting from OpenMPT 1.23.01.04, FT2-style panning has its own mix mode instead.
	if(GetType() == MOD_TYPE_XM)
	{
		if(m_dwLastSavedWithVersion >= MPT_V("1.22.07.19")
			&& m_dwLastSavedWithVersion < MPT_V("1.23.01.04")
			&& GetMixLevels() == MixLevels::Compatible)
		{
			SetMixLevels(MixLevels::CompatibleFT2);
		}
	}

	if(m_dwLastSavedWithVersion < MPT_V("1.25.00.07") && m_dwLastSavedWithVersion != MPT_V("1.25.00.00"))
	{
		// Instrument plugins can now receive random volume variation.
		// For old instruments, disable volume swing in case there was no sample associated.
		for(INSTRUMENTINDEX i = 1; i <= GetNumInstruments(); i++)
		{
			if(Instruments[i] != nullptr && Instruments[i]->nVolSwing != 0 && Instruments[i]->nMidiChannel != MidiNoChannel)
			{
				bool hasSample = false;
				for(auto smp : Instruments[i]->Keyboard)
				{
					if(smp != 0)
					{
						hasSample = true;
						break;
					}
				}
				if(!hasSample)
				{
					Instruments[i]->nVolSwing = 0;
				}
			}
		}
	}

	if(m_dwLastSavedWithVersion < MPT_V("1.26.00.00"))
	{
		for(INSTRUMENTINDEX i = 1; i <= GetNumInstruments(); i++)
		{
			ModInstrument *ins = Instruments[i];
			if(!ins)
				continue;
			// Even after fixing it in OpenMPT 1.18, instrument PPS was only half the depth.
			ins->nPPS = static_cast<int8>((ins->nPPS + (ins->nPPS >= 0 ? 1 : -1)) / 2);

			// OpenMPT 1.18 fixed the depth of random pan in compatible mode.
			// OpenMPT 1.26 fixes it in normal mode too.
			if(!compatModeIT || m_dwLastSavedWithVersion < MPT_V("1.18.00.00"))
				ins->nPanSwing = static_cast<uint8>((ins->nPanSwing + 3) / 4u);

			// Before OpenMPT 1.26 (r6129), it was possible to trigger MIDI notes using channel plugins if the instrument had a valid MIDI channel.
			if(!ins->nMixPlug && ins->HasValidMIDIChannel() && m_dwLastSavedWithVersion >= MPT_V("1.17.00.00"))
				m_playBehaviour.set(kMIDINotesFromChannelPlugin);
		}
	}

	if(m_dwLastSavedWithVersion < MPT_V("1.28.00.12"))
	{
		for(INSTRUMENTINDEX i = 1; i <= GetNumInstruments(); i++)
		{
			if(Instruments[i] != nullptr && Instruments[i]->VolEnv.nReleaseNode != ENV_RELEASE_NODE_UNSET)
			{
				m_playBehaviour.set(kLegacyReleaseNode);
				break;
			}
		}
	}

	if(m_dwLastSavedWithVersion < MPT_V("1.28.03.04"))
	{
		for(INSTRUMENTINDEX i = 1; i <= GetNumInstruments(); i++)
		{
			if(Instruments[i] != nullptr && (Instruments[i]->pluginVolumeHandling == PLUGIN_VOLUMEHANDLING_MIDI || Instruments[i]->pluginVolumeHandling == PLUGIN_VOLUMEHANDLING_DRYWET))
			{
				m_playBehaviour.set(kMIDIVolumeOnNoteOffBug);
				break;
			}
		}
	}

	if(m_dwLastSavedWithVersion < MPT_V("1.30.00.54"))
	{
		for(SAMPLEINDEX i = 1; i <= GetNumSamples(); i++)
		{
			if(Samples[i].HasSampleData() && Samples[i].uFlags[CHN_PINGPONGLOOP | CHN_PINGPONGSUSTAIN])
			{
				m_playBehaviour.set(kImprecisePingPongLoops);
				break;
			}
		}
	}

	Patterns.ForEachModCommand(UpgradePatternData(*this));

	// Convert compatibility flags
	// NOTE: Some of these version numbers are just approximations.
	// Sometimes a quirk flag is shared by several code locations which might have been fixed at different times.
	// Sometimes the quirk behaviour has been revised over time, in which case the first version that emulated the quirk enables it.
	struct PlayBehaviourVersion
	{
		PlayBehaviour behaviour;
		Version version;
	};
	
	if(compatModeIT && m_dwLastSavedWithVersion < MPT_V("1.26.00.00"))
	{
		// Pre-1.26: Detailed compatibility flags did not exist.
		static constexpr PlayBehaviourVersion behaviours[] =
		{
			{ kTempoClamp,                    MPT_V("1.17.03.02") },
			{ kPerChannelGlobalVolSlide,      MPT_V("1.17.03.02") },
			{ kPanOverride,                   MPT_V("1.17.03.02") },
			{ kITInstrWithoutNote,            MPT_V("1.17.02.46") },
			{ kITVolColFinePortamento,        MPT_V("1.17.02.49") },
			{ kITArpeggio,                    MPT_V("1.17.02.49") },
			{ kITOutOfRangeDelay,             MPT_V("1.17.02.49") },
			{ kITPortaMemoryShare,            MPT_V("1.17.02.49") },
			{ kITPatternLoopTargetReset,      MPT_V("1.17.02.49") },
			{ kITFT2PatternLoop,              MPT_V("1.17.02.49") },
			{ kITPingPongNoReset,             MPT_V("1.17.02.51") },
			{ kITEnvelopeReset,               MPT_V("1.17.02.51") },
			{ kITClearOldNoteAfterCut,        MPT_V("1.17.02.52") },
			{ kITVibratoTremoloPanbrello,     MPT_V("1.17.03.02") },
			{ kITTremor,                      MPT_V("1.17.03.02") },
			{ kITRetrigger,                   MPT_V("1.17.03.02") },
			{ kITMultiSampleBehaviour,        MPT_V("1.17.03.02") },
			{ kITPortaTargetReached,          MPT_V("1.17.03.02") },
			{ kITPatternLoopBreak,            MPT_V("1.17.03.02") },
			{ kITOffset,                      MPT_V("1.17.03.02") },
			{ kITSwingBehaviour,              MPT_V("1.18.00.00") },
			{ kITNNAReset,                    MPT_V("1.18.00.00") },
			{ kITSCxStopsSample,              MPT_V("1.18.00.01") },
			{ kITEnvelopePositionHandling,    MPT_V("1.18.01.00") },
			{ kITPortamentoInstrument,        MPT_V("1.19.00.01") },
			{ kITPingPongMode,                MPT_V("1.19.00.21") },
			{ kITRealNoteMapping,             MPT_V("1.19.00.30") },
			{ kITHighOffsetNoRetrig,          MPT_V("1.20.00.14") },
			{ kITFilterBehaviour,             MPT_V("1.20.00.35") },
			{ kITNoSurroundPan,               MPT_V("1.20.00.53") },
			{ kITShortSampleRetrig,           MPT_V("1.20.00.54") },
			{ kITPortaNoNote,                 MPT_V("1.20.00.56") },
			{ kRowDelayWithNoteDelay,         MPT_V("1.20.00.76") },
			{ kITFT2DontResetNoteOffOnPorta,  MPT_V("1.20.02.06") },
			{ kITVolColMemory,                MPT_V("1.21.01.16") },
			{ kITPortamentoSwapResetsPos,     MPT_V("1.21.01.25") },
			{ kITEmptyNoteMapSlot,            MPT_V("1.21.01.25") },
			{ kITFirstTickHandling,           MPT_V("1.22.07.09") },
			{ kITSampleAndHoldPanbrello,      MPT_V("1.22.07.19") },
			{ kITClearPortaTarget,            MPT_V("1.23.04.03") },
			{ kITPanbrelloHold,               MPT_V("1.24.01.06") },
			{ kITPanningReset,                MPT_V("1.24.01.06") },
			{ kITPatternLoopWithJumpsOld,     MPT_V("1.25.00.19") },
		};

		for(const auto &b : behaviours)
		{
			m_playBehaviour.set(b.behaviour, (m_dwLastSavedWithVersion >= b.version || m_dwLastSavedWithVersion == b.version.Masked(0xFFFF0000u)));
		}
	} else if(compatModeXM && m_dwLastSavedWithVersion < MPT_V("1.26.00.00"))
	{
		// Pre-1.26: Detailed compatibility flags did not exist.
		static constexpr PlayBehaviourVersion behaviours[] =
		{
			{ kTempoClamp,               MPT_V("1.17.03.02") },
			{ kPerChannelGlobalVolSlide, MPT_V("1.17.03.02") },
			{ kPanOverride,              MPT_V("1.17.03.02") },
			{ kITFT2PatternLoop,         MPT_V("1.17.03.02") },
			{ kFT2Arpeggio,              MPT_V("1.17.03.02") },
			{ kFT2Retrigger,             MPT_V("1.17.03.02") },
			{ kFT2VolColVibrato,         MPT_V("1.17.03.02") },
			{ kFT2PortaNoNote,           MPT_V("1.17.03.02") },
			{ kFT2KeyOff,                MPT_V("1.17.03.02") },
			{ kFT2PanSlide,              MPT_V("1.17.03.02") },
			{ kFT2ST3OffsetOutOfRange,   MPT_V("1.17.03.02") },
			{ kFT2RestrictXCommand,      MPT_V("1.18.00.00") },
			{ kFT2RetrigWithNoteDelay,   MPT_V("1.18.00.00") },
			{ kFT2SetPanEnvPos,          MPT_V("1.18.00.00") },
			{ kFT2PortaIgnoreInstr,      MPT_V("1.18.00.01") },
			{ kFT2VolColMemory,          MPT_V("1.18.01.00") },
			{ kFT2LoopE60Restart,        MPT_V("1.18.02.01") },
			{ kFT2ProcessSilentChannels, MPT_V("1.18.02.01") },
			{ kFT2ReloadSampleSettings,  MPT_V("1.20.00.36") },
			{ kFT2PortaDelay,            MPT_V("1.20.00.40") },
			{ kFT2Transpose,             MPT_V("1.20.00.62") },
			{ kFT2PatternLoopWithJumps,  MPT_V("1.20.00.69") },
			{ kFT2PortaTargetNoReset,    MPT_V("1.20.00.69") },
			{ kFT2EnvelopeEscape,        MPT_V("1.20.00.77") },
			{ kFT2Tremor,                MPT_V("1.20.01.11") },
			{ kFT2OutOfRangeDelay,       MPT_V("1.20.02.02") },
			{ kFT2Periods,               MPT_V("1.22.03.01") },
			{ kFT2PanWithDelayedNoteOff, MPT_V("1.22.03.02") },
			{ kFT2VolColDelay,           MPT_V("1.22.07.19") },
			{ kFT2FinetunePrecision,     MPT_V("1.22.07.19") },
		};

		for(const auto &b : behaviours)
		{
			m_playBehaviour.set(b.behaviour, m_dwLastSavedWithVersion >= b.version);
		}
	}
	
	if(GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT))
	{
		// The following behaviours were added in/after OpenMPT 1.26, so are not affected by the upgrade mechanism above.
		static constexpr PlayBehaviourVersion behaviours[] =
		{
			{ kITInstrWithNoteOff,            MPT_V("1.26.00.01") },
			{ kITMultiSampleInstrumentNumber, MPT_V("1.27.00.27") },
			{ kITInstrWithNoteOffOldEffects,  MPT_V("1.28.02.06") },
			{ kITDoNotOverrideChannelPan,     MPT_V("1.29.00.22") },
			{ kITPatternLoopWithJumps,        MPT_V("1.29.00.32") },
			{ kITDCTBehaviour,                MPT_V("1.29.00.57") },
			{ kITPitchPanSeparation,          MPT_V("1.30.00.53") },
			{ kITResetFilterOnPortaSmpChange, MPT_V("1.30.08.02") },
			{ kITInitialNoteMemory,           MPT_V("1.31.00.25") },
			{ kITNoSustainOnPortamento,       MPT_V("1.32.00.13") },
			{ kITEmptyNoteMapSlotIgnoreCell,  MPT_V("1.32.00.13") },
			{ kITOffsetWithInstrNumber,       MPT_V("1.32.00.15") },
			{ kITDoublePortamentoSlides,      MPT_V("1.32.00.27") },
			{ kITCarryAfterNoteOff,           MPT_V("1.32.00.40") },
			{ kITNoteCutWithPorta,            MPT_V("1.32.01.02") },
			{ kITVolColNoSlidePropagation,    MPT_V("1.32.02.03") },
			{ kITStoppedFilterEnvAtStart,     MPT_V("1.32.03.04") },
		};

		for(const auto &b : behaviours)
		{
			if(m_dwLastSavedWithVersion < b.version.Masked(0xFFFF0000u))
				m_playBehaviour.reset(b.behaviour);
			// Full version information available, i.e. not compatibility-exported.
			else if(m_dwLastSavedWithVersion > b.version.Masked(0xFFFF0000u) && m_dwLastSavedWithVersion < b.version)
				m_playBehaviour.reset(b.behaviour);
		}
	} else if(GetType() == MOD_TYPE_XM)
	{
		// The following behaviours were added after OpenMPT 1.26, so are not affected by the upgrade mechanism above.
		static constexpr PlayBehaviourVersion behaviours[] =
		{
			{ kFT2NoteOffFlags,              MPT_V("1.27.00.27") },
			{ kRowDelayWithNoteDelay,        MPT_V("1.27.00.37") },
			{ kFT2MODTremoloRampWaveform,    MPT_V("1.27.00.37") },
			{ kFT2PortaUpDownMemory,         MPT_V("1.27.00.37") },
			{ kFT2PanSustainRelease,         MPT_V("1.28.00.09") },
			{ kFT2NoteDelayWithoutInstr,     MPT_V("1.28.00.44") },
			{ kITFT2DontResetNoteOffOnPorta, MPT_V("1.29.00.34") },
			{ kFT2PortaResetDirection,       MPT_V("1.30.00.40") },
			{ kFT2AutoVibratoAbortSweep,     MPT_V("1.32.00.29") },
			{ kFT2OffsetMemoryRequiresNote,  MPT_V("1.32.00.43") },
		};

		for(const auto &b : behaviours)
		{
			if(m_dwLastSavedWithVersion < b.version)
				m_playBehaviour.reset(b.behaviour);
		}
	} else if(GetType() == MOD_TYPE_S3M)
	{
		// We do not store any of these flags in S3M files.
		static constexpr PlayBehaviourVersion behaviours[] =
		{
			{ kST3NoMutedChannels,         MPT_V("1.18.00.00") },
			{ kST3EffectMemory,            MPT_V("1.20.00.00") },
			{ kRowDelayWithNoteDelay,      MPT_V("1.20.00.00") },
			{ kST3PortaSampleChange,       MPT_V("1.22.00.00") },
			{ kST3VibratoMemory,           MPT_V("1.26.00.00") },
			{ kITPanbrelloHold,            MPT_V("1.26.00.00") },
			{ KST3PortaAfterArpeggio,      MPT_V("1.27.00.00") },
			{ kST3OffsetWithoutInstrument, MPT_V("1.28.00.00") },
			{ kST3RetrigAfterNoteCut,      MPT_V("1.29.00.00") },
			{ kFT2ST3OffsetOutOfRange,     MPT_V("1.29.00.00") },
			{ kApplyUpperPeriodLimit,      MPT_V("1.30.00.45") },
			{ kST3TonePortaWithAdlibNote,  MPT_V("1.31.00.13") },
		};

		for(const auto &b : behaviours)
		{
			if(m_dwLastSavedWithVersion < b.version)
				m_playBehaviour.reset(b.behaviour);
		}
	}

	if(GetType() == MOD_TYPE_XM && m_dwLastSavedWithVersion < MPT_V("1.19.00.00"))
	{
		// This bug was introduced sometime between 1.18.03.00 and 1.19.01.00
		m_playBehaviour.set(kFT2NoteDelayWithoutInstr);
	}

	if(m_dwLastSavedWithVersion >= MPT_V("1.27.00.27") && m_dwLastSavedWithVersion < MPT_V("1.27.00.49"))
	{
		// OpenMPT 1.27 inserted some IT/FT2 flags before the S3M flags that are never saved to files anyway, to keep the flag IDs a bit more compact.
		// However, it was overlooked that these flags would still be read by OpenMPT 1.26 and thus S3M-specific behaviour would be enabled in IT/XM files.
		// Hence, in OpenMPT 1.27.00.49 the flag IDs got remapped to no longer conflict with OpenMPT 1.26.
		// Files made with the affected pre-release versions of OpenMPT 1.27 are upgraded here to use the new IDs.
		for(int i = 0; i < 5; i++)
		{
			m_playBehaviour.set(kFT2NoteOffFlags + i, m_playBehaviour[kST3NoMutedChannels + i]);
			m_playBehaviour.reset(kST3NoMutedChannels + i);
		}
	}

	if(m_dwLastSavedWithVersion < MPT_V("1.17.00.00"))
	{
		// MPT 1.16 has a maximum tempo of 255.
		m_playBehaviour.set(kTempoClamp);
	} else if(m_dwLastSavedWithVersion >= MPT_V("1.17.00.00") && m_dwLastSavedWithVersion <= MPT_V("1.20.01.03") && m_dwLastSavedWithVersion != MPT_V("1.20.00.00"))
	{
		// OpenMPT introduced some "fixes" that execute regular portamentos also at speed 1.
		m_playBehaviour.set(kSlidesAtSpeed1);
	}

	if(m_SongFlags[SONG_LINEARSLIDES])
	{
		if(m_dwLastSavedWithVersion < MPT_V("1.24.00.00"))
		{
			// No frequency slides in Hz before OpenMPT 1.24
			m_playBehaviour.reset(kPeriodsAreHertz);
		} else if(m_dwLastSavedWithVersion >= MPT_V("1.24.00.00") && m_dwLastSavedWithVersion < MPT_V("1.26.00.00") && (GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT)))
		{
			// Frequency slides were always in Hz rather than periods in this version range.
			m_playBehaviour.set(kPeriodsAreHertz);
		}
	} else
	{
		if(m_dwLastSavedWithVersion < MPT_V("1.30.00.36") && m_dwLastSavedWithVersion != MPT_V("1.30.00.00"))
		{
			// No frequency slides in Hz before OpenMPT 1.30
			m_playBehaviour.reset(kPeriodsAreHertz);
		}
	}

	if(m_playBehaviour[kITEnvelopePositionHandling]
		&& m_dwLastSavedWithVersion >= MPT_V("1.23.01.02") && m_dwLastSavedWithVersion < MPT_V("1.28.00.43"))
	{
		// Bug that effectively clamped the release node to the sustain end
		for(INSTRUMENTINDEX i = 1; i <= GetNumInstruments(); i++)
		{
			if(Instruments[i] != nullptr
				&& Instruments[i]->VolEnv.nReleaseNode != ENV_RELEASE_NODE_UNSET
				&& Instruments[i]->VolEnv.dwFlags[ENV_SUSTAIN]
				&& Instruments[i]->VolEnv.nReleaseNode > Instruments[i]->VolEnv.nSustainEnd)
			{
				m_playBehaviour.set(kReleaseNodePastSustainBug);
				break;
			}
		}
	}

	if(GetType() & (MOD_TYPE_MPT | MOD_TYPE_S3M))
	{
		for(SAMPLEINDEX i = 1; i <= GetNumSamples(); i++)
		{
			if(Samples[i].uFlags[CHN_ADLIB])
			{
				if(GetType() == MOD_TYPE_MPT && GetNumInstruments() && m_dwLastSavedWithVersion >= MPT_V("1.28.00.20") && m_dwLastSavedWithVersion <= MPT_V("1.29.00.55"))
					m_playBehaviour.set(kOPLNoResetAtEnvelopeEnd);
				if(m_dwLastSavedWithVersion <= MPT_V("1.30.00.34") && m_dwLastSavedWithVersion != MPT_V("1.30"))
					m_playBehaviour.reset(kOPLNoteOffOnNoteChange);
				if(GetType() == MOD_TYPE_S3M && m_dwLastSavedWithVersion < MPT_V("1.29"))
					m_playBehaviour.set(kOPLRealRetrig);
				else if(GetType() != MOD_TYPE_S3M)
					m_playBehaviour.reset(kOPLRealRetrig);
				break;
			}
		}
	}

#ifndef NO_PLUGINS
	if(m_dwLastSavedWithVersion >= MPT_V("1.27.00.42") && m_dwLastSavedWithVersion < MPT_V("1.30.00.46") && hasAnyPlugins)
	{
		// The Flanger DMO plugin is almost identical to the Chorus... but only almost.
		// The effect implementation was the same in OpenMPT 1.27-1.29, now it isn't anymore.
		// As the old implementation continues to exist for the Chorus plugin, there is a legacy wrapper for the Flanger plugin.
		for(auto &plugin : m_MixPlugins)
		{
			if(plugin.Info.dwPluginId1 == kDmoMagic && plugin.Info.dwPluginId2 == int32(0xEFCA3D92) && plugin.pluginData.size() == 32)
				plugin.Info.szLibraryName = "Flanger (Legacy)";
		}
	}

	if(m_dwLastSavedWithVersion < MPT_V("1.30.00.54") && hasAnyPlugins)
	{
		// Currently active program and bank is assumed to be 1 when starting playback
		for(INSTRUMENTINDEX i = 1; i <= GetNumInstruments(); i++)
		{
			if(Instruments[i] && (Instruments[i]->nMidiProgram == 1 || Instruments[i]->wMidiBank == 1))
			{
				m_playBehaviour.set(kPluginDefaultProgramAndBank1);
				break;
			}
		}
	}

	if(m_dwLastSavedWithVersion < MPT_V("1.31.00.09") && hasAnyPlugins)
	{
		// Old-style plugin tone portamento
		m_playBehaviour.set(kPluginIgnoreTonePortamento);
	}

	if(m_dwLastSavedWithVersion >= MPT_V("1.27") && m_dwLastSavedWithVersion < MPT_V("1.30.06.00") && hasAnyPlugins)
	{
		// Fix off-by-one delay length in older Echo DMO emulation
		for(auto &plugin : m_MixPlugins)
		{
			if(plugin.Info.dwPluginId1 == kDmoMagic && plugin.Info.dwPluginId2 == int32(0xEF3E932C) && plugin.pluginData.size() == 24)
			{
				float32le leftDelay, rightDelay;
				memcpy(&leftDelay, plugin.pluginData.data() + 12, 4);
				memcpy(&rightDelay, plugin.pluginData.data() + 16, 4);
				leftDelay = float32le{mpt::safe_clamp(((leftDelay * 2000.0f) - 1.0f) / 1999.0f, 0.0f, 1.0f)};
				rightDelay = float32le{mpt::safe_clamp(((rightDelay * 2000.0f) - 1.0f) / 1999.0f, 0.0f, 1.0f)};
				memcpy(plugin.pluginData.data() + 12, &leftDelay, 4);
				memcpy(plugin.pluginData.data() + 16, &rightDelay, 4);
			}
		}
	}

	if(m_dwLastSavedWithVersion >= MPT_V("1.17") && m_dwLastSavedWithVersion < MPT_V("1.32.00.30") && hasAnyPlugins)
	{
		for(const auto &plugin : m_MixPlugins)
		{
			if(plugin.Info.dwPluginId1 == PLUGMAGIC('V', 's', 't', 'P'))
			{
				m_playBehaviour.set(kLegacyPPQpos);
				break;
			}
		}
	}

	if(m_dwLastSavedWithVersion < MPT_V("1.32.00.38") && hasAnyPlugins)
	{
		for(const auto &plugin : m_MixPlugins)
		{
			if(plugin.Info.dwPluginId1 == PLUGMAGIC('V', 's', 't', 'P'))
			{
				m_playBehaviour.set(kLegacyPluginNNABehaviour);
				break;
			}
		}
	}
#endif
}

OPENMPT_NAMESPACE_END
