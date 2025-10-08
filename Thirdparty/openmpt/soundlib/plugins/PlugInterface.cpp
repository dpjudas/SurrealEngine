/*
 * PlugInterface.cpp
 * -----------------
 * Purpose: Default plugin interface implementation
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "PlugInterface.h"
#include "../../common/FileReader.h"
#include "../Sndfile.h"
#include "PluginManager.h"
#ifdef MODPLUG_TRACKER
#include "../../mptrack/AbstractVstEditor.h"
#include "../../mptrack/DefaultVstEditor.h"
#include "../../mptrack/InputHandler.h"
#include "../../mptrack/Mainfrm.h"
#include "../../mptrack/Moddoc.h"
#include "../../mptrack/Reporting.h"
#include "../../mptrack/TrackerSettings.h"
#include "../../mptrack/WindowMessages.h"
// LoadProgram/SaveProgram
#include "../mod_specifications.h"
#include "../../common/mptFileIO.h"
#include "../../mptrack/FileDialog.h"
#include "../../mptrack/VstPresets.h"
#include "mpt/io_file/inputfile.hpp"
#include "mpt/io_file_read/inputfile_filecursor.hpp"
#include "mpt/io_file/outputfile.hpp"
#include "mpt/fs/fs.hpp"
#endif // MODPLUG_TRACKER

#include "../../soundlib/AudioCriticalSection.h"
#include "mpt/base/aligned_array.hpp"
#include "mpt/io/base.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_span.hpp"

#include <cmath>

#ifndef NO_PLUGINS

OPENMPT_NAMESPACE_BEGIN


#ifdef MODPLUG_TRACKER
CModDoc *IMixPlugin::GetModDoc() { return m_SndFile.GetpModDoc(); }
const CModDoc *IMixPlugin::GetModDoc() const { return m_SndFile.GetpModDoc(); }
#endif // MODPLUG_TRACKER


IMixPlugin::IMixPlugin(VSTPluginLib &factory, CSoundFile &sndFile, SNDMIXPLUGIN &mixStruct)
	: m_Factory(factory)
	, m_SndFile(sndFile)
	, m_pMixStruct(&mixStruct)
{
	m_SndFile.m_loadedPlugins++;
	m_MixState.pMixBuffer = mpt::align_bytes<8, MIXBUFFERSIZE * 2>(m_MixBuffer);
	while(m_pMixStruct != &(m_SndFile.m_MixPlugins[m_nSlot]) && m_nSlot < MAX_MIXPLUGINS - 1)
	{
		m_nSlot++;
	}
}


IMixPlugin::~IMixPlugin()
{
#ifdef MODPLUG_TRACKER
	CloseEditor();
	CriticalSection cs;
#endif // MODPLUG_TRACKER
	m_pMixStruct->pMixPlugin = nullptr;
	m_SndFile.m_loadedPlugins--;
	m_pMixStruct = nullptr;
}


#ifdef MODPLUG_TRACKER

void IMixPlugin::SetSlot(PLUGINDEX slot)
{
	m_nSlot = slot;
	m_pMixStruct = &m_SndFile.m_MixPlugins[slot];
}


PlugParamValue IMixPlugin::GetScaledUIParam(PlugParamIndex param)
{
	const auto [paramMin, paramMax] = GetParamUIRange(param);
	return (std::clamp(GetParameter(param), paramMin, paramMax) - paramMin) / (paramMax - paramMin);
}


void IMixPlugin::SetScaledUIParam(PlugParamIndex param, PlugParamValue value)
{
	const auto [paramMin, paramMax] = GetParamUIRange(param);
	const auto scaledVal = paramMin + std::clamp(value, 0.0f, 1.0f) * (paramMax - paramMin);
	SetParameter(param, scaledVal);
}


CString IMixPlugin::GetFormattedParamName(PlugParamIndex param)
{
	CString paramName = GetParamName(param);
	CString name;
	if(paramName.IsEmpty())
	{
		name = MPT_CFORMAT("{}: Parameter {}")(mpt::cfmt::dec0<2>(param), mpt::cfmt::dec0<2>(param));
	} else
	{
		name = MPT_CFORMAT("{}: {}")(mpt::cfmt::dec0<2>(param), paramName);
	}
	return name;
}


// Get a parameter's current value, represented by the plugin.
CString IMixPlugin::GetFormattedParamValue(PlugParamIndex param)
{
	CString paramDisplay = GetParamDisplay(param);
	CString paramUnits = GetParamLabel(param);
	paramDisplay.Trim();
	paramUnits.Trim();
	paramDisplay += _T(" ") + paramUnits;

	return paramDisplay;
}


CString IMixPlugin::GetFormattedProgramName(int32 index)
{
	CString rawname = GetProgramName(index);
	
	// Let's start counting at 1 for the program name (as most MIDI hardware / software does)
	index++;

	CString formattedName;
	if(rawname[0] >= 0 && rawname[0] < _T(' '))
		formattedName = MPT_CFORMAT("{} - Program {}")(mpt::cfmt::dec0<2>(index), index);
	else
		formattedName = MPT_CFORMAT("{} - {}")(mpt::cfmt::dec0<2>(index), rawname);

	return formattedName;
}


void IMixPlugin::SetEditorPos(int32 x, int32 y)
{
	m_pMixStruct->editorX = x;
	m_pMixStruct->editorY = y;
}


void IMixPlugin::GetEditorPos(int32 &x, int32 &y) const
{
	x = m_pMixStruct->editorX;
	y = m_pMixStruct->editorY;
}


#endif // MODPLUG_TRACKER


bool IMixPlugin::IsBypassed() const
{
	return m_pMixStruct != nullptr && m_pMixStruct->IsBypassed();
}


void IMixPlugin::RecalculateGain()
{
	float gain = 0.1f * static_cast<float>(m_pMixStruct ? m_pMixStruct->GetGain() : 10);
	if(gain < 0.1f) gain = 1.0f;

	if(IsInstrument())
	{
		gain /= m_SndFile.GetPlayConfig().getVSTiAttenuation();
		gain = gain * (static_cast<float>(m_SndFile.m_nVSTiVolume) / m_SndFile.GetPlayConfig().getNormalVSTiVol());
	}
	m_fGain = gain;
}


void IMixPlugin::SetDryRatio(float dryRatio)
{
	m_pMixStruct->fDryRatio = std::clamp(dryRatio, 0.0f, 1.0f);
#ifdef MODPLUG_TRACKER
	m_SndFile.m_pluginDryWetRatioChanged.set(m_nSlot);
#endif  // MODPLUG_TRACKER
}


void IMixPlugin::Bypass(bool bypass)
{
	m_pMixStruct->Info.SetBypass(bypass);

#ifdef MODPLUG_TRACKER
	if(m_SndFile.GetpModDoc())
		m_SndFile.GetpModDoc()->UpdateAllViews(PluginHint(m_nSlot + 1).Info());
#endif // MODPLUG_TRACKER
}


double IMixPlugin::GetOutputLatency() const
{
	if(GetSoundFile().IsRenderingToDisc())
		return 0;
	else
		return GetSoundFile().m_TimingInfo.OutputLatency;
}


void IMixPlugin::ProcessMixOps(float * MPT_RESTRICT pOutL, float * MPT_RESTRICT pOutR, float * MPT_RESTRICT leftPlugOutput, float * MPT_RESTRICT rightPlugOutput, uint32 numFrames)
{
/*	float *leftPlugOutput;
	float *rightPlugOutput;

	if(m_Effect.numOutputs == 1)
	{
		// If there was just the one plugin output we copy it into our 2 outputs
		leftPlugOutput = rightPlugOutput = mixBuffer.GetOutputBuffer(0);
	} else if(m_Effect.numOutputs > 1)
	{
		// Otherwise we actually only cater for two outputs max (outputs > 2 have been mixed together already).
		leftPlugOutput = mixBuffer.GetOutputBuffer(0);
		rightPlugOutput = mixBuffer.GetOutputBuffer(1);
	} else
	{
		return;
	}*/

	// -> mixop == 0 : normal processing
	// -> mixop == 1 : MIX += DRY - WET * wetRatio
	// -> mixop == 2 : MIX += WET - DRY * dryRatio
	// -> mixop == 3 : MIX -= WET - DRY * wetRatio
	// -> mixop == 4 : MIX -= middle - WET * wetRatio + middle - DRY
	// -> mixop == 5 : MIX_L += wetRatio * (WET_L - DRY_L) + dryRatio * (DRY_R - WET_R)
	//                 MIX_R += dryRatio * (WET_L - DRY_L) + wetRatio * (DRY_R - WET_R)
	// -> mixop == 6:  same as normal, but forces dry ratio to 1

	const PluginMixMode mixop = m_pMixStruct->GetMixMode();

	float wetRatio = 1.f - m_pMixStruct->fDryRatio;
	float dryRatio = (mixop == PluginMixMode::Instrument) ? 1.f : m_pMixStruct->fDryRatio;

	// Wet / Dry range expansion [0,1] -> [-1,1]
	if(m_pMixStruct->IsExpandedMix())
	{
		wetRatio = 2.0f * wetRatio - 1.0f;
		dryRatio = -wetRatio;
	}

	wetRatio *= m_fGain;
	dryRatio *= m_fGain;

	float * MPT_RESTRICT plugInputL = m_mixBuffer.GetInputBuffer(0);
	float * MPT_RESTRICT plugInputR = m_mixBuffer.GetInputBuffer(1);

	// Mix operation
	switch(mixop)
	{

	// Default mix
	case PluginMixMode::Default:
	case PluginMixMode::Instrument:
		for(uint32 i = 0; i < numFrames; i++)
		{
			pOutL[i] += leftPlugOutput[i] * wetRatio + plugInputL[i] * dryRatio;
			pOutR[i] += rightPlugOutput[i] * wetRatio + plugInputR[i] * dryRatio;
		}
		break;

	// Wet subtract
	case PluginMixMode::WetSubtract:
		for(uint32 i = 0; i < numFrames; i++)
		{
			pOutL[i] += plugInputL[i] - leftPlugOutput[i] * wetRatio;
			pOutR[i] += plugInputR[i] - rightPlugOutput[i] * wetRatio;
		}
		break;

	// Dry subtract
	case PluginMixMode::DrySubtract:
		for(uint32 i = 0; i < numFrames; i++)
		{
			pOutL[i] += leftPlugOutput[i] - plugInputL[i] * dryRatio;
			pOutR[i] += rightPlugOutput[i] - plugInputR[i] * dryRatio;
		}
		break;

	// Mix subtract
	case PluginMixMode::MixSubtract:
		for(uint32 i = 0; i < numFrames; i++)
		{
			pOutL[i] -= leftPlugOutput[i] - plugInputL[i] * wetRatio;
			pOutR[i] -= rightPlugOutput[i] - plugInputR[i] * wetRatio;
		}
		break;

	// Middle subtract
	case PluginMixMode::MiddleSubtract:
		for(uint32 i = 0; i < numFrames; i++)
		{
			float middle = (pOutL[i] + plugInputL[i] + pOutR[i] + plugInputR[i]) * 0.5f;
			pOutL[i] -= middle - leftPlugOutput[i] * wetRatio + middle - plugInputL[i];
			pOutR[i] -= middle - rightPlugOutput[i] * wetRatio + middle - plugInputR[i];
		}
		break;

	// Left / Right balance
	case PluginMixMode::LRBalance:
		if(m_pMixStruct->IsExpandedMix())
		{
			wetRatio /= 2.0f;
			dryRatio /= 2.0f;
		}

		for(uint32 i = 0; i < numFrames; i++)
		{
			pOutL[i] += wetRatio * (leftPlugOutput[i] - plugInputL[i]) + dryRatio * (plugInputR[i] - rightPlugOutput[i]);
			pOutR[i] += dryRatio * (leftPlugOutput[i] - plugInputL[i]) + wetRatio * (plugInputR[i] - rightPlugOutput[i]);
		}
		break;
	}

	// If dry mix is ticked, we add the unprocessed buffer,
	// except with the instrument mixop as it has already been done:
	if(m_pMixStruct->IsDryMix() && mixop != PluginMixMode::Instrument)
	{
		for(uint32 i = 0; i < numFrames; i++)
		{
			pOutL[i] += plugInputL[i];
			pOutR[i] += plugInputR[i];
		}
	}
}


