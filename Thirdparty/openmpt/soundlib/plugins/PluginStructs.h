/*
 * PluginStructs.h
 * ---------------
 * Purpose: Basic plugin structs for CSoundFile.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "../Snd_defs.h"
#ifndef NO_PLUGINS
#include "openmpt/base/Endian.hpp"
#endif // NO_PLUGINS

OPENMPT_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////
// Mix Plugins

struct SNDMIXPLUGINSTATE;
struct SNDMIXPLUGIN;
class IMixPlugin;
class CSoundFile;

#ifndef NO_PLUGINS

enum class PluginMixMode : uint8
{
	Default        = 0,
	WetSubtract    = 1,
	DrySubtract    = 2,
	MixSubtract    = 3,
	MiddleSubtract = 4,
	LRBalance      = 5,
	Instrument     = 6,
};

struct SNDMIXPLUGININFO
{
	// dwInputRouting flags
	enum RoutingFlags
	{
		irApplyToMaster = 0x01,  // Apply to master mix
		irBypass        = 0x02,  // Bypass effect
		irDryMix        = 0x04,  // Wet Mix (dry added)
		irExpandMix     = 0x08,  // [0%,100%] -> [-200%,200%]
		irAutoSuspend   = 0x10,  // Plugin will automatically suspend on silence
	};

	int32le dwPluginId1;   // Plugin type (kEffectMagic, kDmoMagic or custom for built-in plugins)
	int32le dwPluginId2;   // Plugin unique ID
	uint8le routingFlags;  // See RoutingFlags
	uint8le mixMode;
	uint8le gain;  // Divide by 10 to get real gain
	uint8le reserved;
	uint32le dwOutputRouting;                                         // 0 = send to master 0x80 + x = send to plugin x
	uint32le shellPluginID;                                           // For shell plugins: The child plugin to load
	uint32le dwReserved[3];                                           // Reserved for routing info
	mpt::modecharbuf<32, mpt::String::nullTerminated> szName;         // User-chosen plugin display name - this is locale ANSI!
	mpt::modecharbuf<64, mpt::String::nullTerminated> szLibraryName;  // original DLL name (shell plugins: child plugin name) - this is UTF-8!

	// Should only be called from SNDMIXPLUGIN::SetBypass() and IMixPlugin::Bypass()
	void SetBypass(bool bypass = true) { if(bypass) routingFlags |= irBypass; else routingFlags &= uint8(~irBypass); }
};

MPT_BINARY_STRUCT(SNDMIXPLUGININFO, 128)	// this is directly written to files, so the size must be correct!


struct SNDMIXPLUGIN
{
	IMixPlugin *pMixPlugin = nullptr;
	std::vector<std::byte> pluginData;
	SNDMIXPLUGININFO Info = {};
	float fDryRatio = 0;
	int32 defaultProgram = 0;
	int32 editorX = 0, editorY = 0;

#if defined(MPT_ENABLE_CHARSET_LOCALE)
	const char * GetNameLocale() const
	{
		return Info.szName.buf;
	}
	mpt::ustring GetName() const
	{
		return mpt::ToUnicode(mpt::Charset::Locale, Info.szName);
	}
#endif // MPT_ENABLE_CHARSET_LOCALE
	mpt::ustring GetLibraryName() const
	{
		return mpt::ToUnicode(mpt::Charset::UTF8, Info.szLibraryName);
	}

	// Check if a plugin is loaded into this slot (also returns true if the plugin in this slot has not been found)
	bool IsValidPlugin() const { return (Info.dwPluginId1 | Info.dwPluginId2) != 0; }

	// Input routing getters
	uint8 GetGain() const
		{ return Info.gain; }
	PluginMixMode GetMixMode() const
		{ return static_cast<PluginMixMode>(Info.mixMode.get()); }
	bool IsMasterEffect() const
		{ return (Info.routingFlags & SNDMIXPLUGININFO::irApplyToMaster) != 0; }
	bool IsDryMix() const
		{ return (Info.routingFlags & SNDMIXPLUGININFO::irDryMix) != 0; }
	bool IsExpandedMix() const
		{ return (Info.routingFlags & SNDMIXPLUGININFO::irExpandMix) != 0; }
	bool IsBypassed() const
		{ return (Info.routingFlags & SNDMIXPLUGININFO::irBypass) != 0; }
	bool IsAutoSuspendable() const
		{ return (Info.routingFlags & SNDMIXPLUGININFO::irAutoSuspend) != 0; }

	// Input routing setters
	void SetGain(uint8 gain);
	void SetMixMode(PluginMixMode mixMode)
		{ Info.mixMode = static_cast<uint8>(mixMode); }
	void SetMasterEffect(bool master = true)
		{ if(master) Info.routingFlags |= SNDMIXPLUGININFO::irApplyToMaster; else Info.routingFlags &= uint8(~SNDMIXPLUGININFO::irApplyToMaster); }
	void SetDryMix(bool wetMix = true)
		{ if(wetMix) Info.routingFlags |= SNDMIXPLUGININFO::irDryMix; else Info.routingFlags &= uint8(~SNDMIXPLUGININFO::irDryMix); }
	void SetExpandedMix(bool expanded = true)
		{ if(expanded) Info.routingFlags |= SNDMIXPLUGININFO::irExpandMix; else Info.routingFlags &= uint8(~SNDMIXPLUGININFO::irExpandMix); }
	void SetBypass(bool bypass = true);
	void SetAutoSuspend(bool suspend = true)
		{ if(suspend) Info.routingFlags |= SNDMIXPLUGININFO::irAutoSuspend; else Info.routingFlags &= uint8(~SNDMIXPLUGININFO::irAutoSuspend); }

	// Output routing getters
	bool IsOutputToMaster() const
		{ return Info.dwOutputRouting == 0; }
	PLUGINDEX GetOutputPlugin() const
		{ return Info.dwOutputRouting >= 0x80 ? static_cast<PLUGINDEX>(Info.dwOutputRouting - 0x80) : PLUGINDEX_INVALID; }

	// Output routing setters
	void SetOutputToMaster()
		{ Info.dwOutputRouting = 0; }
	void SetOutputPlugin(PLUGINDEX plugin)
		{ if(plugin < MAX_MIXPLUGINS) Info.dwOutputRouting = plugin + 0x80; else Info.dwOutputRouting = 0; }

	void Destroy();
};

bool CreateMixPluginProc(SNDMIXPLUGIN &mixPlugin, CSoundFile &sndFile);

#endif // NO_PLUGINS

OPENMPT_NAMESPACE_END
