/*
 * PluginManager.cpp
 * -----------------
 * Purpose: Implementation of the plugin manager, which keeps a list of known plugins and instantiates them.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#ifndef NO_PLUGINS

#include "PluginManager.h"
#include "PlugInterface.h"
#include "../../common/version.h"

#if defined(MPT_WITH_DMO)
#include "mpt/uuid/guid.hpp"
#endif // MPT_WITH_DMO
#include "mpt/uuid/uuid.hpp"

// Built-in plugins
#include "DigiBoosterEcho.h"
#include "LFOPlugin.h"
#include "SymMODEcho.h"
#include "dmo/DMOPlugin.h"
#include "dmo/Chorus.h"
#include "dmo/Compressor.h"
#include "dmo/Distortion.h"
#include "dmo/Echo.h"
#include "dmo/Flanger.h"
#include "dmo/Gargle.h"
#include "dmo/I3DL2Reverb.h"
#include "dmo/ParamEq.h"
#include "dmo/WavesReverb.h"
#ifdef MODPLUG_TRACKER
#include "../../mptrack/plugins/MidiInOut.h"
#endif // MODPLUG_TRACKER

#include "../../common/mptStringBuffer.h"
#include "../Sndfile.h"
#include "../Loaders.h"

#ifdef MPT_WITH_VST
#include "../../mptrack/Vstplug.h"
#include "../../pluginBridge/BridgeWrapper.h"
#ifdef MODPLUG_TRACKER
#include "mpt/fs/fs.hpp"
#endif // MODPLUG_TRACKER
#endif // MPT_WITH_VST

#if defined(MPT_WITH_DMO)
#include <winreg.h>
#include <strmif.h>
#include <tchar.h>
#endif // MPT_WITH_DMO

#ifdef MODPLUG_TRACKER
#include "../../mptrack/Mptrack.h"
#include "../../mptrack/Reporting.h"
#include "../../mptrack/TrackerSettings.h"
#include "../../mptrack/AbstractVstEditor.h"
#include "../../soundlib/AudioCriticalSection.h"
#include "../mptrack/ExceptionHandler.h"
#include "mpt/crc/crc.hpp"
#endif // MODPLUG_TRACKER


OPENMPT_NAMESPACE_BEGIN


using namespace mpt::uuid_literals;


#ifdef MPT_ALL_LOGGING
#define VST_LOG
#define DMO_LOG
#endif

#ifdef MODPLUG_TRACKER
static constexpr const mpt::uchar *cacheSection = UL_("PluginCache");
#endif // MODPLUG_TRACKER


#ifdef MPT_WITH_VST


uint8 VSTPluginLib::GetNativePluginArch()
{
	uint8 result = 0;
	switch(mpt::OS::Windows::GetProcessArchitecture())
	{
	case mpt::OS::Windows::Architecture::x86:
		result = PluginArch_x86;
		break;
	case mpt::OS::Windows::Architecture::amd64:
		result = PluginArch_amd64;
		break;
	case mpt::OS::Windows::Architecture::arm:
		result = PluginArch_arm;
		break;
	case mpt::OS::Windows::Architecture::arm64:
		result = PluginArch_arm64;
		break;
	default:
		result = 0;
		break;
	}
	return result;
}


mpt::ustring VSTPluginLib::GetPluginArchName(uint8 arch)
{
	mpt::ustring result;
	switch(arch)
	{
	case PluginArch_x86:
		result = U_("x86");
		break;
	case PluginArch_amd64:
		result = U_("amd64");
		break;
	case PluginArch_arm:
		result = U_("arm");
		break;
	case PluginArch_arm64:
		result = U_("arm64");
		break;
	default:
		result = U_("");
		break;
	}
	return result;
}


uint8 VSTPluginLib::GetDllArch(bool fromCache) const
{
	// Built-in plugins are always native.
	if(dllPath.empty())
		return GetNativePluginArch();
#ifdef MPT_WITH_VST
	if(!dllArch || !fromCache)
	{
		dllArch = static_cast<uint8>(BridgeWrapper::GetPluginBinaryType(dllPath));
	}
#else // !MPT_WITH_VST
	MPT_UNREFERENCED_PARAMETER(fromCache);
#endif // MPT_WITH_VST
	return dllArch;
}


mpt::ustring VSTPluginLib::GetDllArchName(bool fromCache) const
{
	return GetPluginArchName(GetDllArch(fromCache));
}


mpt::ustring VSTPluginLib::GetDllArchNameUser(bool fromCache) const
{
	return GetPluginArchName(GetDllArch(fromCache));
}


bool VSTPluginLib::IsNative(bool fromCache) const
{
	return GetDllArch(fromCache) == GetNativePluginArch();
}


bool VSTPluginLib::IsNativeFromCache() const
{
	return dllArch == GetNativePluginArch() || dllArch == 0;
}


#endif // MPT_WITH_VST


// PluginCache format:
// FullDllPath = <ID1><ID2><CRC32> (hex-encoded)
// <ID1><ID2><CRC32>.Flags = Plugin Flags (see VSTPluginLib::DecodeCacheFlags).
// <ID1><ID2><CRC32>.Vendor = Plugin Vendor String.

#ifdef MODPLUG_TRACKER
void VSTPluginLib::WriteToCache() const
{
	SettingsContainer &cacheFile = theApp.GetPluginCache();

	mpt::ustring pathStr;
	if(theApp.IsPortableMode())
		pathStr = theApp.PathAbsoluteToInstallRelative(dllPath).ToUnicode();
	else
		pathStr = dllPath.ToUnicode();

	if(shellPluginID)
		pathStr += U_("|") + mpt::ufmt::HEX0<8>(shellPluginID);

	// CRC is used to distinguish plugins sharing the same IDs
	const std::string crcName = mpt::ToCharset(mpt::Charset::UTF8, pathStr);
	const mpt::crc32 crc(crcName);
	const mpt::ustring IDs = mpt::ufmt::HEX0<8>(static_cast<uint32>(pluginId1)) + mpt::ufmt::HEX0<8>(static_cast<uint32>(pluginId2)) + mpt::ufmt::HEX0<8>(crc.result());

	cacheFile.Write<mpt::ustring>(cacheSection, pathStr, IDs);
	cacheFile.Write<CString>(cacheSection, IDs + U_(".Vendor"), vendor);
	cacheFile.Write<int32>(cacheSection, IDs + U_(".Flags"), EncodeCacheFlags());
}
#endif // MODPLUG_TRACKER


void VSTPluginLib::InsertPluginInstanceIntoList(IMixPlugin &pluginInstance)
{
	pluginInstance.m_pNext = pPluginsList;
	if(pPluginsList)
	{
		pPluginsList->m_pPrev = &pluginInstance;
	}
	pPluginsList = &pluginInstance;
}


void VSTPluginLib::RemovePluginInstanceFromList(IMixPlugin &pluginInstance)
{
	if(pPluginsList == &pluginInstance)
	{
		pPluginsList = pluginInstance.m_pNext;
	}
	if(pluginInstance.m_pNext)
	{
		pluginInstance.m_pNext->m_pPrev = pluginInstance.m_pPrev;
	}
	if(pluginInstance.m_pPrev)
	{
		pluginInstance.m_pPrev->m_pNext = pluginInstance.m_pNext;
	}
	pluginInstance.m_pPrev = nullptr;
	pluginInstance.m_pNext = nullptr;
}


bool CreateMixPluginProc(SNDMIXPLUGIN &mixPlugin, CSoundFile &sndFile)
{
#ifdef MODPLUG_TRACKER
	CVstPluginManager *that = theApp.GetPluginManager();
	if(that)
	{
		return that->CreateMixPlugin(mixPlugin, sndFile);
	}
	return false;
#else
	if(!sndFile.m_PluginManager)
	{
		sndFile.m_PluginManager = std::make_unique<CVstPluginManager>();
	}
	return sndFile.m_PluginManager->CreateMixPlugin(mixPlugin, sndFile);
#endif // MODPLUG_TRACKER
}


CVstPluginManager::CVstPluginManager()
{
#if defined(MPT_WITH_DMO)
	HRESULT COMinit = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(COMinit == S_OK || COMinit == S_FALSE)
	{
		MustUnInitilizeCOM = true;
	}
#endif

	// Hard-coded "plugins"
	static constexpr struct
	{
		VSTPluginLib::CreateProc createProc;
		const char *filename, *name;
		uint32 pluginId1, pluginId2;
		PluginCategory category;
		bool isInstrument, isOurs;
	} BuiltInPlugins[] =
	{
		// DirectX Media Objects Emulation
		{ DMO::Chorus::Create,      "{EFE6629C-81F7-4281-BD91-C9D604A95AF6}", "Chorus",      kDmoMagic, 0xEFE6629C, PluginCategory::DMO, false, false },
		{ DMO::Compressor::Create,  "{EF011F79-4000-406D-87AF-BFFB3FC39D57}", "Compressor",  kDmoMagic, 0xEF011F79, PluginCategory::DMO, false, false },
		{ DMO::Distortion::Create,  "{EF114C90-CD1D-484E-96E5-09CFAF912A21}", "Distortion",  kDmoMagic, 0xEF114C90, PluginCategory::DMO, false, false },
		{ DMO::Echo::Create,        "{EF3E932C-D40B-4F51-8CCF-3F98F1B29D5D}", "Echo",        kDmoMagic, 0xEF3E932C, PluginCategory::DMO, false, false },
		{ DMO::Flanger::Create,     "{EFCA3D92-DFD8-4672-A603-7420894BAD98}", "Flanger",     kDmoMagic, 0xEFCA3D92, PluginCategory::DMO, false, false },
		{ DMO::Gargle::Create,      "{DAFD8210-5711-4B91-9FE3-F75B7AE279BF}", "Gargle",      kDmoMagic, 0xDAFD8210, PluginCategory::DMO, false, false },
		{ DMO::I3DL2Reverb::Create, "{EF985E71-D5C7-42D4-BA4D-2D073E2E96F4}", "I3DL2Reverb", kDmoMagic, 0xEF985E71, PluginCategory::DMO, false, false },
		{ DMO::ParamEq::Create,     "{120CED89-3BF4-4173-A132-3CB406CF3231}", "ParamEq",     kDmoMagic, 0x120CED89, PluginCategory::DMO, false, false },
		{ DMO::WavesReverb::Create, "{87FC0268-9A55-4360-95AA-004A1D9DE26C}", "WavesReverb", kDmoMagic, 0x87FC0268, PluginCategory::DMO, false, false },
		// First (inaccurate) Flanger implementation (will be chosen based on library name, shares ID1 and ID2 with regular Flanger)
		{ DMO::Flanger::CreateLegacy, "{EFCA3D92-DFD8-4672-A603-7420894BAD98}", "Flanger (Legacy)", kDmoMagic, 0xEFCA3D92, PluginCategory::Hidden, false, false },
		// DigiBooster Pro Echo DSP
		{ DigiBoosterEcho::Create, "", "DigiBooster Pro Echo", MagicLE("DBM0"), MagicLE("Echo"), PluginCategory::RoomFx, false, true },
		// LFO
		{ LFOPlugin::Create, "", "LFO", MagicLE("OMPT"), MagicLE("LFO "), PluginCategory::Generator, false, true },
		// SymMOD Echo
		{ SymMODEcho::Create, "", "SymMOD Echo", MagicLE("SymM"), MagicLE("Echo"), PluginCategory::RoomFx, false, true },
#ifdef MODPLUG_TRACKER
		{ MidiInOut::Create, "", "MIDI Input Output", PLUGMAGIC('V','s','t','P'), PLUGMAGIC('M','M','I','D'), PluginCategory::Synth, true, true },
#endif // MODPLUG_TRACKER
	};

	pluginList.reserve(std::size(BuiltInPlugins));
	for(const auto &plugin : BuiltInPlugins)
	{
		auto &plug = pluginList.emplace_back(std::make_unique<VSTPluginLib>(plugin.createProc, true, mpt::PathString::FromUTF8(plugin.filename), mpt::PathString::FromUTF8(plugin.name)));
		plug->pluginId1 = plugin.pluginId1;
		plug->pluginId2 = plugin.pluginId2;
		plug->category = plugin.category;
		plug->isInstrument = plugin.isInstrument;
#ifdef MODPLUG_TRACKER
		if(plugin.isOurs)
			plug->vendor = _T("OpenMPT Project");
#endif // MODPLUG_TRACKER
	}

#ifdef MODPLUG_TRACKER
	// For security reasons, we do not load untrusted DMO plugins in libopenmpt.
	EnumerateDirectXDMOs();
#endif
}


CVstPluginManager::~CVstPluginManager()
{
	for(auto &plug : pluginList)
	{
		while(plug->pPluginsList != nullptr)
		{
			IMixPlugin *pluginInstance = plug->pPluginsList;
			plug->RemovePluginInstanceFromList(*pluginInstance);
			pluginInstance->Release();
		}
	}
#if defined(MPT_WITH_DMO)
	if(MustUnInitilizeCOM)
	{
		CoUninitialize();
		MustUnInitilizeCOM = false;
	}
#endif
}


bool CVstPluginManager::IsValidPlugin(const VSTPluginLib *pLib) const
{
	return std::find_if(pluginList.begin(), pluginList.end(), [pLib](const std::unique_ptr<VSTPluginLib> &value)
	{
		return value.get() == pLib;
	}) != pluginList.end();
}


void CVstPluginManager::EnumerateDirectXDMOs()
{
#if defined(MPT_WITH_DMO)
	static constexpr mpt::UUID knownDMOs[] =
	{
		"745057C7-F353-4F2D-A7EE-58434477730E"_uuid, // AEC (Acoustic echo cancellation, not usable)
		"EFE6629C-81F7-4281-BD91-C9D604A95AF6"_uuid, // Chorus
		"EF011F79-4000-406D-87AF-BFFB3FC39D57"_uuid, // Compressor
		"EF114C90-CD1D-484E-96E5-09CFAF912A21"_uuid, // Distortion
		"EF3E932C-D40B-4F51-8CCF-3F98F1B29D5D"_uuid, // Echo
		"EFCA3D92-DFD8-4672-A603-7420894BAD98"_uuid, // Flanger
		"DAFD8210-5711-4B91-9FE3-F75B7AE279BF"_uuid, // Gargle
		"EF985E71-D5C7-42D4-BA4D-2D073E2E96F4"_uuid, // I3DL2Reverb
		"120CED89-3BF4-4173-A132-3CB406CF3231"_uuid, // ParamEq
		"87FC0268-9A55-4360-95AA-004A1D9DE26C"_uuid, // WavesReverb
		"F447B69E-1884-4A7E-8055-346F74D6EDB3"_uuid, // Resampler DMO (not usable)
		"A8122FF4-9E52-4374-B3D9-B4063E77109D"_uuid, // XnaVisualizerDmo (not usable)
	};

	HKEY hkEnum;
	TCHAR keyname[128];

	LONG cr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("software\\classes\\DirectShow\\MediaObjects\\Categories\\f3602b3f-0592-48df-a4cd-674721e7ebeb"), 0, KEY_READ, &hkEnum);
	DWORD index = 0;
	while (cr == ERROR_SUCCESS)
	{
		if ((cr = RegEnumKey(hkEnum, index, keyname, mpt::saturate_cast<DWORD>(std::size(keyname)))) == ERROR_SUCCESS)
		{
			CLSID clsid;
			mpt::winstring formattedKey = mpt::winstring(_T("{")) + mpt::winstring(keyname) + mpt::winstring(_T("}"));
			if(mpt::VerifyStringToCLSID(formattedKey, clsid))
			{
				if(!mpt::contains(knownDMOs, clsid))
				{
					HKEY hksub;
					formattedKey = mpt::winstring(_T("software\\classes\\DirectShow\\MediaObjects\\")) + mpt::winstring(keyname);
					if (RegOpenKey(HKEY_LOCAL_MACHINE, formattedKey.c_str(), &hksub) == ERROR_SUCCESS)
					{
						TCHAR name[64];
						DWORD datatype = REG_SZ;
						DWORD datasize = sizeof(name);

						if(ERROR_SUCCESS == RegQueryValueEx(hksub, nullptr, 0, &datatype, reinterpret_cast<LPBYTE>(name), &datasize))
						{
							auto &plug = pluginList.emplace_back(std::make_unique<VSTPluginLib>(DMOPlugin::Create, true, mpt::PathString::FromNative(mpt::GUIDToString(clsid)), mpt::PathString::FromNative(ParseMaybeNullTerminatedStringFromBufferWithSizeInBytes<mpt::winstring>(name, datasize))));
							plug->pluginId1 = kDmoMagic;
							plug->pluginId2 = clsid.Data1;
							plug->category = PluginCategory::DMO;
#ifdef DMO_LOG
							MPT_LOG_GLOBAL(LogDebug, "DMO", MPT_UFORMAT("Found \"{}\" clsid={}\n")(plug->libraryName, plug->dllPath));
#endif
						}
						RegCloseKey(hksub);
					}
				}
			}
		}
		index++;
	}
	if (hkEnum) RegCloseKey(hkEnum);
#endif // MPT_WITH_DMO
}


#ifdef MPT_WITH_VST
// Convert CVstPlugin::LoadResult into a collection of VSTPluginLibs.
static std::vector<VSTPluginLib> GetPluginInformation(VSTPluginLib plug, const CVstPlugin::LoadResult &loadResult)
{
	plug.pluginId1 = loadResult.magic;
	plug.pluginId2 = loadResult.uniqueID;
	plug.isInstrument = CVstPlugin::IsInstrument(*loadResult.effect);

	std::vector<VSTPluginLib> containedPlugins;
	if(loadResult.shellPlugins.empty())
	{
#ifdef MODPLUG_TRACKER
		plug.WriteToCache();
#endif  // MODPLUG_TRACKER
		containedPlugins.push_back(std::move(plug));
	} else
	{
		for(auto &shellPlug : loadResult.shellPlugins)
		{
			plug.shellPluginID = shellPlug.shellPluginID;
			plug.libraryName = mpt::PathString::FromLocale(shellPlug.name);
#ifdef MODPLUG_TRACKER
			plug.WriteToCache();
#endif  // MODPLUG_TRACKER
			containedPlugins.push_back(plug);
		}
	}

	return containedPlugins;
}
#endif // !NO_VST


#ifdef MODPLUG_TRACKER
// Add a plugin to the list of known plugins.
std::vector<VSTPluginLib *> CVstPluginManager::AddPlugin(const mpt::PathString &dllPath, bool maskCrashes, bool fromCache, bool *fileFound, uint32 shellPluginID)
{
	const mpt::PathString fileName = dllPath.GetFilenameBase();

	// Check if this is already a known plugin
	for(const auto &dupePlug : pluginList)
	{
		if(shellPluginID == dupePlug->shellPluginID && !mpt::PathCompareNoCase(dllPath, dupePlug->dllPath))
			return {dupePlug.get()};
	}

	if(fileFound != nullptr)
	{
		*fileFound = mpt::native_fs{}.is_file(dllPath);
	}

	// Look if the plugin info is stored in the PluginCache
	if(fromCache)
	{
		mpt::ustring shellStr;
		if(shellPluginID)
			shellStr = UL_("|") + mpt::ufmt::HEX0<8>(shellPluginID);

		SettingsContainer &cacheFile = theApp.GetPluginCache();
		// First try finding the full path
		mpt::ustring IDs = cacheFile.Read<mpt::ustring>(cacheSection, dllPath.ToUnicode() + shellStr, U_(""));
		if(IDs.length() < 16)
		{
			// If that didn't work out, find relative path
			mpt::PathString relPath = theApp.PathAbsoluteToInstallRelative(dllPath);
			IDs = cacheFile.Read<mpt::ustring>(cacheSection, relPath.ToUnicode() + shellStr, U_(""));
		}

		if(IDs.length() >= 16)
		{
			auto &plug = pluginList.emplace_back(std::make_unique<VSTPluginLib>(nullptr, false, dllPath, fileName));

			// Extract plugin IDs
			uint32 id1 = 0, id2 = 0;
			for(int i = 0; i < 16; i++)
			{
				uint32 n = IDs[i] - '0';
				if(n > 9)
					n = IDs[i] + 10 - 'A';
				n &= 0x0F;
				if (i < 8)
					id1 = (id1 << 4) | n;
				else
					id2 = (id2 << 4) | n;
			}
			plug->pluginId1 = id1;
			plug->pluginId2 = id2;

			const mpt::ustring flagKey = IDs + U_(".Flags");
			plug->DecodeCacheFlags(cacheFile.Read<int32>(cacheSection, flagKey, 0));
			plug->vendor = cacheFile.Read<CString>(cacheSection, IDs + U_(".Vendor"), CString());
			plug->shellPluginID = shellPluginID;

#ifdef VST_LOG
			MPT_LOG_GLOBAL(LogDebug, "VST", MPT_UFORMAT("Plugin \"{}\" found in PluginCache")(plug->libraryName));
#endif // VST_LOG
			return {plug.get()};
		} else
		{
#ifdef VST_LOG
			MPT_LOG_GLOBAL(LogDebug, "VST", MPT_UFORMAT("Plugin mismatch in PluginCache: \"{}\" [{}]")(dllPath, IDs));
#endif // VST_LOG
		}
	}

	// If this key contains a file name on program launch, a plugin previously crashed OpenMPT.
	theApp.GetSettings().Write<mpt::PathString>(U_("VST Plugins"), U_("FailedPlugin"), dllPath, SettingWriteThrough);

	std::vector<VSTPluginLib *> foundPlugins;

#ifdef MPT_WITH_VST
	unsigned long exception = 0;
	// Always scan plugins in a separate process
	{
		ExceptionHandler::Context ectx{ MPT_UFORMAT("VST Plugin: {}")(dllPath.ToUnicode()) };
		ExceptionHandler::ContextSetter ectxguard{&ectx};

		VSTPluginLib plug{nullptr, false, dllPath, fileName};
		auto loadResult = CVstPlugin::LoadPlugin(maskCrashes, plug, CVstPlugin::BridgeMode::DetectRequiredBridgeMode, exception);
		Vst::AEffect *pEffect = loadResult.effect;
		if(pEffect)
		{
			foundPlugins = AddPluginsToList(GetPluginInformation(std::move(plug), loadResult),
				[&](VSTPluginLib &library, bool updateExisting)
				{
					if(updateExisting)
						return;
					if(library.shellPluginID)
					{
						if(!CVstPlugin::SelectShellPlugin(maskCrashes, loadResult, library))
							return;
					}
					CVstPlugin::GetPluginMetadata(maskCrashes, loadResult, library);
				});
			CVstPlugin::DispatchSEH(maskCrashes, *pEffect, Vst::effClose, 0, 0, 0, 0, exception);
		}
		if(loadResult.library)
		{
			FreeLibrary(loadResult.library);
		}
	}
	if(exception != 0)
	{
		CVstPluginManager::ReportPlugException(MPT_UFORMAT("Exception {} while trying to load plugin \"{}\"!\n")(mpt::ufmt::HEX0<8>(exception), fileName));
	}

#endif // MPT_WITH_VST

	// Now it should be safe to assume that this plugin loaded properly. :)
	theApp.GetSettings().Remove(U_("VST Plugins"), U_("FailedPlugin"));

	return foundPlugins;
}


// Remove a plugin from the list of known plugins and release any remaining instances of it.
bool CVstPluginManager::RemovePlugin(VSTPluginLib *pFactory)
{
	for(const_iterator p = begin(); p != end(); p++)
	{
		VSTPluginLib *plug = p->get();
		if(plug == pFactory)
		{
			// Kill all instances of this plugin
			CriticalSection cs;

			while(plug->pPluginsList != nullptr)
			{
				IMixPlugin *pluginInstance = plug->pPluginsList;
				plug->RemovePluginInstanceFromList(*pluginInstance);
				pluginInstance->Release();
			}
			pluginList.erase(p);
			return true;
		}
	}
	return false;
}
#endif // MODPLUG_TRACKER


// Create an instance of a plugin.
bool CVstPluginManager::CreateMixPlugin(SNDMIXPLUGIN &mixPlugin, CSoundFile &sndFile)
{
	VSTPluginLib *pFound = nullptr;

	// Find plugin in library
	enum PlugMatchQuality
	{
		kNoMatch,
		kMatchName,
		kMatchId,
		kMatchNameAndId,
	};

	PlugMatchQuality match = kNoMatch;  // "Match quality" of found plugin. Higher value = better match.
#if defined(MODPLUG_TRACKER) && MPT_OS_WINDOWS && !MPT_OS_WINDOWS_WINRT
	const mpt::PathString libraryName = mpt::PathString::FromUnicode(mixPlugin.GetLibraryName());
#else
	const std::string libraryName = mpt::ToCharset(mpt::Charset::UTF8, mixPlugin.GetLibraryName());
#endif
	for(const auto &plug : pluginList)
	{
		const bool matchID = (plug->pluginId1 == mixPlugin.Info.dwPluginId1)
			&& (plug->pluginId2 == mixPlugin.Info.dwPluginId2)
			&& (plug->shellPluginID == mixPlugin.Info.shellPluginID);
#if defined(MODPLUG_TRACKER) && MPT_OS_WINDOWS && !MPT_OS_WINDOWS_WINRT
		const bool matchName = !mpt::PathCompareNoCase(plug->libraryName, libraryName);
#else
		const bool matchName = !mpt::CompareNoCaseAscii(plug->libraryName.ToUTF8(), libraryName);
#endif

		if(matchID && matchName)
		{
			pFound = plug.get();
#ifdef MPT_WITH_VST
			if(plug->IsNative(false))
			{
				break;
			}
#endif // MPT_WITH_VST
			// If the plugin isn't native, first check if a native version can be found.
			match = kMatchNameAndId;
		} else if(matchID && match < kMatchId)
		{
			pFound = plug.get();
			match = kMatchId;
		} else if(matchName && match < kMatchName)
		{
			pFound = plug.get();
			match = kMatchName;
		}
	}

	if(!pFound)
		return false;

	IMixPlugin *plugin = nullptr;
	if(pFound->Create != nullptr)
	{
		plugin = pFound->Create(*pFound, sndFile, mixPlugin);
	}
#ifdef MPT_WITH_VST
	// Note: we don't check if dwPluginId1 matches Vst::kEffectMagic here, even if it should.
	// I have an old file I made with OpenMPT 1.17 where the primary plugin ID has an unexpected value.
	// No idea how that could happen, apart from some plugin.cache corruption (back then, the IDs were not re-checked
	// after instantiating a plugin and the cached plugin ID was blindly written to the module file)
	else
	{
		bool maskCrashes = TrackerSettings::Instance().BrokenPluginsWorkaroundVSTMaskAllCrashes;

		unsigned long exception = 0;
		auto loadResult = CVstPlugin::LoadPlugin(maskCrashes, *pFound, TrackerSettings::Instance().bridgeAllPlugins ? CVstPlugin::BridgeMode::ForceBridgeWithFallback : CVstPlugin::BridgeMode::Automatic, exception);
		Vst::AEffect *pEffect = loadResult.effect;

		if(pEffect != nullptr)
		{
			// If filename matched during load but plugin ID didn't (or vice versa), make sure it's updated.
			pFound->pluginId1 = loadResult.magic;
			pFound->pluginId2 = loadResult.uniqueID;

			plugin = new (std::nothrow) CVstPlugin(maskCrashes, loadResult.library, *pFound, mixPlugin, *pEffect, sndFile);

#ifdef MODPLUG_TRACKER
			AddPluginsToList(GetPluginInformation(*pFound, loadResult),
				[&](VSTPluginLib &library, bool updateExisting)
				{
					if(&library == pFound && updateExisting)
						CVstPlugin::GetPluginMetadata(maskCrashes, loadResult, library);
				});
#endif
		}

		if(!plugin && loadResult.library)
		{
			FreeLibrary(loadResult.library);
			CVstPluginManager::ReportPlugException(MPT_UFORMAT("Unable to create plugin \"{}\"!\n")(pFound->libraryName));
		}
	}
#endif // MPT_WITH_VST

#ifdef MODPLUG_TRACKER
	CriticalSection cs;
#endif
	if(plugin)
		pFound->InsertPluginInstanceIntoList(*plugin);
	mixPlugin.pMixPlugin = plugin;
	// If filename matched during load but plugin ID didn't (or vice versa), make sure it's updated.
	mixPlugin.Info.dwPluginId1 = pFound->pluginId1;
	mixPlugin.Info.dwPluginId2 = pFound->pluginId2;
	mixPlugin.Info.szLibraryName = pFound->libraryName.ToUTF8();

	return plugin != nullptr;
}


#ifdef MPT_WITH_VST
std::vector<VSTPluginLib *> CVstPluginManager::AddPluginsToList(std::vector<VSTPluginLib> containedPlugins, std::function<void(VSTPluginLib &, bool)> updateFunc)
{
	std::vector<VSTPluginLib *> newPlugins;
	if(containedPlugins.empty())
		return newPlugins;

	// Find existing shell plugins belonging to the same file first, so that we don't have to iterate through the whole plugin list again and again
	std::map<uint32, size_t> existingCandidates;
	for(size_t i = 0; i < pluginList.size(); i++)
	{
		const auto &plug = pluginList[i];
		if(plug->pluginId1 == containedPlugins.front().pluginId1 && plug->pluginId2 == containedPlugins.front().pluginId2)
		{
			if(!mpt::PathCompareNoCase(plug->dllPath, containedPlugins.front().dllPath))
				existingCandidates[plug->shellPluginID] = i;
		}
	}

	// Add found plugins to list or update them if they already exist
	std::set<uint32> containedIDs;
	VSTPluginLib *first = nullptr;
	for(auto &containedPlug : containedPlugins)
	{
		containedIDs.insert(containedPlug.shellPluginID);
		VSTPluginLib *found = nullptr;
		if(auto it = existingCandidates.find(containedPlug.shellPluginID); it != existingCandidates.end())
		{
			auto &plug = pluginList[it->second];
			MPT_ASSERT(plug->pluginId1 == containedPlug.pluginId1 && plug->pluginId2 == containedPlug.pluginId2);
			if(plug->shellPluginID == containedPlug.shellPluginID)
				found = plug.get();
		}
		const bool updateExisting = found != nullptr;
		if(updateExisting)
		{
			found->libraryName = containedPlug.libraryName;
		} else
		{
			auto &plug = pluginList.emplace_back(std::make_unique<VSTPluginLib>(std::move(containedPlug)));
			found = plug.get();
			newPlugins.push_back(found);
		}
		updateFunc(*found, updateExisting);

		if(!first)
			first = found;
			
#ifdef MODPLUG_TRACKER
		found->WriteToCache();
#endif // MODPLUG_TRACKER
	}

	// Are there any shell plugins in our list that are no longer part of the shell plugin?
	if(containedPlugins.size() > 1)
	{
		size_t deleted = 0;
		for(const auto &[id, i] : existingCandidates)
		{
			if(!mpt::contains(containedIDs, id) && !pluginList[i - deleted]->pPluginsList)
			{
				MPT_ASSERT(pluginList[i - deleted]->shellPluginID == id);
				pluginList.erase(pluginList.begin() + i - deleted);
				deleted++;
			}
		}
	}

	if(newPlugins.empty() && first)
		newPlugins.push_back(first);
	return newPlugins;
}
#endif // MPT_WITH_VST


#ifdef MODPLUG_TRACKER
void CVstPluginManager::OnIdle()
{
	for(auto &factory : pluginList)
	{
		// Note: bridged plugins won't receive these messages and generate their own idle messages.
		IMixPlugin *p = factory->pPluginsList;
		while (p)
		{
			//rewbs. VSTCompliance: A specific plug has requested indefinite periodic processing time.
			p->Idle();
			//We need to update all open editors
			CAbstractVstEditor *editor = p->GetEditor();
			if (editor && editor->m_hWnd)
			{
				editor->UpdateParamDisplays();
			}
			//end rewbs. VSTCompliance:

			p = p->GetNextInstance();
		}
	}
}


void CVstPluginManager::ReportPlugException(const mpt::ustring &msg)
{
	Reporting::Notification(msg);
#ifdef VST_LOG
	MPT_LOG_GLOBAL(LogDebug, "VST", mpt::ToUnicode(msg));
#endif
}

#endif // MODPLUG_TRACKER

OPENMPT_NAMESPACE_END

#endif // NO_PLUGINS