// Render some silence and return maximum level returned by the plugin.
float IMixPlugin::RenderSilence(uint32 numFrames)
{
	// The JUCE framework doesn't like processing while being suspended.
	const bool wasSuspended = !IsResumed();
	if(wasSuspended)
	{
		Resume();
	}

	float out[2][MIXBUFFERSIZE]; // scratch buffers
	float maxVal = 0.0f;
	m_mixBuffer.ClearInputBuffers(MIXBUFFERSIZE);

	while(numFrames > 0)
	{
		uint32 renderSamples = numFrames;
		LimitMax(renderSamples, mpt::saturate_cast<uint32>(std::size(out[0])));
		MemsetZero(out);

		Process(out[0], out[1], renderSamples);
		for(size_t i = 0; i < renderSamples; i++)
		{
			maxVal = std::max(maxVal, std::fabs(out[0][i]));
			maxVal = std::max(maxVal, std::fabs(out[1][i]));
		}

		numFrames -= renderSamples;
	}

	if(wasSuspended)
	{
		Suspend();
	}

	return maxVal;
}


bool IMixPlugin::MidiSend(uint32 midiCode)
{
	std::array<std::byte, 4> midiData;
	memcpy(midiData.data(), &midiCode, 4);
	return MidiSend(mpt::as_span(midiData.data(), std::min(static_cast<uint8>(midiData.size()), MIDIEvents::GetEventLength(mpt::byte_cast<uint8>(midiData[0])))));
}


