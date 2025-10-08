
#ifndef ASIO_ASIOSYSTEMWINDOWS_HPP
#define ASIO_ASIOSYSTEMWINDOWS_HPP



#include "ASIOVersion.hpp"
#include "ASIOConfig.hpp"
#include "ASIOCore.hpp"

#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <cassert>

#if ASIO_SYSTEM_WINDOWS
#include <windows.h>
#if !defined(NTDDI_VERSION)
#error "NTDDI_VERSION undefined"
#endif
#if !defined(_WIN32_WINNT)
#error "_WIN32_WINNT undefined"
#endif
#include <avrt.h>
#endif // ASIO_SYSTEM_WINDOWS



#if ASIO_SYSTEM_WINDOWS
#if ASIO_HAVE_PRAGMA_COMMENT_LIB
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "avrt.lib")
#endif // ASIO_HAVE_PRAGMA_COMMENT_LIB
#endif // ASIO_SYSTEM_WINDOWS



namespace ASIO {



inline namespace ASIO_VERSION_NAMESPACE {



#if ASIO_SYSTEM_WINDOWS



namespace Windows {



static_assert(NTDDI_VERSION >= NTDDI_WIN7);
static_assert(_WIN32_WINNT >= _WIN32_WINNT_WIN7);



#if defined(UNICODE)
inline namespace Unicode {
#else
inline namespace Ansi {
#endif



struct DriverInfo {
	std::basic_string<TCHAR> Key;
	std::basic_string<TCHAR> Id;
	CLSID                    Clsid{};
	std::basic_string<TCHAR> Name;
	std::basic_string<TCHAR> Description;
	std::basic_string<TCHAR> DisplayName() const {
		if (Description.empty()) {
			return Key;
		}
		return Description;
	}
};


class HKey {
private:
	HKEY m_Key = NULL;

public:
	HKey()                         = default;
	HKey(const HKey &)             = delete;
	HKey & operator=(const HKey &) = delete;
	~HKey() {
		if (m_Key) {
			RegCloseKey(m_Key);
		}
	}
	operator HKEY &() {
		return m_Key;
	}
	operator HKEY *() {
		return &m_Key;
	}
};


inline LRESULT CheckLRESULTOutOfMemory(LRESULT lr) {
	if ((lr == ERROR_NOT_ENOUGH_MEMORY) || (lr == ERROR_OUTOFMEMORY)) {
		throw std::bad_alloc();
	}
	return lr;
}

inline HRESULT CheckHRESULTOutOfMemory(HRESULT hr) {
	if (hr == E_OUTOFMEMORY) {
		throw std::bad_alloc();
	}
	return hr;
}


inline std::vector<DriverInfo> EnumerateDrivers() {
	std::vector<DriverInfo> drivers;
	HKey                    hkAsioEnum;
	if (CheckLRESULTOutOfMemory(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\ASIO"), 0, KEY_READ, hkAsioEnum)) != ERROR_SUCCESS) {
		return drivers;
	}
	DWORD numSubKeys   = 0;
	DWORD maxSubKeyLen = 0;
	if (CheckLRESULTOutOfMemory(RegQueryInfoKey(hkAsioEnum, NULL, NULL, NULL, &numSubKeys, &maxSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL)) != ERROR_SUCCESS) {
		return drivers;
	}
	for (DWORD i = 0; i < numSubKeys; ++i) {
		std::vector<TCHAR> bufKey(static_cast<std::size_t>(maxSubKeyLen) + 1);
		DWORD              lenKey = static_cast<DWORD>(bufKey.size());
		if (CheckLRESULTOutOfMemory(RegEnumKeyEx(hkAsioEnum, i, bufKey.data(), &lenKey, NULL, NULL, NULL, NULL)) != ERROR_SUCCESS) {
			continue;
		}
		std::basic_string<TCHAR> key(bufKey.data(), bufKey.data() + lenKey);
		HKey                     hkDriver;
		if (CheckLRESULTOutOfMemory(RegOpenKeyEx(hkAsioEnum, key.c_str(), 0, KEY_READ, hkDriver)) != ERROR_SUCCESS) {
			continue;
		}
		DWORD maxValueLen = 0;
		if (CheckLRESULTOutOfMemory(RegQueryInfoKey(hkDriver, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &maxValueLen, NULL, NULL)) != ERROR_SUCCESS) {
			continue;
		}
		std::vector<TCHAR> bufClsid(static_cast<std::size_t>(maxValueLen) + 1);
		DWORD              lenClsid  = static_cast<DWORD>(bufClsid.size()) * sizeof(TCHAR);
		DWORD              typeClsid = REG_SZ;
		if (CheckLRESULTOutOfMemory(RegQueryValueEx(hkDriver, TEXT("CLSID"), NULL, &typeClsid, reinterpret_cast<LPBYTE>(bufClsid.data()), &lenClsid)) != ERROR_SUCCESS) {
			continue;
		}
		std::basic_string<TCHAR> strClsid = std::basic_string<TCHAR>(bufClsid.data(), bufClsid.data() + (lenClsid / sizeof(TCHAR))).c_str();
		std::vector<OLECHAR>     oleClsid(strClsid.c_str(), strClsid.c_str() + strClsid.length() + 1);
		CLSID                    clsid = CLSID();
		if (CheckHRESULTOutOfMemory(CLSIDFromString(oleClsid.data(), &clsid)) != NOERROR) {
			continue;
		}
		std::vector<TCHAR>       bufName(static_cast<std::size_t>(maxValueLen) + 1);
		DWORD                    lenName  = static_cast<DWORD>(bufName.size()) * sizeof(TCHAR);
		DWORD                    typeName = REG_SZ;
		std::basic_string<TCHAR> name;
		if (CheckLRESULTOutOfMemory(RegQueryValueEx(hkDriver, TEXT(""), NULL, &typeName, reinterpret_cast<LPBYTE>(bufName.data()), &lenName)) == ERROR_SUCCESS) {
			name = std::basic_string<TCHAR>(bufName.data(), bufName.data() + (lenName / sizeof(TCHAR))).c_str();
		}
		std::vector<TCHAR>       bufDesc(static_cast<std::size_t>(maxValueLen) + 1);
		DWORD                    lenDesc  = static_cast<DWORD>(bufDesc.size()) * sizeof(TCHAR);
		DWORD                    typeDesc = REG_SZ;
		std::basic_string<TCHAR> desc;
		if (CheckLRESULTOutOfMemory(RegQueryValueEx(hkDriver, TEXT("Description"), NULL, &typeDesc, reinterpret_cast<LPBYTE>(bufDesc.data()), &lenDesc)) == ERROR_SUCCESS) {
			desc = std::basic_string<TCHAR>(bufDesc.data(), bufDesc.data() + (lenDesc / sizeof(TCHAR))).c_str();
		}
		DriverInfo info;
		info.Key         = key;
		info.Id          = strClsid;
		info.Clsid       = clsid;
		info.Name        = name;
		info.Description = desc;
		drivers.push_back(std::move(info));
	}
	return drivers;
}


[[nodiscard]] inline ISystemDriver * OpenDriver(CLSID clsid) {
	ISystemDriver * driver = nullptr;
	if (CheckHRESULTOutOfMemory(CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, reinterpret_cast<void **>(&driver))) != S_OK) {
		return nullptr;
	}
	return driver;
}

inline ULONG CloseDriver(ISystemDriver * driver) {
	return driver->Release();
}


struct DriverLoadFailed
	: public std::runtime_error {
	DriverLoadFailed()
		: std::runtime_error("ASIO Driver load failed.") {
		return;
	}
};


struct DriverInitFailed
	: public std::runtime_error {
	DriverInitFailed()
		: std::runtime_error("ASIO Driver init failed.") {
		return;
	}
};


class Driver
	: public IDriver {

private:
	ISystemDriver * m_Driver = nullptr;

public:
	explicit Driver(CLSID clsid, HWND wnd) {
		m_Driver = openDriver(clsid);
		if (!m_Driver) {
			throw DriverLoadFailed();
		}
		if (!initDriver(wnd)) {
			closeDriver(m_Driver);
			throw DriverInitFailed();
		}
	}

	Driver(const Driver &) = delete;

	Driver & operator=(const Driver &) = delete;

	~Driver() override {
		closeDriver(m_Driver);
	}

private:
	ISystemDriver * openDriver(CLSID clsid) {
		return OpenDriver(clsid);
	}

	ULONG closeDriver(ISystemDriver * driver) {
		return driver->Release();
	}

private:
	Bool initDriver(HWND sysHandle) {
		return static_cast<Bool>(m_Driver->init(reinterpret_cast<SysHandle>(sysHandle)));
	}

public:
	void getDriverName(DriverName * name) final {
		return m_Driver->getDriverName(name);
	}
	[[nodiscard]] Long getDriverVersion() final {
		return m_Driver->getDriverVersion();
	}
	void getErrorMessage(ErrorMessage * string) final {
		return m_Driver->getErrorMessage(string);
	}
	[[nodiscard]] ErrorCode start() final {
		return m_Driver->start();
	}
	[[nodiscard]] ErrorCode stop() final {
		return m_Driver->stop();
	}
	[[nodiscard]] ErrorCode getChannels(Long * numInputChannels, Long * numOutputChannels) final {
		return m_Driver->getChannels(numInputChannels, numOutputChannels);
	}
	[[nodiscard]] ErrorCode getLatencies(Long * inputLatency, Long * outputLatency) final {
		return m_Driver->getLatencies(inputLatency, outputLatency);
	}
	[[nodiscard]] ErrorCode getBufferSize(Long * minSize, Long * maxSize, Long * preferredSize, Long * granularity) final {
		return m_Driver->getBufferSize(minSize, maxSize, preferredSize, granularity);
	}
	[[nodiscard]] ErrorCode canSampleRate(SampleRate sampleRate) final {
		return m_Driver->canSampleRate(sampleRate);
	}
	[[nodiscard]] ErrorCode getSampleRate(SampleRate * sampleRate) final {
		return m_Driver->getSampleRate(sampleRate);
	}
	[[nodiscard]] ErrorCode setSampleRate(SampleRate sampleRate) final {
		return m_Driver->setSampleRate(sampleRate);
	}
	[[nodiscard]] ErrorCode getClockSources(ClockSource * clocks, Long * numSources) final {
		return m_Driver->getClockSources(clocks, numSources);
	}
	[[nodiscard]] ErrorCode setClockSource(Long reference) final {
		return m_Driver->setClockSource(reference);
	}
	[[nodiscard]] ErrorCode getSamplePosition(HiLoLongLong * samplePosition, HiLoLongLong * timeStamp) final {
		return m_Driver->getSamplePosition(samplePosition, timeStamp);
	}
	[[nodiscard]] ErrorCode getChannelInfo(ChannelInfo * info) final {
		return m_Driver->getChannelInfo(info);
	}
	[[nodiscard]] ErrorCode createBuffers(BufferInfo * bufferInfos, Long numChannels, Long bufferSize, const Callbacks * callbacks) final {
		return m_Driver->createBuffers(bufferInfos, numChannels, bufferSize, callbacks);
	}
	[[nodiscard]] ErrorCode disposeBuffers() final {
		return m_Driver->disposeBuffers();
	}
	[[nodiscard]] ErrorCode controlPanel() final {
		return m_Driver->controlPanel();
	}
	[[nodiscard]] ErrorCode future(FutureSelector selector, void * opt) final {
		return m_Driver->future(selector, opt);
	}
	[[nodiscard]] ErrorCode outputReady() final {
		return m_Driver->outputReady();
	}
};



class IBufferSwitchDispatcher {
public:
	virtual ~IBufferSwitchDispatcher() = default;

public:
	virtual void Dispatch(std::size_t bufferIndex) = 0;
};

class BufferSwitchDispatcherBase
	: public IBufferSwitchDispatcher {

private:
	HANDLE m_hBufferSwitch[2] = {NULL, NULL};

	HANDLE m_hStarted     = NULL;
	HANDLE m_hStopRequest = NULL;

	HANDLE m_hThread = NULL;

public:
	BufferSwitchDispatcherBase() {
		m_hBufferSwitch[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hBufferSwitch[0] == NULL) {
			goto error;
		}
		m_hBufferSwitch[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hBufferSwitch[1] == NULL) {
			goto error;
		}
		m_hStarted = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (m_hStarted == NULL) {
			goto error;
		}
		m_hStopRequest = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (m_hStopRequest == NULL) {
			goto error;
		}
		m_hThread = CreateThread(NULL, 0, &ThreadProc, this, 0, NULL);
		if (m_hThread == NULL) {
			goto error;
		}
		if (WaitForSingleObject(m_hStarted, INFINITE) != WAIT_OBJECT_0) {
			if (SetEvent(m_hStopRequest) != TRUE) {
				goto error;
			}
			if (WaitForSingleObject(m_hThread, INFINITE) != WAIT_OBJECT_0) {
				goto error;
			}
			goto error;
		}
		return;
error:
		if (m_hThread != NULL) {
			CloseHandle(m_hThread);
		}
		if (m_hStopRequest != NULL) {
			CloseHandle(m_hStopRequest);
		}
		if (m_hStarted != NULL) {
			CloseHandle(m_hStarted);
		}
		if (m_hBufferSwitch[1] != NULL) {
			CloseHandle(m_hBufferSwitch[1]);
		}
		if (m_hBufferSwitch[0] != NULL) {
			CloseHandle(m_hBufferSwitch[0]);
		}
		throw std::bad_alloc();
	}

	~BufferSwitchDispatcherBase() override {
		SetEvent(m_hStopRequest);
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
		CloseHandle(m_hStopRequest);
		CloseHandle(m_hStarted);
		CloseHandle(m_hBufferSwitch[1]);
		CloseHandle(m_hBufferSwitch[0]);
	}

	BufferSwitchDispatcherBase(const BufferSwitchDispatcherBase &) noexcept             = delete;
	BufferSwitchDispatcherBase & operator=(const BufferSwitchDispatcherBase &) noexcept = delete;

private:
	static DWORD WINAPI ThreadProc(LPVOID lpParameter) noexcept {
		if (!lpParameter) {
			return 1;
		}
		return static_cast<BufferSwitchDispatcherBase *>(lpParameter)->ThreadMain() ? 0 : 1;
	}

	[[nodiscard]] bool ThreadMain() noexcept {
		DWORD  task_idx = 0;
		HANDLE hTask    = AvSetMmThreadCharacteristics(TEXT("Pro Audio"), &task_idx);
		SetEvent(m_hStarted);
		bool result = ThreadLoop();
		if (hTask) {
			AvRevertMmThreadCharacteristics(hTask);
		}
		hTask    = NULL;
		task_idx = 0;
		return result;
	}

	[[nodiscard]] bool ThreadLoop() noexcept {
		bool stop = false;
		while (!stop) {
			HANDLE events[3] = {m_hBufferSwitch[0], m_hBufferSwitch[1], m_hStopRequest};
			switch (WaitForMultipleObjects(3, events, FALSE, INFINITE)) {
				case WAIT_OBJECT_0 + 0:
					CallFunc(0);
					break;
				case WAIT_OBJECT_0 + 1:
					CallFunc(1);
					break;
				case WAIT_OBJECT_0 + 2:
					stop = true;
					break;
				default:
					return false;
					break;
			}
		}
		return true;
	}

protected:
	virtual void CallFunc(std::size_t bufferIndex) = 0;

public:
	void Dispatch(std::size_t bufferIndex) final {
		if (SetEvent(m_hBufferSwitch[bufferIndex & 1u]) != TRUE) {
			throw std::bad_alloc();
		}
	}
};

template <typename Tfunc>
class BufferSwitchDispatcher
	: public BufferSwitchDispatcherBase {

private:
	Tfunc m_func;

public:
	BufferSwitchDispatcher(Tfunc func)
		: m_func(func) {
		return;
	}

protected:
	void CallFunc(std::size_t bufferIndex) final {
		m_func(bufferIndex);
	}
};

template <typename Tfunc>
inline std::unique_ptr<IBufferSwitchDispatcher> CreateBufferSwitchDispatcher(Tfunc func) {
	return std::make_unique<BufferSwitchDispatcher<Tfunc>>(func);
}



#if defined(UNICODE)
} // namespace Unicode
#else
} // namespace Ansi
#endif



} // namespace Windows



#endif // ASIO_SYSTEM_WINDOWS



} // namespace ASIO_VERSION_NAMESPACE



} // namespace ASIO



#endif // ASIO_ASIOSYSTEMWINDOWS_HPP
