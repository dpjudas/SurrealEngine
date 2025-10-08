/*
 * Load_s3m.cpp
 * ------------
 * Purpose: S3M (ScreamTracker 3) module loader / saver
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "S3MTools.h"
#include "ITTools.h"
#ifndef MODPLUG_NO_FILESAVE
#include "mpt/io/base.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "../common/mptFileIO.h"
#ifdef MODPLUG_TRACKER
#include "../mptrack/Moddoc.h"
#include "../mptrack/TrackerSettings.h"
#endif // MODPLUG_TRACKER
#endif // MODPLUG_NO_FILESAVE
#include "../common/version.h"


OPENMPT_NAMESPACE_BEGIN


void CSoundFile::S3MConvert(ModCommand &m, const uint8 command, const uint8 param, const bool fromIT)
{
	m.param = param;
	switch(command | 0x40)
	{
	case '@': m.command = (m.param ? CMD_DUMMY : CMD_NONE); break;
	case 'A': m.command = CMD_SPEED; break;
	case 'B': m.command = CMD_POSITIONJUMP; break;
	case 'C': m.command = CMD_PATTERNBREAK; if(!fromIT) m.param = static_cast<uint8>((m.param >> 4) * 10 + (m.param & 0x0F)); break;
	case 'D': m.command = CMD_VOLUMESLIDE; break;
	case 'E': m.command = CMD_PORTAMENTODOWN; break;
	case 'F': m.command = CMD_PORTAMENTOUP; break;
	case 'G': m.command = CMD_TONEPORTAMENTO; break;
	case 'H': m.command = CMD_VIBRATO; break;
	case 'I': m.command = CMD_TREMOR; break;
	case 'J': m.command = CMD_ARPEGGIO; break;
	case 'K': m.command = CMD_VIBRATOVOL; break;
	case 'L': m.command = CMD_TONEPORTAVOL; break;
	case 'M': m.command = CMD_CHANNELVOLUME; break;
	case 'N': m.command = CMD_CHANNELVOLSLIDE; break;
	case 'O': m.command = CMD_OFFSET; break;
	case 'P': m.command = CMD_PANNINGSLIDE; break;
	case 'Q': m.command = CMD_RETRIG; break;
	case 'R': m.command = CMD_TREMOLO; break;
	case 'S': m.command = CMD_S3MCMDEX; break;
	case 'T': m.command = CMD_TEMPO; break;
	case 'U': m.command = CMD_FINEVIBRATO; break;
	case 'V': m.command = CMD_GLOBALVOLUME; break;
	case 'W': m.command = CMD_GLOBALVOLSLIDE; break;
	case 'X': m.command = CMD_PANNING8; break;
	case 'Y': m.command = CMD_PANBRELLO; break;
	case 'Z': m.command = CMD_MIDI; break;
	case '\\': m.command = fromIT ? CMD_SMOOTHMIDI : CMD_MIDI; break;
	// Chars under 0x40 don't save properly, so the following commands don't map to their pattern editor representations
	case ']': m.command = fromIT ? CMD_DELAYCUT : CMD_NONE; break;
	case '[': m.command = fromIT ? CMD_XPARAM : CMD_NONE; break;
	case '^': m.command = fromIT ? CMD_FINETUNE : CMD_NONE; break;
	case '_': m.command = fromIT ? CMD_FINETUNE_SMOOTH : CMD_NONE; break;
	// BeRoTracker extensions
	case '1' + 0x41: m.command = fromIT ? CMD_KEYOFF : CMD_NONE; break;
	case '2' + 0x41: m.command = fromIT ? CMD_SETENVPOSITION : CMD_NONE; break;
	default: m.command = CMD_NONE;
	}
}

#ifndef MODPLUG_NO_FILESAVE

void CSoundFile::S3MSaveConvert(const ModCommand &source, uint8 &command, uint8 &param, const bool toIT, const bool compatibilityExport) const
{
	command = 0;
	param = source.param;
	const bool extendedIT = !compatibilityExport && toIT;
	switch(source.command)
	{
	case CMD_DUMMY:           command = (param ? '@' : 0); break;
	case CMD_SPEED:           command = 'A'; break;
	case CMD_POSITIONJUMP:    command = 'B'; break;
	case CMD_PATTERNBREAK:    command = 'C'; if(!toIT) param = static_cast<uint8>(((param / 10) << 4) + (param % 10)); break;
	case CMD_VOLUMESLIDE:     command = 'D'; break;
	case CMD_PORTAMENTODOWN:  command = 'E'; if(param >= 0xE0 && (GetType() & (MOD_TYPE_MOD | MOD_TYPE_XM))) param = 0xDF; break;
	case CMD_PORTAMENTOUP:    command = 'F'; if(param >= 0xE0 && (GetType() & (MOD_TYPE_MOD | MOD_TYPE_XM))) param = 0xDF; break;
	case CMD_TONEPORTAMENTO:  command = 'G'; break;
	case CMD_VIBRATO:         command = 'H'; break;
	case CMD_TREMOR:          command = 'I'; break;
	case CMD_ARPEGGIO:        command = 'J'; break;
	case CMD_VIBRATOVOL:      command = 'K'; break;
	case CMD_TONEPORTAVOL:    command = 'L'; break;
	case CMD_CHANNELVOLUME:   command = 'M'; break;
	case CMD_CHANNELVOLSLIDE: command = 'N'; break;
	case CMD_OFFSETPERCENTAGE:
	case CMD_OFFSET:          command = 'O'; break;
	case CMD_PANNINGSLIDE:    command = 'P'; break;
	case CMD_RETRIG:          command = 'Q'; break;
	case CMD_TREMOLO:         command = 'R'; break;
	case CMD_S3MCMDEX:        command = 'S'; break;
	case CMD_TEMPO:           command = 'T'; break;
	case CMD_FINEVIBRATO:     command = 'U'; break;
	case CMD_GLOBALVOLUME:    command = 'V'; break;
	case CMD_GLOBALVOLSLIDE:  command = 'W'; break;
	case CMD_PANNING8:
		command = 'X';
		if(toIT && !(GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT | MOD_TYPE_XM | MOD_TYPE_MOD)))
		{
			if(param == 0xA4) { command = 'S'; param = 0x91; }
			else if(param == 0x80) { param = 0xFF; }
			else if(param < 0x80) { param <<= 1; }
			else command = 0;
		} else if(!toIT && (GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT | MOD_TYPE_XM | MOD_TYPE_MOD)))
		{
			param >>= 1;
		}
		break;
	case CMD_PANBRELLO: command = 'Y'; break;
	case CMD_MIDI:      command = 'Z'; break;
	case CMD_SMOOTHMIDI:
		if(extendedIT)
			command = '\\';
		else
			command = 'Z';
		break;
	case CMD_XFINEPORTAUPDOWN:
		switch(param & 0xF0)
		{
		case 0x10: command = 'F'; param = (param & 0x0F) | 0xE0; break;
		case 0x20: command = 'E'; param = (param & 0x0F) | 0xE0; break;
		case 0x90: command = 'S'; break;
		default:   command = 0;
		}
		break;
	case CMD_MODCMDEX:
		{
			ModCommand mConv;
			mConv.command = CMD_MODCMDEX;
			mConv.param = param;
			mConv.ExtendedMODtoS3MEffect();
			S3MSaveConvert(mConv, command, param, toIT, compatibilityExport);
		}
		return;
	// Chars under 0x40 don't save properly, so map : to ] and # to [.
	case CMD_DELAYCUT:
		command = extendedIT ? ']' : 0;
		break;
	case CMD_XPARAM:
		command = extendedIT ? '[' : 0;
		break;
	case CMD_FINETUNE:
		command = extendedIT ? '^' : 0;
		break;
	case CMD_FINETUNE_SMOOTH:
		command = extendedIT ? '_' : 0;
		break;
	default:
		command = 0;
	}
	if(command == 0)
	{
		param = 0;
	}

	command &= ~0x40;
}

#endif // MODPLUG_NO_FILESAVE

static bool ValidateHeader(const S3MFileHeader &fileHeader)
{
	if(std::memcmp(fileHeader.magic, "SCRM", 4)
		|| fileHeader.fileType != S3MFileHeader::idS3MType
		|| (fileHeader.formatVersion != S3MFileHeader::oldVersion && fileHeader.formatVersion != S3MFileHeader::newVersion)
		)
	{
		return false;
	}
	return true;
}


static uint64 GetHeaderMinimumAdditionalSize(const S3MFileHeader &fileHeader)
{
	return fileHeader.ordNum + (fileHeader.smpNum + fileHeader.patNum) * 2;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderS3M(MemoryFileReader file, const uint64 *pfilesize)
{
	S3MFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return ProbeWantMoreData;
	}
	if(!ValidateHeader(fileHeader))
	{
		return ProbeFailure;
	}
	return ProbeAdditionalSize(file, pfilesize, GetHeaderMinimumAdditionalSize(fileHeader));
}


bool CSoundFile::ReadS3M(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	// Is it a valid S3M file?
	S3MFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return false;
	}
	if(!ValidateHeader(fileHeader))
	{
		return false;
	}
	if(!file.CanRead(mpt::saturate_cast<FileReader::pos_type>(GetHeaderMinimumAdditionalSize(fileHeader))))
	{
		return false;
	}
	if(loadFlags == onlyVerifyHeader)
	{
		return true;
	}

	InitializeGlobals(MOD_TYPE_S3M, fileHeader.GetNumChannels());
	m_nMinPeriod = 64;
	m_nMaxPeriod = 32767;

	ReadOrderFromFile<uint8>(Order(), file, fileHeader.ordNum, 0xFF, 0xFE);

	// Read sample header offsets
	std::vector<uint16le> sampleOffsets;
	file.ReadVector(sampleOffsets, fileHeader.smpNum);
	// Read pattern offsets
	std::vector<uint16le> patternOffsets;
	file.ReadVector(patternOffsets, fileHeader.patNum);

	// ST3 ignored Zxx commands, so if we find that a file was made with ST3, we should erase all MIDI macros.
	bool keepMidiMacros = false;

	mpt::ustring madeWithTracker;
	bool formatTrackerStr = false;
	bool nonCompatTracker = false;
	bool isST3 = false;
	bool isSchism = false;
	const bool usePanningTable = fileHeader.usePanningTable == S3MFileHeader::idPanning;
	const bool offsetsAreCanonical = !patternOffsets.empty() && !sampleOffsets.empty() && patternOffsets[0] > sampleOffsets[0];
	const int32 schismDateVersion = SchismTrackerEpoch + ((fileHeader.cwtv == 0x4FFF) ? fileHeader.reserved2 : (fileHeader.cwtv - 0x4050));
	uint32 editTimer = 0;
	switch(fileHeader.cwtv & S3MFileHeader::trackerMask)
	{
	case S3MFileHeader::trkAkord & S3MFileHeader::trackerMask:
		if(fileHeader.cwtv == S3MFileHeader::trkAkord)
			madeWithTracker = UL_("Akord");
		break;
	case S3MFileHeader::trkScreamTracker:
		if(!memcmp(&fileHeader.reserved2, "SCLUB2.0", 8))
		{
			madeWithTracker = UL_("Sound Club 2");
		} else if(fileHeader.cwtv == S3MFileHeader::trkST3_20 && fileHeader.special == 0 && (fileHeader.ordNum & 0x01) == 0 && fileHeader.ultraClicks == 0 && (fileHeader.flags & ~0x50) == 0 && usePanningTable && offsetsAreCanonical)
		{
			// Canonical offset check avoids mis-detection of an automatic conversion of Vic's "Paper" demo track
			if((fileHeader.ordNum & 0x0F) == 0)
			{
				// MPT and OpenMPT before 1.17.03.02 - Simply keep default (filter) MIDI macros
				if((fileHeader.masterVolume & 0x80) != 0)
				{
					m_dwLastSavedWithVersion = MPT_V("1.16");
					madeWithTracker = UL_("ModPlug Tracker / OpenMPT 1.17");
				} else
				{
					// MPT 1.0 alpha5 doesn't set the stereo flag, but MPT 1.0 alpha6 does.
					m_dwLastSavedWithVersion = MPT_V("1.00.00.A0");
					madeWithTracker = UL_("ModPlug Tracker 1.0 alpha");
				}
			} else if((fileHeader.masterVolume & 0x80) != 0)
			{
				madeWithTracker = UL_("Schism Tracker");
			}
			keepMidiMacros = true;
			nonCompatTracker = true;
			m_playBehaviour.set(kST3LimitPeriod);
		} else if(fileHeader.cwtv == S3MFileHeader::trkST3_20 && fileHeader.special == 0 && fileHeader.ultraClicks == 0 && fileHeader.flags == 0 && !usePanningTable)
		{
			if(fileHeader.globalVol == 64 && fileHeader.masterVolume == 48)
				madeWithTracker = UL_("PlayerPRO");
			else  // Always stereo
				madeWithTracker = UL_("Velvet Studio");
		} else if(fileHeader.cwtv == S3MFileHeader::trkST3_20 && fileHeader.special == 0 && fileHeader.ultraClicks == 0 && fileHeader.flags == 8 && !usePanningTable)
		{
			madeWithTracker = UL_("Impulse Tracker < 1.03");  // Not sure if 1.02 saves like this as I don't have it
		} else
		{
			// ST3.20 should only ever write ultra-click values 16, 24 and 32 (corresponding to 8, 12 and 16 in the GUI), ST3.01/3.03 should only write 0,
			// though several ST3.01/3.03 files with ultra-click values of 16 have been found as well.
			// However, we won't fingerprint these values here as it's unlikely that there is any other tracker out there disguising as ST3 and using a strange ultra-click value.
			// Also, re-saving a file with a strange ultra-click value in ST3 doesn't fix this value unless the user manually changes it, or if it's below 16.
			isST3 = true;
			if(fileHeader.cwtv == S3MFileHeader::trkST3_20)
			{
				// 3.21 writes the version number as 3.20. There is no known way to differentiate between the two.
				madeWithTracker = UL_("Scream Tracker 3.20 - 3.21");
			} else
			{
				madeWithTracker = UL_("Scream Tracker");
				formatTrackerStr = true;
			}
		}
		break;
	case S3MFileHeader::trkImagoOrpheus:
		formatTrackerStr = (fileHeader.cwtv != S3MFileHeader::trkPlayerPRO);
		if(formatTrackerStr)
			madeWithTracker = UL_("Imago Orpheus");
		else
			madeWithTracker = UL_("PlayerPRO");
		nonCompatTracker = true;
		break;
	case S3MFileHeader::trkImpulseTracker:
		if(fileHeader.cwtv == S3MFileHeader::trkIT1_old)
			madeWithTracker = UL_("Impulse Tracker 1.03");  // Could also be 1.02, maybe? I don't have that one
		else
			madeWithTracker = GetImpulseTrackerVersion(fileHeader.cwtv, 0);

		if(fileHeader.cwtv >= S3MFileHeader::trkIT2_07 && fileHeader.reserved3 != 0)
		{
			// Starting from version 2.07, IT stores the total edit time of a module in the "reserved" field
			editTimer = DecodeITEditTimer(fileHeader.cwtv, fileHeader.reserved3);
		}
		nonCompatTracker = true;
		m_playBehaviour.set(kPeriodsAreHertz);
		m_playBehaviour.set(kITRetrigger);
		m_playBehaviour.set(kITShortSampleRetrig);
		m_playBehaviour.set(kST3SampleSwap);  // Not exactly like ST3, but close enough
		m_playBehaviour.set(kITPortaNoNote);
		m_playBehaviour.set(kITPortamentoSwapResetsPos);
		m_nMinPeriod = 1;
		break;
	case S3MFileHeader::trkSchismTracker:
		if(fileHeader.cwtv == S3MFileHeader::trkBeRoTrackerOld)
		{
			madeWithTracker = UL_("BeRoTracker");
			m_playBehaviour.set(kST3LimitPeriod);
		} else
		{
			madeWithTracker = GetSchismTrackerVersion(fileHeader.cwtv, fileHeader.reserved2);
			m_nMinPeriod = 1;
			isSchism = true;
			if(schismDateVersion >= SchismVersionFromDate<2021, 05, 02>::date)
				m_playBehaviour.set(kPeriodsAreHertz);
			if(schismDateVersion >= SchismVersionFromDate<2016, 05, 13>::date)
				m_playBehaviour.set(kITShortSampleRetrig);
			m_playBehaviour.reset(kST3TonePortaWithAdlibNote);

			if(fileHeader.cwtv == (S3MFileHeader::trkSchismTracker | 0xFFF) && fileHeader.reserved3 != 0)
			{
				// Added in commit 6c4b71f10d4e0bf202dddfa8bd781de510b8bc0b
				editTimer = fileHeader.reserved3;
			}
		}
		nonCompatTracker = true;
		break;
	case S3MFileHeader::trkOpenMPT:
		if((fileHeader.cwtv & 0xFF00) == S3MFileHeader::trkNESMusa)
		{
			madeWithTracker = UL_("NESMusa");
			formatTrackerStr = true;
		} else if(fileHeader.reserved2 == 0 && fileHeader.ultraClicks == 16 && fileHeader.channels[1] != 1)
		{
			// Liquid Tracker's ID clashes with OpenMPT's.
			// OpenMPT started writing full version information with OpenMPT 1.29 and later changed the ultraClicks value from 8 to 16.
			// Liquid Tracker writes an ultraClicks value of 16.
			// So we assume that a file was saved with Liquid Tracker if the reserved fields are 0 and ultraClicks is 16.
			madeWithTracker = UL_("Liquid Tracker");
			formatTrackerStr = true;
		} else if(fileHeader.cwtv != S3MFileHeader::trkGraoumfTracker)
		{
			uint32 mptVersion = (fileHeader.cwtv & S3MFileHeader::versionMask) << 16;
			if(mptVersion >= 0x01'29'00'00)
			{
				mptVersion |= fileHeader.reserved2;
				// Added in OpenMPT 1.32.00.31
				if(fileHeader.reserved3 != 0)
					editTimer = fileHeader.reserved3;
			}
			m_dwLastSavedWithVersion = Version(mptVersion);
			madeWithTracker = UL_("OpenMPT ") + mpt::ufmt::val(m_dwLastSavedWithVersion);
		} else
		{
			madeWithTracker = UL_("Graoumf Tracker");
		}
		break; 
	case S3MFileHeader::trkBeRoTracker:
		madeWithTracker = UL_("BeRoTracker");
		m_playBehaviour.set(kST3LimitPeriod);
		break;
	case S3MFileHeader::trkCreamTracker:
		madeWithTracker = UL_("CreamTracker");
		break;
	default:
		if(fileHeader.cwtv == S3MFileHeader::trkCamoto)
			madeWithTracker = UL_("Camoto");
		break;
	}
	if(formatTrackerStr)
	{
		madeWithTracker = MPT_UFORMAT("{} {}.{}")(madeWithTracker, (fileHeader.cwtv & 0xF00) >> 8, mpt::ufmt::hex0<2>(fileHeader.cwtv & 0xFF));
	}

	// IT edit timer
	if(editTimer != 0)
	{
		FileHistory hist;
		hist.openTime = static_cast<uint32>(editTimer * (HISTORY_TIMER_PRECISION / 18.2));
		m_FileHistory.push_back(hist);
	}

	m_modFormat.formatName = UL_("Scream Tracker 3");
	m_modFormat.type = UL_("s3m");
	m_modFormat.madeWithTracker = std::move(madeWithTracker);
	m_modFormat.charset = m_dwLastSavedWithVersion ? mpt::Charset::Windows1252 : mpt::Charset::CP437;

	if(nonCompatTracker)
	{
		m_playBehaviour.reset(kST3NoMutedChannels);
		m_playBehaviour.reset(kST3EffectMemory);
		m_playBehaviour.reset(kST3PortaSampleChange);
		m_playBehaviour.reset(kST3VibratoMemory);
		m_playBehaviour.reset(KST3PortaAfterArpeggio);
		m_playBehaviour.reset(kST3OffsetWithoutInstrument);
		m_playBehaviour.reset(kApplyUpperPeriodLimit);
	}
	if(fileHeader.cwtv <= S3MFileHeader::trkST3_01)
	{
		// This broken behaviour is not present in ST3.01
		m_playBehaviour.reset(kST3TonePortaWithAdlibNote);
	}

	if((fileHeader.cwtv & S3MFileHeader::trackerMask) > S3MFileHeader::trkScreamTracker)
	{
		if((fileHeader.cwtv & S3MFileHeader::trackerMask) != S3MFileHeader::trkImpulseTracker || fileHeader.cwtv >= S3MFileHeader::trkIT2_14)
		{
			// Keep MIDI macros if this is not an old IT version (BABYLON.S3M by Necros has Zxx commands and was saved with IT 2.05)
			keepMidiMacros = true;
		}
	}

	m_MidiCfg.Reset();
	if(!keepMidiMacros)
	{
		// Remove macros so they don't interfere with tunes made in trackers that don't support Zxx
		m_MidiCfg.ClearZxxMacros();
	}

	m_songName = mpt::String::ReadBuf(mpt::String::nullTerminated, fileHeader.name);

	if(fileHeader.flags & S3MFileHeader::amigaLimits) m_SongFlags.set(SONG_AMIGALIMITS);
	if(fileHeader.flags & S3MFileHeader::st2Vibrato) m_SongFlags.set(SONG_S3MOLDVIBRATO);

	if(fileHeader.cwtv == S3MFileHeader::trkST3_00 || (fileHeader.flags & S3MFileHeader::fastVolumeSlides) != 0)
	{
		m_SongFlags.set(SONG_FASTVOLSLIDES);
	}

	// Even though ST3 accepts the command AFF as expected, it mysteriously fails to load a default speed of 255...
	if(fileHeader.speed == 0 || (fileHeader.speed == 255 && isST3))
		Order().SetDefaultSpeed(6);
	else
		Order().SetDefaultSpeed(fileHeader.speed);

	// ST3 also fails to load an otherwise valid default tempo of 32...
	if(fileHeader.tempo < 33)
		Order().SetDefaultTempoInt(isST3 ? 125 : 32);
	else
		Order().SetDefaultTempoInt(fileHeader.tempo);

	// Global Volume
	m_nDefaultGlobalVolume = std::min(fileHeader.globalVol.get(), uint8(64)) * 4u;
	// The following check is probably not very reliable, but it fixes a few tunes, e.g.
	// DARKNESS.S3M by Purple Motion (ST 3.00) and "Image of Variance" by C.C.Catch (ST 3.01).
	// Note that even ST 3.01b imports these files with a global volume of 0,
	// so it's not clear if these files ever played "as intended" in any ST3 versions (I don't have any older ST3 versions).
	if(m_nDefaultGlobalVolume == 0 && fileHeader.cwtv < S3MFileHeader::trkST3_20)
	{
		m_nDefaultGlobalVolume = MAX_GLOBAL_VOLUME;
	}

	if(fileHeader.formatVersion == S3MFileHeader::oldVersion && fileHeader.masterVolume < 8)
		m_nSamplePreAmp = std::min((fileHeader.masterVolume + 1) * 0x10, 0x7F);
	// These changes were probably only supposed to be done for older format revisions, where supposedly 0x10 was the stereo flag.
	// However, this version check is missing in ST3, so any mono file with a master volume of 18 will be converted to a stereo file with master volume 32.
	else if(fileHeader.masterVolume == 2 || fileHeader.masterVolume == (2 | 0x10))
		m_nSamplePreAmp = 0x20;
	else if(!(fileHeader.masterVolume & 0x7F))
		m_nSamplePreAmp = 48;
	else
		m_nSamplePreAmp = std::max(fileHeader.masterVolume & 0x7F, 0x10);  // Bit 7 = Stereo (we always use stereo)

	// Approximately as loud as in DOSBox and a real SoundBlaster 16
	m_nVSTiVolume = 36;
	if(isSchism && schismDateVersion < SchismVersionFromDate<2018, 11, 12>::date)
		m_nVSTiVolume = 64;

	const bool isStereo = (fileHeader.masterVolume & 0x80) != 0 || m_dwLastSavedWithVersion;
	if(!isStereo)
	{
		m_nSamplePreAmp = Util::muldivr_unsigned(m_nSamplePreAmp, 8, 11);
		m_nVSTiVolume = Util::muldivr_unsigned(m_nVSTiVolume, 8, 11);
	}

	// Channel setup
	std::bitset<32> isAdlibChannel;
	for(CHANNELINDEX i = 0; i < GetNumChannels(); i++)
	{
		uint8 ctype = fileHeader.channels[i] & ~0x80;
		if(fileHeader.channels[i] != 0xFF && isStereo)
			ChnSettings[i].nPan = (ctype & 8) ? 0xCC : 0x33;  // 200 : 56
		if(fileHeader.channels[i] & 0x80)
			ChnSettings[i].dwFlags = CHN_MUTE;
		if(ctype >= 16 && ctype <= 29)
		{
			// Adlib channel - except for OpenMPT 1.19 and older, which would write wrong channel types for PCM channels 16-32.
			// However, MPT/OpenMPT always wrote the extra panning table, so there is no need to consider this here.
			ChnSettings[i].nPan = 128;
			isAdlibChannel[i] = true;
		}
	}

	// Read extended channel panning
	if(usePanningTable)
	{
		bool hasChannelsWithoutPanning = false;
		const auto pan = file.ReadArray<uint8, 32>();
		for(CHANNELINDEX i = 0; i < GetNumChannels(); i++)
		{
			if((pan[i] & 0x20) != 0 && (!isST3 || !isAdlibChannel[i]))
				ChnSettings[i].nPan = static_cast<uint16>((static_cast<uint16>(pan[i] & 0x0F) * 256 + 8) / 15u);
			else if(pan[i] < 0x10)
				hasChannelsWithoutPanning = true;
		}
		if(GetNumChannels() < 32 && m_dwLastSavedWithVersion == MPT_V("1.16"))
		{
			// MPT 1.0 alpha 6 up to 1.16.203 set the panning bit for all channels, regardless of whether they are used or not.
			// Note: Schism Tracker fixed the same bug in git commit f21fe8bcae8b6dde2df27ede4ac9fe563f91baff
			if(hasChannelsWithoutPanning)
				m_modFormat.madeWithTracker = UL_("ModPlug Tracker 1.16 / OpenMPT 1.17");
			else
				m_modFormat.madeWithTracker = UL_("ModPlug Tracker");
		}
	}

	// Reading sample headers
	m_nSamples = std::min(static_cast<SAMPLEINDEX>(fileHeader.smpNum), static_cast<SAMPLEINDEX>(MAX_SAMPLES - 1));
	bool anySamples = false, anyADPCM = false;
	uint16 gusAddresses = 0;
	for(SAMPLEINDEX smp = 0; smp < m_nSamples; smp++)
	{
		S3MSampleHeader sampleHeader;

		if(!file.Seek(sampleOffsets[smp] * 16) || !file.ReadStruct(sampleHeader))
		{
			continue;
		}

		sampleHeader.ConvertToMPT(Samples[smp + 1], isST3);
		// Old ModPlug Tracker allowed to write into the last byte reserved for the null terminator
		m_szNames[smp + 1] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, sampleHeader.name);

		if(sampleHeader.sampleType < S3MSampleHeader::typeAdMel)
		{
			if(sampleHeader.length != 0)
			{
				SampleIO sampleIO = sampleHeader.GetSampleFormat((fileHeader.formatVersion == S3MFileHeader::oldVersion));
				if((loadFlags & loadSampleData) && file.Seek(sampleHeader.GetSampleOffset()))
					sampleIO.ReadSample(Samples[smp + 1], file);
				anySamples = true;
				if(sampleIO.GetEncoding() == SampleIO::ADPCM)
					anyADPCM = true;
			}
			gusAddresses |= sampleHeader.gusAddress;
		}
	}

	const bool useGUS = gusAddresses > 1;
	if(isST3 && anySamples && !gusAddresses && fileHeader.cwtv != S3MFileHeader::trkST3_00)
	{
		// All Scream Tracker versions except for some probably early revisions of Scream Tracker 3.00 write GUS addresses. GUS support might not have existed at that point (1992).
		// Hence if a file claims to be written with ST3 (but not ST3.00), but has no GUS addresses, we deduce that it must be written by some other software (e.g. some PSM -> S3M conversions)
		isST3 = false;
		m_modFormat.madeWithTracker = UL_("Unknown");
		// Check these only after we are certain that it can't be ST3.01 because that version doesn't sanitize the ultraClicks value yet
		if(fileHeader.cwtv == S3MFileHeader::trkST3_01 && fileHeader.ultraClicks == 0)
		{
			if(!(fileHeader.flags & ~(S3MFileHeader::fastVolumeSlides | S3MFileHeader::amigaLimits)) && (fileHeader.masterVolume & 0x80) && usePanningTable)
				m_modFormat.madeWithTracker = UL_("UNMO3");
			else if(!fileHeader.flags && fileHeader.globalVol == 48 && fileHeader.masterVolume == 176 && fileHeader.tempo == 150 && !usePanningTable)
				m_modFormat.madeWithTracker = UL_("deMODifier");  // SoundSmith to S3M converter
			else if(!fileHeader.flags && fileHeader.globalVol == 64 && (fileHeader.masterVolume & 0x7F) == 48 && fileHeader.speed == 6 && fileHeader.tempo == 125 && !usePanningTable)
				m_modFormat.madeWithTracker = UL_("Kosmic To-S3M");  // MTM to S3M converter by Zab/Kosmic
		}
	} else if(isST3)
	{
		// Saving an S3M file in ST3 with the Gravis Ultrasound driver loaded will write a unique GUS memory address for each non-empty sample slot (and 0 for unused slots).
		// Re-saving that file in ST3 with the SoundBlaster driver loaded will reset the GUS address for all samples to 0 (unused) or 1 (used).
		// The first used sample will also have an address of 1 with the GUS driver.
		// So this is a safe way of telling if the file was last saved with the GUS driver loaded or not if there's more than one sample.
		m_playBehaviour.set(kST3PortaSampleChange, useGUS);
		m_playBehaviour.set(kST3SampleSwap, !useGUS);
		m_playBehaviour.set(kITShortSampleRetrig, !useGUS);  // Only half the truth but close enough for now
		m_modFormat.madeWithTracker += useGUS ? UL_(" (GUS)") : UL_(" (SB)");
		// ST3's GUS driver doesn't use this value. Ignoring it fixes the balance between FM and PCM samples (e.g. in Rotagilla by Manwe)
		if(useGUS)
			m_nSamplePreAmp = 48;
	}
	if(isST3)
		m_playBehaviour.set(kS3MIgnoreCombinedFineSlides);

	if(anyADPCM)
		m_modFormat.madeWithTracker += UL_(" (ADPCM packed)");

	// Try to find out if Zxx commands are supposed to be panning commands (PixPlay).
	// Actually I am only aware of one module that uses this panning style, namely "Crawling Despair" by $volkraq
	// and I have no idea what PixPlay is, so this code is solely based on the sample text of that module.
	// We won't convert if there are not enough Zxx commands, too "high" Zxx commands
	// or there are only "left" or "right" pannings (we assume that stereo should be somewhat balanced),
	// and modules not made with an old version of ST3 were probably made in a tracker that supports panning anyway.
	bool pixPlayPanning = (fileHeader.cwtv < S3MFileHeader::trkST3_20);
	int zxxCountRight = 0, zxxCountLeft = 0;

	// Reading patterns
	if(!(loadFlags & loadPatternData))
	{
		return true;
	}
	// Order list cannot contain pattern indices > 255, so do not even try to load higher patterns
	const PATTERNINDEX readPatterns = std::min(static_cast<PATTERNINDEX>(fileHeader.patNum), static_cast<PATTERNINDEX>(uint8_max));
	Patterns.ResizeArray(readPatterns);
	for(PATTERNINDEX pat = 0; pat < readPatterns; pat++)
	{
		// A zero parapointer indicates an empty pattern.
		if(!Patterns.Insert(pat, 64) || patternOffsets[pat] == 0 || !file.Seek(patternOffsets[pat] * 16))
		{
			continue;
		}

		// Skip pattern length indication.
		// Some modules, for example http://aminet.net/mods/8voic/s3m_hunt.lha seem to have a wrong pattern length -
		// If you strictly adhere the pattern length, you won't read some patterns (e.g. 17) correctly in that module.
		// It's most likely a broken copy because there are other versions of the track which don't have this issue.
		// Still, we don't really need this information, so we just ignore it.
		file.Skip(2);

		// Read pattern data
		ROWINDEX row = 0;
		auto rowBase = Patterns[pat].GetRow(0);

		ModCommand dummy;
		while(row < 64)
		{
			uint8 info = file.ReadUint8();

			if(info == s3mEndOfRow)
			{
				// End of row
				if(++row < 64)
				{
					rowBase = Patterns[pat].GetRow(row);
				}
				continue;
			}

			CHANNELINDEX channel = (info & s3mChannelMask);
			ModCommand &m = (channel < GetNumChannels()) ? rowBase[channel] : dummy;

			if(info & s3mNotePresent)
			{
				const auto [note, instr] = file.ReadArray<uint8, 2>();
				if(note < 0xF0)
					m.note = static_cast<ModCommand::NOTE>(Clamp((note & 0x0F) + 12 * (note >> 4) + 12 + NOTE_MIN, NOTE_MIN, NOTE_MAX));
				else if(note == s3mNoteOff)
					m.note = NOTE_NOTECUT;
				else if(note == s3mNoteNone)
					m.note = NOTE_NONE;
				m.instr = instr;
			}

			if(info & s3mVolumePresent)
			{
				uint8 volume = file.ReadUint8();
				if(volume >= 128 && volume <= 192)
				{
					m.volcmd = VOLCMD_PANNING;
					m.vol = volume - 128;
				} else
				{
					m.volcmd = VOLCMD_VOLUME;
					m.vol = std::min(volume, uint8(64));
				}
			}

			if(info & s3mEffectPresent)
			{
				const auto [command, param] = file.ReadArray<uint8, 2>();
				S3MConvert(m, command, param, false);

				if(m.command == CMD_S3MCMDEX && (m.param & 0xF0) == 0xA0 && fileHeader.cwtv < S3MFileHeader::trkST3_20)
				{
					// Convert the old messy SoundBlaster stereo control command (or an approximation of it, anyway)
					const uint8 ctype = fileHeader.channels[channel] & 0x7F;
					if(useGUS || ctype >= 0x10)
						m.command = CMD_DUMMY;
					else if(m.param == 0xA0 || m.param == 0xA2)  // Normal panning
						m.param = (ctype & 8) ? 0x8C : 0x83;
					else if(m.param == 0xA1 || m.param == 0xA3)  // Swap left / right channel
						m.param = (ctype & 8) ? 0x83 : 0x8C;
					else if(m.param <= 0xA7)  // Center
						m.param = 0x88;
					else
						m.command = CMD_DUMMY;
				} else if(m.command == CMD_MIDI)
				{
					// PixPlay panning test
					if(m.param > 0x0F)
					{
						// PixPlay has Z00 to Z0F panning, so we ignore this.
						pixPlayPanning = false;
					} else
					{
						if(m.param < 0x08)
							zxxCountLeft++;
						else if(m.param > 0x08)
							zxxCountRight++;
					}
				} else if(m.command == CMD_OFFSET && m.param == 0 && isST3 && fileHeader.cwtv <= S3MFileHeader::trkST3_01)
				{
					// Offset command didn't have effect memory in ST3.01; fixed in ST3.03
					m.command = CMD_DUMMY;
				}
			}
		}
	}

	if(pixPlayPanning && zxxCountLeft + zxxCountRight >= GetNumChannels() && (-zxxCountLeft + zxxCountRight) < static_cast<int>(GetNumChannels()))
	{
		// There are enough Zxx commands, so let's assume this was made to be played with PixPlay
		Patterns.ForEachModCommand([](ModCommand &m)
		{
			if(m.command == CMD_MIDI)
			{
				m.command = CMD_S3MCMDEX;
				m.param |= 0x80;
			}
		});
	}

	return true;
}


#ifndef MODPLUG_NO_FILESAVE

bool CSoundFile::SaveS3M(std::ostream &f) const
{
	static constexpr uint8 filler[16] =
	{
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	};

	if(GetNumChannels() == 0)
	{
		return false;
	}

	const bool saveMuteStatus =
#ifdef MODPLUG_TRACKER
		TrackerSettings::Instance().MiscSaveChannelMuteStatus;
#else
		true;
#endif

	S3MFileHeader fileHeader;
	MemsetZero(fileHeader);

	mpt::String::WriteBuf(mpt::String::nullTerminated, fileHeader.name) = m_songName;
	fileHeader.dosEof = S3MFileHeader::idEOF;
	fileHeader.fileType = S3MFileHeader::idS3MType;

	// Orders
	ORDERINDEX writeOrders = Order().GetLengthTailTrimmed();
	if(writeOrders < 2)
	{
		writeOrders = 2;
	} else if((writeOrders % 2u) != 0)
	{
		// Number of orders should be even
		writeOrders++;
	}
	LimitMax(writeOrders, static_cast<ORDERINDEX>(256));
	fileHeader.ordNum = static_cast<uint16>(writeOrders);

	// Samples
	SAMPLEINDEX writeSamples = static_cast<SAMPLEINDEX>(GetNumInstruments());
	if(writeSamples == 0)
	{
		writeSamples = GetNumSamples();
	}
	writeSamples = Clamp(writeSamples, static_cast<SAMPLEINDEX>(1), static_cast<SAMPLEINDEX>(99));
	fileHeader.smpNum = static_cast<uint16>(writeSamples);

	// Patterns
	PATTERNINDEX writePatterns = std::min(Patterns.GetNumPatterns(), PATTERNINDEX(100));
	fileHeader.patNum = static_cast<uint16>(writePatterns);

	// Flags
	if(m_SongFlags[SONG_FASTVOLSLIDES])
	{
		fileHeader.flags |= S3MFileHeader::fastVolumeSlides;
	}
	if(m_nMaxPeriod < 20000 || m_SongFlags[SONG_AMIGALIMITS])
	{
		fileHeader.flags |= S3MFileHeader::amigaLimits;
	}
	if(m_SongFlags[SONG_S3MOLDVIBRATO])
	{
		fileHeader.flags |= S3MFileHeader::st2Vibrato;
	}

	// Version info following: ST3.20 = 0x1320
	// Most significant nibble = Tracker ID, see S3MFileHeader::S3MTrackerVersions
	// Following: One nibble = Major version, one byte = Minor version (hex)
	const uint32 mptVersion = Version::Current().GetRawVersion();
	fileHeader.cwtv = S3MFileHeader::trkOpenMPT | static_cast<uint16>((mptVersion >> 16) & S3MFileHeader::versionMask);
	fileHeader.reserved2 = static_cast<uint16>(mptVersion);
	fileHeader.formatVersion = S3MFileHeader::newVersion;
	memcpy(fileHeader.magic, "SCRM", 4);

	// Song Variables
	fileHeader.globalVol = static_cast<uint8>(std::min(m_nDefaultGlobalVolume / 4u, uint32(64)));
	fileHeader.speed = static_cast<uint8>(Clamp(Order().GetDefaultSpeed(), 1u, 254u));
	fileHeader.tempo = static_cast<uint8>(Clamp(Order().GetDefaultTempo().GetInt(), 33u, 255u));
	fileHeader.masterVolume = static_cast<uint8>(Clamp(m_nSamplePreAmp, 16u, 127u) | 0x80);
	fileHeader.ultraClicks = 16;
	fileHeader.usePanningTable = S3MFileHeader::idPanning;

	// IT edit timer
	uint64 editTimer = 0;
	for(const auto &mptHistory : GetFileHistory())
	{
		editTimer += static_cast<uint32>(mptHistory.openTime * HISTORY_TIMER_PRECISION / 18.2);
	}
#ifdef MODPLUG_TRACKER
	if(const auto modDoc = GetpModDoc(); modDoc != nullptr)
	{
		auto creationTime = modDoc->GetCreationTime();
		editTimer += mpt::saturate_round<uint64>((mpt::Date::UnixAsSeconds(mpt::Date::UnixNow()) - mpt::Date::UnixAsSeconds(creationTime)) * HISTORY_TIMER_PRECISION);
	}
#endif  // MODPLUG_TRACKER
	fileHeader.reserved3 = mpt::saturate_cast<uint32>(editTimer);

	mpt::IO::Write(f, fileHeader);
	Order().WriteAsByte(f, writeOrders);

	// Comment about parapointers stolen from Schism Tracker:
	// The sample data parapointers are 24+4 bits, whereas pattern data and sample headers are only 16+4
	// bits -- so while the sample data can be written up to 268 MB within the file (starting at 0xffffff0),
	// the pattern data and sample headers are restricted to the first 1 MB (starting at 0xffff0). In effect,
	// this practically requires the sample data to be written last in the file, as it is entirely possible
	// (and quite easy, even) to write more than 1 MB of sample data in a file.
	// The "practical standard order" listed in TECH.DOC is sample headers, patterns, then sample data.

	// Calculate offset of first sample header...
	mpt::IO::Offset sampleHeaderOffset = mpt::IO::TellWrite(f) + (writeSamples + writePatterns) * 2 + 32;
	// ...which must be a multiple of 16, because parapointers omit the lowest 4 bits.
	sampleHeaderOffset = (sampleHeaderOffset + 15) & ~15;

	std::vector<uint16le> sampleOffsets(writeSamples);
	for(SAMPLEINDEX smp = 0; smp < writeSamples; smp++)
	{
		static_assert((sizeof(S3MSampleHeader) % 16) == 0);
		sampleOffsets[smp] = static_cast<uint16>((sampleHeaderOffset + smp * sizeof(S3MSampleHeader)) / 16);
	}
	mpt::IO::Write(f, sampleOffsets);

	mpt::IO::Offset patternPointerOffset = mpt::IO::TellWrite(f);
	mpt::IO::Offset firstPatternOffset = sampleHeaderOffset + writeSamples * sizeof(S3MSampleHeader);
	std::vector<uint16le> patternOffsets(writePatterns);

	// Need to calculate the real offsets later.
	mpt::IO::Write(f, patternOffsets);

	// Write channel panning
	uint8 chnPan[32];
	for(CHANNELINDEX chn = 0; chn < 32; chn++)
	{
		if(chn < GetNumChannels())
			chnPan[chn] = static_cast<uint8>(((ChnSettings[chn].nPan * 15 + 128) / 256) | 0x20);
		else
			chnPan[chn] = 0x08;
	}
	mpt::IO::Write(f, chnPan);

	// Do we need to fill up the file with some padding bytes for 16-Byte alignment?
	mpt::IO::Offset curPos = mpt::IO::TellWrite(f);
	if(curPos < sampleHeaderOffset)
	{
		MPT_ASSERT(sampleHeaderOffset - curPos < 16);
		mpt::IO::WriteRaw(f, filler, static_cast<std::size_t>(sampleHeaderOffset - curPos));
	}

	// Don't write sample headers for now, we are lacking the sample offset data.
	mpt::IO::SeekAbsolute(f, firstPatternOffset);

	// Write patterns
	enum class S3MChannelType : uint8 { kUnused = 0, kPCM = 1, kAdlib = 2 };
	std::array<FlagSet<S3MChannelType>, 32> channelType;
	channelType.fill(S3MChannelType::kUnused);
	bool globalCmdOnMutedChn = false;
	for(PATTERNINDEX pat = 0; pat < writePatterns; pat++)
	{
		if(Patterns.IsPatternEmpty(pat))
		{
			patternOffsets[pat] = 0;
			continue;
		}

		mpt::IO::Offset patOffset = mpt::IO::TellWrite(f);
		if(patOffset > 0xFFFF0)
		{
			AddToLog(LogError, MPT_UFORMAT("Too much pattern data! Writing patterns failed starting from pattern {}.")(pat));
			break;
		}
		MPT_ASSERT((patOffset % 16) == 0);
		patternOffsets[pat] = static_cast<uint16>(patOffset / 16);

		std::vector<uint8> buffer;
		buffer.reserve(5 * 1024);
		// Reserve space for length bytes
		buffer.resize(2, 0);

		if(Patterns.IsValidPat(pat))
		{
			for(ROWINDEX row = 0; row < 64; row++)
			{
				if(row >= Patterns[pat].GetNumRows())
				{
					// Invent empty row
					buffer.push_back(s3mEndOfRow);
					continue;
				}

				const auto rowBase = Patterns[pat].GetRow(row);

				CHANNELINDEX writeChannels = std::min(CHANNELINDEX(32), GetNumChannels());
				bool writePatternBreak = (Patterns[pat].GetNumRows() < 64 && row + 1 == Patterns[pat].GetNumRows() && !Patterns[pat].RowHasJump(row));
				for(CHANNELINDEX chn = 0; chn < writeChannels; chn++)
				{
					const ModCommand &m = rowBase[chn];

					uint8 info = static_cast<uint8>(chn);
					uint8 note = m.note;

					if(note != NOTE_NONE || m.instr != 0)
					{
						info |= s3mNotePresent;

						if(ModCommand::IsSpecialNote(note))
						{
							// Note Cut
							note = s3mNoteOff;
						} else if(note >= NOTE_MIN + 12 && note <= NOTE_MIN + 107)
						{
							note -= NOTE_MIN + 12;
							note = static_cast<uint8>((note % 12) + ((note / 12) << 4));
						} else
						{
							note = s3mNoteNone;
						}

						if(m.instr > 0 && m.instr <= GetNumSamples())
						{
							const ModSample &smp = Samples[m.instr];
							if(smp.uFlags[CHN_ADLIB])
								channelType[chn].set(S3MChannelType::kAdlib);
							else if(smp.HasSampleData())
								channelType[chn].set(S3MChannelType::kPCM);
						}
					}

					uint8 vol = std::min(m.vol, ModCommand::VOL(64));
					if(m.volcmd == VOLCMD_VOLUME)
					{
						info |= s3mVolumePresent;
					} else if(m.volcmd == VOLCMD_PANNING)
					{
						info |= s3mVolumePresent;
						vol |= 0x80;
					} else if(m.command == CMD_VOLUME)
					{
						info |= s3mVolumePresent;
						vol = std::min(m.param, ModCommand::PARAM(64));
					}

					uint8 command = 0, param = 0;
					if(m.command != CMD_NONE && m.command != CMD_VOLUME)
					{
						S3MSaveConvert(m, command, param, false, true);
						if(command || param)
						{
							info |= s3mEffectPresent;
							if(saveMuteStatus && ChnSettings[chn].dwFlags[CHN_MUTE] && m.IsGlobalCommand())
							{
								globalCmdOnMutedChn = true;
							}
						}
					}
					if(writePatternBreak && !(info & s3mEffectPresent))
					{
						info |= s3mEffectPresent;
						command = 'C' ^ 0x40;
						writePatternBreak = false;
					}

					if(info & s3mAnyPresent)
					{
						buffer.push_back(info);
						if(info & s3mNotePresent)
						{
							buffer.push_back(note);
							buffer.push_back(m.instr);
						}
						if(info & s3mVolumePresent)
						{
							buffer.push_back(vol);
						}
						if(info & s3mEffectPresent)
						{
							buffer.push_back(command);
							buffer.push_back(param);
						}
					}
				}

				buffer.push_back(s3mEndOfRow);
			}
		} else
		{
			// Invent empty pattern
			buffer.insert(buffer.end(), 64, s3mEndOfRow);
		}

		uint16 length = mpt::saturate_cast<uint16>(buffer.size());
		buffer[0] = static_cast<uint8>(length & 0xFF);
		buffer[1] = static_cast<uint8>((length >> 8) & 0xFF);

		if((buffer.size() % 16u) != 0)
		{
			// Add padding bytes
			buffer.insert(buffer.end(), 16 - (buffer.size() % 16u), 0);
		}

		mpt::IO::Write(f, buffer);
	}
	if(globalCmdOnMutedChn)
	{
		//AddToLog(LogWarning, U_("Global commands on muted channels are interpreted only by some S3M players."));
	}

	mpt::IO::Offset sampleDataOffset = mpt::IO::TellWrite(f);

	// Write samples
	std::vector<S3MSampleHeader> sampleHeader(writeSamples);

	for(SAMPLEINDEX smp = 0; smp < writeSamples; smp++)
	{
		SAMPLEINDEX realSmp = smp + 1;
		if(GetNumInstruments() != 0 && Instruments[smp] != nullptr)
		{
			// Find some valid sample associated with this instrument.
			for(SAMPLEINDEX keySmp : Instruments[smp]->Keyboard)
			{
				if(keySmp > 0 && keySmp <= GetNumSamples())
				{
					realSmp = keySmp;
					break;
				}
			}
		}

		if(realSmp > GetNumSamples())
		{
			continue;
		}

		const SmpLength smpLength = sampleHeader[smp].ConvertToS3M(Samples[realSmp]);
		mpt::String::WriteBuf(mpt::String::nullTerminated, sampleHeader[smp].name) = m_szNames[realSmp];

		if(smpLength != 0)
		{
			// Write sample data
			if(sampleDataOffset > 0xFFFFFF0)
			{
				AddToLog(LogError, MPT_UFORMAT("Too much sample data! Writing samples failed starting from sample {}.")(realSmp));
				break;
			}

			sampleHeader[smp].dataPointer[1] = static_cast<uint8>((sampleDataOffset >> 4) & 0xFF);
			sampleHeader[smp].dataPointer[2] = static_cast<uint8>((sampleDataOffset >> 12) & 0xFF);
			sampleHeader[smp].dataPointer[0] = static_cast<uint8>((sampleDataOffset >> 20) & 0xFF);

			size_t writtenLength = sampleHeader[smp].GetSampleFormat(false).WriteSample(f, Samples[realSmp], smpLength);
			sampleDataOffset += writtenLength;
			if((writtenLength % 16u) != 0)
			{
				size_t fillSize = 16 - (writtenLength % 16u);
				mpt::IO::WriteRaw(f, filler, fillSize);
				sampleDataOffset += fillSize;
			}
		}
	}

	// Channel Table
	uint8 sampleCh = 0, adlibCh = 0;
	for(CHANNELINDEX chn = 0; chn < 32; chn++)
	{
		if(chn < GetNumChannels())
		{
			if(channelType[chn][S3MChannelType::kPCM] && channelType[chn][S3MChannelType::kAdlib])
			{
				AddToLog(LogWarning, MPT_UFORMAT("Pattern channel {} constains both samples and OPL instruments, which is not supported by Scream Tracker 3.")(chn + 1));
			}
			// ST3 only supports 16 PCM channels, so if channels 17-32 are used,
			// they must be mapped to the same "internal channels" as channels 1-16.
			// The channel indices determine in which order channels are evaluated in ST3.
			// First, the "left" channels (0...7) are evaluated, then the "right" channels (8...15).
			// Previously, an alternating LRLR scheme was written, which would lead to a different
			// effect processing in ST3 than LLL...RRR, but since OpenMPT doesn't care about the
			// channel order and always parses them left to right as they appear in the pattern,
			// we should just write in the LLL...RRR manner.
			uint8 ch = sampleCh % 16u; // If there are neither PCM nor AdLib instruments on this channel, just fall back a regular sample-based channel for maximum compatibility.
			if(channelType[chn][S3MChannelType::kPCM])
				ch = (sampleCh++) % 16u;
			else if(channelType[chn][S3MChannelType::kAdlib])
				ch = 16 + ((adlibCh++) % 9u);

			if(saveMuteStatus && ChnSettings[chn].dwFlags[CHN_MUTE])
			{
				ch |= 0x80;
			}
			fileHeader.channels[chn] = ch;
		} else
		{
			fileHeader.channels[chn] = 0xFF;
		}
	}
	if(sampleCh > 16)
	{
		AddToLog(LogWarning, MPT_UFORMAT("This module has more than 16 ({}) sample channels, which is not supported by Scream Tracker 3.")(sampleCh));
	}
	if(adlibCh > 9)
	{
		AddToLog(LogWarning, MPT_UFORMAT("This module has more than 9 ({}) OPL channels, which is not supported by Scream Tracker 3.")(adlibCh));
	}

	mpt::IO::SeekAbsolute(f, 0);
	mpt::IO::Write(f, fileHeader);

	// Now we know where the patterns are.
	if(writePatterns != 0)
	{
		mpt::IO::SeekAbsolute(f, patternPointerOffset);
		mpt::IO::Write(f, patternOffsets);
	}

	// And we can finally write the sample headers.
	if(writeSamples != 0)
	{
		mpt::IO::SeekAbsolute(f, sampleHeaderOffset);
		mpt::IO::Write(f, sampleHeader);
	}

	return true;
}

#endif // MODPLUG_NO_FILESAVE


OPENMPT_NAMESPACE_END