// Get list of plugins to which output is sent. A nullptr indicates master output.
size_t IMixPlugin::GetOutputPlugList(std::vector<IMixPlugin *> &list)
{
	// At the moment we know there will only be 1 output.
	// Returning nullptr means plugin outputs directly to master.
	list.clear();

	IMixPlugin *outputPlug = nullptr;
	if(!m_pMixStruct->IsOutputToMaster())
	{
		PLUGINDEX nOutput = m_pMixStruct->GetOutputPlugin();
		if(nOutput > m_nSlot && nOutput != PLUGINDEX_INVALID)
		{
			outputPlug = m_SndFile.m_MixPlugins[nOutput].pMixPlugin;
		}
	}
	list.push_back(outputPlug);

	return 1;
}


// Get a list of plugins that send data to this plugin.
size_t IMixPlugin::GetInputPlugList(std::vector<IMixPlugin *> &list)
{
	std::vector<IMixPlugin *> candidatePlugOutputs;
	list.clear();

	for(PLUGINDEX plug = 0; plug < MAX_MIXPLUGINS; plug++)
	{
		IMixPlugin *candidatePlug = m_SndFile.m_MixPlugins[plug].pMixPlugin;
		if(candidatePlug)
		{
			candidatePlug->GetOutputPlugList(candidatePlugOutputs);

			for(auto &outPlug : candidatePlugOutputs)
			{
				if(outPlug == this)
				{
					list.push_back(candidatePlug);
					break;
				}
			}
		}
	}

	return list.size();
}


