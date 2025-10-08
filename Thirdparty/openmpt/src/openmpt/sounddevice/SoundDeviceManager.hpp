/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "SoundDevice.hpp"

#include "mpt/base/detect.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/logging/Logger.hpp"

#include <map>
#include <memory>
#include <vector>

#include <cstddef>


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice
{


struct EnabledBackends
{
#if defined(MPT_WITH_PULSEAUDIO) && defined(MPT_ENABLE_PULSEAUDIO_FULL)
	bool Pulseaudio = true;
#endif  // MPT_WITH_PULSEAUDIO && MPT_ENABLE_PULSEAUDIO_FULL
#if defined(MPT_WITH_PULSEAUDIO) && defined(MPT_WITH_PULSEAUDIOSIMPLE)
	bool PulseaudioSimple = true;
#endif  // MPT_WITH_PULSEAUDIO && MPT_WITH_PULSEAUDIOSIMPLE
#if MPT_OS_WINDOWS
	bool WaveOut = true;
#endif  // MPT_OS_WINDOWS
#if defined(MPT_WITH_DIRECTSOUND)
	bool DirectSound = true;
#endif  // MPT_WITH_DIRECTSOUND
#ifdef MPT_WITH_ASIO
	bool ASIO = true;
#endif  // MPT_WITH_ASIO
#ifdef MPT_WITH_PORTAUDIO
	bool PortAudio = true;
#endif  // MPT_WITH_PORTAUDIO
#ifdef MPT_WITH_RTAUDIO
	bool RtAudio = true;
#endif  // MPT_WITH_RTAUDIO
};


class IDevicesEnumerator
{
protected:
	typedef SoundDevice::IBase *(*CreateSoundDeviceFunc)(ILogger &logger, const SoundDevice::Info &info, SoundDevice::SysInfo sysInfo);

protected:
	IDevicesEnumerator() = default;

public:
	virtual ~IDevicesEnumerator() = default;

public:
	virtual std::unique_ptr<SoundDevice::BackendInitializer> BackendInitializer() = 0;
	virtual std::vector<SoundDevice::Info> EnumerateDevices(ILogger &logger, SoundDevice::SysInfo sysInfo) = 0;
	virtual CreateSoundDeviceFunc GetCreateFunc() = 0;
};


template <typename TSoundDevice>
class DevicesEnumerator
	: public IDevicesEnumerator
{
public:
	DevicesEnumerator() = default;
	~DevicesEnumerator() override = default;

public:
	std::unique_ptr<SoundDevice::BackendInitializer> BackendInitializer() override
	{
		return TSoundDevice::BackendInitializer();
	}
	std::vector<SoundDevice::Info> EnumerateDevices(ILogger &logger, SoundDevice::SysInfo sysInfo) override
	{
		return TSoundDevice::EnumerateDevices(logger, sysInfo);
	}
	virtual CreateSoundDeviceFunc GetCreateFunc() override
	{
		return &ConstructSoundDevice;
	}

public:
	static SoundDevice::IBase *ConstructSoundDevice(ILogger &logger, const SoundDevice::Info &info, SoundDevice::SysInfo sysInfo)
	{
		return new TSoundDevice(logger, info, sysInfo);
	}
};


class Manager
{

public:
	typedef std::size_t GlobalID;

protected:
	typedef SoundDevice::IBase *(*CreateSoundDeviceFunc)(ILogger &logger, const SoundDevice::Info &info, SoundDevice::SysInfo sysInfo);

protected:
	ILogger &m_Logger;
	const SoundDevice::SysInfo m_SysInfo;
	const SoundDevice::AppInfo m_AppInfo;

	std::vector<std::shared_ptr<IDevicesEnumerator>> m_DeviceEnumerators;

	std::vector<std::unique_ptr<BackendInitializer>> m_BackendInitializers;

	std::vector<SoundDevice::Info> m_SoundDevices;
	std::map<SoundDevice::Identifier, bool> m_DeviceUnavailable;
	std::map<SoundDevice::Identifier, CreateSoundDeviceFunc> m_DeviceFactoryMethods;
	std::map<SoundDevice::Identifier, SoundDevice::Caps> m_DeviceCaps;
	std::map<SoundDevice::Identifier, SoundDevice::DynamicCaps> m_DeviceDynamicCaps;

public:
	Manager(ILogger &logger, SoundDevice::SysInfo sysInfo, SoundDevice::AppInfo appInfo, std::vector<std::shared_ptr<IDevicesEnumerator>> deviceEnumerators = GetDefaultEnumerators());
	~Manager();

public:
	ILogger &GetLogger() const { return m_Logger; }
	SoundDevice::SysInfo GetSysInfo() const { return m_SysInfo; }
	SoundDevice::AppInfo GetAppInfo() const { return m_AppInfo; }

	static std::vector<std::shared_ptr<IDevicesEnumerator>> GetDefaultEnumerators();
	static std::vector<std::shared_ptr<IDevicesEnumerator>> GetEnabledEnumerators(EnabledBackends enabledBackends);

	void ReEnumerate(bool firstRun = false);

	std::vector<SoundDevice::Info>::const_iterator begin() const { return m_SoundDevices.begin(); }
	std::vector<SoundDevice::Info>::const_iterator end() const { return m_SoundDevices.end(); }
	const std::vector<SoundDevice::Info> &GetDeviceInfos() const { return m_SoundDevices; }

	SoundDevice::Manager::GlobalID GetGlobalID(SoundDevice::Identifier identifier) const;

	SoundDevice::Info FindDeviceInfo(SoundDevice::Manager::GlobalID id) const;
	SoundDevice::Info FindDeviceInfo(SoundDevice::Identifier identifier) const;
	SoundDevice::Info FindDeviceInfoBestMatch(SoundDevice::Identifier identifier);

	bool OpenDriverSettings(SoundDevice::Identifier identifier, SoundDevice::IMessageReceiver *messageReceiver = nullptr, SoundDevice::IBase *currentSoundDevice = nullptr);

	void SetDeviceUnavailable(SoundDevice::Identifier identifier) { m_DeviceUnavailable[identifier] = true; }
	bool IsDeviceUnavailable(SoundDevice::Identifier identifier) { return m_DeviceUnavailable[identifier]; }

	SoundDevice::Caps GetDeviceCaps(SoundDevice::Identifier identifier, SoundDevice::IBase *currentSoundDevice = nullptr);
	SoundDevice::DynamicCaps GetDeviceDynamicCaps(SoundDevice::Identifier identifier, const std::vector<uint32> &baseSampleRates, SoundDevice::IMessageReceiver *messageReceiver = nullptr, SoundDevice::IBase *currentSoundDevice = nullptr, bool update = false);

	SoundDevice::IBase *CreateSoundDevice(SoundDevice::Identifier identifier);
};


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