// Get a list of instruments that send data to this plugin.
size_t IMixPlugin::GetInputInstrumentList(std::vector<INSTRUMENTINDEX> &list)
{
	list.clear();
	const PLUGINDEX nThisMixPlug = m_nSlot + 1;		//m_nSlot is position in mixplug array.

	for(INSTRUMENTINDEX ins = 0; ins <= m_SndFile.GetNumInstruments(); ins++)
	{
		if(m_SndFile.Instruments[ins] != nullptr && m_SndFile.Instruments[ins]->nMixPlug == nThisMixPlug)
		{
			list.push_back(ins);
		}
	}

	return list.size();
}


size_t IMixPlugin::GetInputChannelList(std::vector<CHANNELINDEX> &list)
{
	list.clear();

	PLUGINDEX nThisMixPlug = m_nSlot + 1;		//m_nSlot is position in mixplug array.
	const CHANNELINDEX chnCount = m_SndFile.GetNumChannels();
	for(CHANNELINDEX nChn=0; nChn<chnCount; nChn++)
	{
		if(m_SndFile.ChnSettings[nChn].nMixPlugin == nThisMixPlug)
		{
			list.push_back(nChn);
		}
	}

	return list.size();

}


void IMixPlugin::SaveAllParameters()
{
	if (m_pMixStruct == nullptr)
	{
		return;
	}
	m_pMixStruct->defaultProgram = -1;
	
	// Default implementation: Save all parameter values
	PlugParamIndex numParams = std::min(GetNumParameters(), static_cast<PlugParamIndex>((std::numeric_limits<uint32>::max() - sizeof(uint32)) / sizeof(IEEE754binary32LE)));
	uint32 nLen = numParams * sizeof(IEEE754binary32LE);
	if (!nLen) return;
	nLen += sizeof(uint32);

	try
	{
		m_pMixStruct->pluginData.resize(nLen);
		auto memFile = std::make_pair(mpt::as_span(m_pMixStruct->pluginData), mpt::IO::Offset(0));
		mpt::IO::WriteIntLE<uint32>(memFile, 0);	// Plugin data type
		BeginGetProgram();
		for(PlugParamIndex i = 0; i < numParams; i++)
		{
			mpt::IO::Write(memFile, IEEE754binary32LE(GetParameter(i)));
		}
		EndGetProgram();
	} catch(mpt::out_of_memory e)
	{
		m_pMixStruct->pluginData.clear();
		mpt::delete_out_of_memory(e);
	}
}


void IMixPlugin::RestoreAllParameters(int32 /*program*/)
{
	if(m_pMixStruct != nullptr && m_pMixStruct->pluginData.size() >= sizeof(uint32))
	{
		FileReader memFile(mpt::as_span(m_pMixStruct->pluginData));
		uint32 type = memFile.ReadUint32LE();
		if(type == 0)
		{
			const uint32 numParams = GetNumParameters();
			if((m_pMixStruct->pluginData.size() - sizeof(uint32)) >= (numParams * sizeof(IEEE754binary32LE)))
			{
				BeginSetProgram();
				for(uint32 i = 0; i < numParams; i++)
				{
					const auto value = memFile.ReadFloatLE();
					SetParameter(i, std::isfinite(value) ? value : 0.0f);
				}
				EndSetProgram();
			}
		}
	}
}


#ifdef MODPLUG_TRACKER
void IMixPlugin::ToggleEditor()
{
	// We only really need this mutex for bridged plugins, as we may be processing window messages (in the same thread) while the editor opens.
	// The user could press the toggle button while the editor is loading and thus close the editor while still being initialized.
	// Note that this does not protect against closing the module while the editor is still loading.
	static bool initializing = false;
	if(initializing)
		return;
	initializing = true;

	if (m_pEditor)
	{
		CloseEditor();
	} else
	{
		m_pEditor = OpenEditor();

		if (m_pEditor)
			m_pEditor->OpenEditor(CMainFrame::GetMainFrame());
	}
	initializing = false;
}


// Provide default plugin editor
CAbstractVstEditor *IMixPlugin::OpenEditor()
{
	try
	{
		return new CDefaultVstEditor(*this);
	} catch(mpt::out_of_memory e)
	{
		mpt::delete_out_of_memory(e);
		return nullptr;
	}
}


void IMixPlugin::CloseEditor()
{
	if(m_pEditor)
	{
		if (m_pEditor->m_hWnd) m_pEditor->DoClose();
		delete m_pEditor;
		m_pEditor = nullptr;
	}
}


// Automate a parameter from the plugin GUI (both custom and default plugin GUI)
void IMixPlugin::AutomateParameter(PlugParamIndex param)
{
	CModDoc *modDoc = GetModDoc();
	if(modDoc == nullptr)
	{
		return;
	}

	// TODO: Check if any params are actually automatable, and if there are but this one isn't, chicken out

	if(m_recordAutomation)
	{
		// Record parameter change
		modDoc->RecordParamChange(GetSlot(), param);
	}

	modDoc->SendNotifyMessageToAllViews(WM_MOD_PLUGPARAMAUTOMATE, m_nSlot, param);

	if(auto *vstEditor = GetEditor(); vstEditor && vstEditor->m_hWnd)
	{
		// Mark track modified if GUI is open and format supports plugins
		SetModified();

		// Do not use InputHandler in case we are coming from a bridged plugin editor
		if((GetAsyncKeyState(VK_SHIFT) & 0x8000) && TrackerSettings::Instance().midiMappingInPluginEditor)
		{
			// Shift pressed -> Open MIDI mapping dialog
			CMainFrame::GetMainFrame()->PostMessage(WM_MOD_MIDIMAPPING, m_nSlot, param);
		}

		// Learn macro
		int macroToLearn = vstEditor->GetLearnMacro();
		if (macroToLearn > -1)
		{
			modDoc->LearnMacro(macroToLearn, param);
			vstEditor->SetLearnMacro(-1);
		}
	}
}


void IMixPlugin::SetModified()
{
	CModDoc *modDoc = GetModDoc();
	if(modDoc != nullptr && m_SndFile.GetModSpecifications().supportsPlugins)
	{
		modDoc->SetModified();
	}
}


bool IMixPlugin::SaveProgram()
{
	mpt::PathString defaultDir = TrackerSettings::Instance().PathPluginPresets.GetWorkingDir();
	const bool useDefaultDir = !defaultDir.empty();
	if(!useDefaultDir && mpt::native_fs{}.is_file(m_Factory.dllPath))
	{
		defaultDir = m_Factory.dllPath.GetDirectoryWithDrive();
	}

	CString progName = m_Factory.libraryName.ToCString() + _T(" - ") + GetCurrentProgramName();
	progName = mpt::SanitizePathComponent(progName);

	FileDialog dlg = SaveFileDialog()
		.DefaultExtension("fxb")
		.DefaultFilename(progName)
		.ExtensionFilter("VST Plugin Programs (*.fxp)|*.fxp|"
			"VST Plugin Banks (*.fxb)|*.fxb||")
		.WorkingDirectory(defaultDir);
	if(!dlg.Show(m_pEditor)) return false;

	if(useDefaultDir)
	{
		TrackerSettings::Instance().PathPluginPresets.SetWorkingDir(dlg.GetWorkingDirectory());
	}

	const bool isBank = (dlg.GetExtension() == P_("fxb"));

	try
	{
		mpt::IO::SafeOutputFile sf(dlg.GetFirstFile(), std::ios::binary, mpt::IO::FlushModeFromBool(TrackerSettings::Instance().MiscFlushFileBuffersOnSave));
		mpt::IO::ofstream &f = sf;
		f.exceptions(f.exceptions() | std::ios::badbit | std::ios::failbit);
		if(f.good() && VSTPresets::SaveFile(f, *this, isBank))
			return true;
	} catch(const std::exception &)
	{
		
	}
	Reporting::Error("Error saving preset.", m_pEditor);
	return false;
}


bool IMixPlugin::LoadProgram(mpt::PathString fileName)
{
	mpt::PathString defaultDir = TrackerSettings::Instance().PathPluginPresets.GetWorkingDir();
	bool useDefaultDir = !defaultDir.empty();
	if(!useDefaultDir && mpt::native_fs{}.is_file(m_Factory.dllPath))
	{
		defaultDir = m_Factory.dllPath.GetDirectoryWithDrive();
	}

	if(fileName.empty())
	{
		FileDialog dlg = OpenFileDialog()
			.DefaultExtension("fxp")
			.ExtensionFilter("VST Plugin Programs and Banks (*.fxp,*.fxb)|*.fxp;*.fxb|"
			"VST Plugin Programs (*.fxp)|*.fxp|"
			"VST Plugin Banks (*.fxb)|*.fxb|"
			"All Files|*.*||")
			.WorkingDirectory(defaultDir);
		if(!dlg.Show(m_pEditor)) return false;

		if(useDefaultDir)
		{
			TrackerSettings::Instance().PathPluginPresets.SetWorkingDir(dlg.GetWorkingDirectory());
		}
		fileName = dlg.GetFirstFile();
	}

	const char *errorStr = nullptr;
	mpt::IO::InputFile f(fileName, SettingCacheCompleteFileBeforeLoading());
	if(f.IsValid())
	{
		FileReader file = GetFileReader(f);
		errorStr = VSTPresets::GetErrorMessage(VSTPresets::LoadFile(file, *this));
	} else
	{
		errorStr = "Can't open file.";
	}

	if(errorStr == nullptr)
	{
		if(GetModDoc() != nullptr && GetSoundFile().GetModSpecifications().supportsPlugins)
		{
			GetModDoc()->SetModified();
		}
		return true;
	} else
	{
		Reporting::Error(errorStr, m_pEditor);
		return false;
	}
}


#endif // MODPLUG_TRACKER


////////////////////////////////////////////////////////////////////
// IMidiPlugin: Default implementation of plugins with MIDI input //
////////////////////////////////////////////////////////////////////

IMidiPlugin::IMidiPlugin(VSTPluginLib &factory, CSoundFile &sndFile, SNDMIXPLUGIN &mixStruct)
	: IMixPlugin(factory, sndFile, mixStruct)
	, m_MidiCh{{}}
{
	for(auto &chn : m_MidiCh)
	{
		chn.midiPitchBendPos = EncodePitchBendParam(MIDIEvents::pitchBendCentre); // centre pitch bend on all channels
		chn.ResetProgram(sndFile.m_playBehaviour[kPluginDefaultProgramAndBank1]);
	}
}


void IMidiPlugin::ApplyPitchWheelDepth(int32 &value, int8 pwd)
{
	if(pwd != 0)
	{
		value = (value * ((MIDIEvents::pitchBendMax - MIDIEvents::pitchBendCentre + 1) / 64)) / pwd;
	} else
	{
		value = 0;
	}
}


// Get the MIDI channel currently associated with a given tracker channel
uint8 IMidiPlugin::GetMidiChannel(const ModChannel &chn, CHANNELINDEX trackChannel) const
{
	if(auto ins = chn.pModInstrument; ins != nullptr)
		return ins->GetMIDIChannel(chn, trackChannel);
	else
		return 0;
}


uint8 IMidiPlugin::GetMidiChannel(CHANNELINDEX trackChannel) const
{
	if(trackChannel < std::size(m_SndFile.m_PlayState.Chn))
		return GetMidiChannel(m_SndFile.m_PlayState.Chn[trackChannel], trackChannel);
	else
		return 0;
}


void IMidiPlugin::MidiCC(MIDIEvents::MidiCC nController, uint8 nParam, CHANNELINDEX trackChannel)
{
	//Error checking
	LimitMax(nController, MIDIEvents::MIDICC_end);
	LimitMax(nParam, uint8(127));
	const auto midiCh = GetMidiChannel(trackChannel);

	if(m_SndFile.m_playBehaviour[kMIDICCBugEmulation])
		MidiSend(MIDIEvents::Event(MIDIEvents::evControllerChange, midiCh, nParam, static_cast<uint8>(nController)));	// param and controller are swapped (old broken implementation)
	else
		MidiSend(MIDIEvents::CC(nController, midiCh, nParam));
}


// Set MIDI pitch for given MIDI channel to the specified raw 14-bit position
void IMidiPlugin::MidiPitchBendRaw(int32 pitchbend, CHANNELINDEX trackerChn)
{
	SendMidiPitchBend(GetMidiChannel(trackerChn), EncodePitchBendParam(Clamp(pitchbend, MIDIEvents::pitchBendMin, MIDIEvents::pitchBendMax)));
}


// Bend MIDI pitch for given MIDI channel using fine tracker param (one unit = 1/64th of a note step)
void IMidiPlugin::MidiPitchBend(int32 increment, int8 pwd, CHANNELINDEX trackerChn)
{
	const auto midiCh = GetMidiChannel(trackerChn);
	if(m_SndFile.m_playBehaviour[kOldMIDIPitchBends])
	{
		// OpenMPT Legacy: Old pitch slides never were really accurate, but setting the PWD to 13 in plugins would give the closest results.
		increment = (increment * 0x800 * 13) / (0xFF * pwd);
		increment = EncodePitchBendParam(increment);
	} else
	{
		increment = EncodePitchBendParam(increment);
		ApplyPitchWheelDepth(increment, pwd);
	}

	int32 newPitchBendPos = (increment + m_MidiCh[midiCh].midiPitchBendPos) & kPitchBendMask;
	Limit(newPitchBendPos, EncodePitchBendParam(MIDIEvents::pitchBendMin), EncodePitchBendParam(MIDIEvents::pitchBendMax));

	SendMidiPitchBend(midiCh, newPitchBendPos);
}


void IMidiPlugin::MidiTonePortamento(int32 increment, uint8 newNote, int8 pwd, CHANNELINDEX trackerChn)
{
	const auto midiCh = GetMidiChannel(trackerChn);

	int32 targetBend = EncodePitchBendParam(64 * (newNote - static_cast<int32>(m_MidiCh[midiCh].lastNote)));
	ApplyPitchWheelDepth(targetBend, pwd);
	targetBend += EncodePitchBendParam(MIDIEvents::pitchBendCentre);

	if(targetBend < m_MidiCh[midiCh].midiPitchBendPos)
		increment = -increment;
	increment = EncodePitchBendParam(increment);
	ApplyPitchWheelDepth(increment, pwd);

	int32 newPitchBendPos = (increment + m_MidiCh[midiCh].midiPitchBendPos) & kPitchBendMask;
	if((newPitchBendPos > targetBend && increment > 0) || (newPitchBendPos < targetBend && increment < 0))
		newPitchBendPos = targetBend;

	Limit(newPitchBendPos, EncodePitchBendParam(MIDIEvents::pitchBendMin), EncodePitchBendParam(MIDIEvents::pitchBendMax));

	SendMidiPitchBend(midiCh, newPitchBendPos);
}


// Set MIDI pitch for given MIDI channel using fixed point pitch bend value (converted back to 0-16383 MIDI range)
void IMidiPlugin::SendMidiPitchBend(uint8 midiCh, int32 newPitchBendPos)
{
	MPT_ASSERT(EncodePitchBendParam(MIDIEvents::pitchBendMin) <= newPitchBendPos && newPitchBendPos <= EncodePitchBendParam(MIDIEvents::pitchBendMax));
	m_MidiCh[midiCh].midiPitchBendPos = newPitchBendPos;
	MidiSend(MIDIEvents::PitchBend(midiCh, DecodePitchBendParam(newPitchBendPos)));
}


// Apply vibrato effect through pitch wheel commands on a given MIDI channel.
void IMidiPlugin::MidiVibrato(int32 depth, int8 pwd, CHANNELINDEX trackerChn)
{
	const auto midiCh = GetMidiChannel(trackerChn);
	depth = EncodePitchBendParam(depth);
	if(depth != 0 || (m_MidiCh[midiCh].midiPitchBendPos & kVibratoFlag))
	{
		ApplyPitchWheelDepth(depth, pwd);

		// Temporarily add vibrato offset to current pitch
		int32 newPitchBendPos = (depth + m_MidiCh[midiCh].midiPitchBendPos) & kPitchBendMask;
		Limit(newPitchBendPos, EncodePitchBendParam(MIDIEvents::pitchBendMin), EncodePitchBendParam(MIDIEvents::pitchBendMax));

		MidiSend(MIDIEvents::PitchBend(midiCh, DecodePitchBendParam(newPitchBendPos)));
	}

	// Update vibrato status
	if(depth != 0)
		m_MidiCh[midiCh].midiPitchBendPos |= kVibratoFlag;
	else
		m_MidiCh[midiCh].midiPitchBendPos &= ~kVibratoFlag;
}


void IMidiPlugin::MidiCommand(const ModInstrument &instr, uint16 note, uint16 vol, CHANNELINDEX trackChannel)
{
	if(trackChannel >= MAX_CHANNELS)
		return;

	const auto midiCh = GetMidiChannel(trackChannel);
	PlugInstrChannel &channel = m_MidiCh[midiCh];
	uint8 rawNote = static_cast<uint8>(note & MIDI_NOTE_MASK);

	uint16 midiBank = instr.wMidiBank - 1;
	uint8 midiProg = instr.nMidiProgram - 1;
	bool bankChanged = (channel.currentBank != midiBank) && (midiBank < 0x4000);
	bool progChanged = (channel.currentProgram != midiProg) && (midiProg < 0x80);
	//get vol in [0,128[
	uint8 volume = static_cast<uint8>(std::min((vol + 1u) / 2u, 127u));

	// Bank change
	if(bankChanged)
	{
		uint8 high = static_cast<uint8>(midiBank >> 7);
		uint8 low = static_cast<uint8>(midiBank & 0x7F);

		//m_SndFile.ProcessMIDIMacro(trackChannel, false, m_SndFile.m_MidiCfg.Global[MIDIOUT_BANKSEL], 0, m_nSlot + 1);
		MidiSend(MIDIEvents::CC(MIDIEvents::MIDICC_BankSelect_Coarse, midiCh, high));
		MidiSend(MIDIEvents::CC(MIDIEvents::MIDICC_BankSelect_Fine, midiCh, low));

		channel.currentBank = midiBank;
	}

	// Program change
	// According to the MIDI specs, a bank change alone doesn't have to change the active program - it will only change the bank of subsequent program changes.
	// Thus we send program changes also if only the bank has changed.
	if(progChanged || (midiProg < 0x80 && bankChanged))
	{
		channel.currentProgram = midiProg;
		//m_SndFile.ProcessMIDIMacro(trackChannel, false, m_SndFile.m_MidiCfg.Global[MIDIOUT_PROGRAM], 0, m_nSlot + 1);
		MidiSend(MIDIEvents::ProgramChange(midiCh, midiProg));
	}


	// Specific Note Off
	if(note & MIDI_NOTE_OFF)
	{
		uint8 i = rawNote - NOTE_MIN;
		if(i < mpt::array_size<decltype(channel.noteOnMap)>::size && channel.noteOnMap[i][trackChannel])
		{
			channel.noteOnMap[i][trackChannel]--;
			MidiSend(MIDIEvents::NoteOff(midiCh, i, 0));
		}
	}

	// "Hard core" All Sounds Off on this midi and tracker channel
	// This one doesn't check the note mask - just one note off per note.
	// Also less likely to cause a VST event buffer overflow.
	else if(note == NOTE_NOTECUT)  // ^^
	{
		MidiSend(MIDIEvents::CC(MIDIEvents::MIDICC_AllNotesOff, midiCh, 0));
		MidiSend(MIDIEvents::CC(MIDIEvents::MIDICC_AllSoundOff, midiCh, 0));

		// Turn off all notes
		for(uint8 i = 0; i < std::size(channel.noteOnMap); i++)
		{
			channel.noteOnMap[i][trackChannel] = 0;
			MidiSend(MIDIEvents::NoteOff(midiCh, i, volume));
		}

	}

	// All "active" notes off on this midi and tracker channel
	// using note mask.
	else if(note == NOTE_KEYOFF || note == NOTE_FADE)  // ==, ~~
	{
		for(uint8 i = 0; i < std::size(channel.noteOnMap); i++)
		{
			// Some VSTis need a note off for each instance of a note on, e.g. fabfilter.
			while(channel.noteOnMap[i][trackChannel])
			{
				MidiSend(MIDIEvents::NoteOff(midiCh, i, volume));
				channel.noteOnMap[i][trackChannel]--;
			}
		}
	}

	// Note On
	else if(rawNote >= NOTE_MIN && rawNote < NOTE_MIN + mpt::array_size<decltype(channel.noteOnMap)>::size)
	{
		if(!(note & MIDI_NOTE_ARPEGGIO))
		{
			m_MidiCh[midiCh].lastNote = rawNote;
			m_SndFile.m_PlayState.Chn[trackChannel].lastMidiNoteWithoutArp = rawNote;

			// Reset pitch bend on each new note, tracker style.
			// This is done if the pitch wheel has been moved or there was a vibrato on the previous row (in which case the "vstVibratoFlag" bit of the pitch bend memory is set)
			auto newPitchBendPos = EncodePitchBendParam(Clamp(m_SndFile.m_PlayState.Chn[trackChannel].GetMIDIPitchBend(), MIDIEvents::pitchBendMin, MIDIEvents::pitchBendMax));
			if(m_MidiCh[midiCh].midiPitchBendPos != newPitchBendPos)
			{
				SendMidiPitchBend(midiCh, newPitchBendPos);
			}
		}

		// count instances of active notes.
		// This is to send a note off for each instance of a note, for plugs like Fabfilter.
		// Problem: if a note dies out naturally and we never send a note off, this counter
		// will block at max until note off. Is this a problem?
		// Safe to assume we won't need more than 255 note offs max on a given note?
		rawNote -= NOTE_MIN;
		if(channel.noteOnMap[rawNote][trackChannel] < uint8_max)
		{
			channel.noteOnMap[rawNote][trackChannel]++;
		}

		MidiSend(MIDIEvents::NoteOn(midiCh, rawNote, volume));
	}
}


bool IMidiPlugin::IsNotePlaying(uint8 note, CHANNELINDEX trackerChn)
{
	if(!ModCommand::IsNote(note) || trackerChn >= std::size(m_MidiCh[GetMidiChannel(trackerChn)].noteOnMap[note]))
		return false;

	note -= NOTE_MIN;
	return (m_MidiCh[GetMidiChannel(trackerChn)].noteOnMap[note][trackerChn] != 0);
}


void IMidiPlugin::MoveChannel(CHANNELINDEX from, CHANNELINDEX to)
{
	if(from >= std::size(m_MidiCh[GetMidiChannel(from)].noteOnMap[0]) || to >= std::size(m_MidiCh[GetMidiChannel(from)].noteOnMap[0]))
		return;

	for(auto &noteOnMap : m_MidiCh[GetMidiChannel(from)].noteOnMap)
	{
		noteOnMap[to] = noteOnMap[from];
		noteOnMap[from] = 0;
	}
}


void IMidiPlugin::ReceiveMidi(mpt::const_byte_span midiData)
{
	if(midiData.empty())
		return;

	ResetSilence();

	// I think we should only route events to plugins that are explicitely specified as output plugins of the current plugin.
	// This should probably use GetOutputPlugList here if we ever get to support multiple output plugins.
	PLUGINDEX receiver;
	if(m_pMixStruct != nullptr && (receiver = m_pMixStruct->GetOutputPlugin()) != PLUGINDEX_INVALID)
	{
		IMixPlugin *plugin = m_SndFile.m_MixPlugins[receiver].pMixPlugin;
		// Add all events to the plugin's queue.
		plugin->MidiSend(midiData);
	}

#ifdef MODPLUG_TRACKER
	if(m_recordMIDIOut && midiData[0] != std::byte{0xF0})
	{
		// Spam MIDI data to all views
		uint32 midiCode = 0;
		memcpy(&midiCode, midiData.data(), std::min(sizeof(midiCode), midiData.size()));
		::PostMessage(CMainFrame::GetMainFrame()->GetMidiRecordWnd(), WM_MOD_MIDIMSG, midiCode, reinterpret_cast<LPARAM>(this));
	}
#endif  // MODPLUG_TRACKER
}


// SNDMIXPLUGIN functions

void SNDMIXPLUGIN::SetGain(uint8 gain)
{
	Info.gain = gain;
	if(pMixPlugin != nullptr) pMixPlugin->RecalculateGain();
}


void SNDMIXPLUGIN::SetBypass(bool bypass)
{
	if(pMixPlugin != nullptr)
		pMixPlugin->Bypass(bypass);
	else
		Info.SetBypass(bypass);
}


void SNDMIXPLUGIN::Destroy()
{
	if(pMixPlugin)
	{
		CriticalSection cs;
		pMixPlugin->GetPluginFactory().RemovePluginInstanceFromList(*pMixPlugin);
		pMixPlugin->Release();
		pMixPlugin = nullptr;
	}
	pluginData.clear();
	pluginData.shrink_to_fit();
}

OPENMPT_NAMESPACE_END

#endif // NO_PLUGINS
