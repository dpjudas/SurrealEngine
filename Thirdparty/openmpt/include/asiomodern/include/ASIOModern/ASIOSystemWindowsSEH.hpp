
#ifndef ASIO_ASIOSYSTEMWINDOWSSEH_HPP
#define ASIO_ASIOSYSTEMWINDOWSSEH_HPP



#include "ASIOVersion.hpp"
#include "ASIOConfig.hpp"
#include "ASIOCore.hpp"
#include "ASIOSystemWindows.hpp"

#include <memory>
#include <string_view>

#include <cassert>

#if ASIO_SYSTEM_WINDOWS
#include <windows.h>
#if !defined(NTDDI_VERSION)
#error "NTDDI_VERSION undefined"
#endif
#if !defined(_WIN32_WINNT)
#error "_WIN32_WINNT undefined"
#endif
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



namespace SEH {

struct DriverCrash {
private:
	DWORD            m_Code;
	std::string_view m_Func;

public:
	explicit constexpr DriverCrash(DWORD code, std::string_view func) noexcept
		: m_Code(code)
		, m_Func(func) {
		return;
	}

public:
	constexpr DWORD code() const noexcept {
		return m_Code;
	}
	constexpr std::string_view func() const noexcept {
		return m_Func;
	}
};

class IState {
protected:
	IState() noexcept = default;

public:
	IState(const IState &)             = delete;
	IState & operator=(const IState &) = delete;

public:
	virtual ~IState() = default;
};

class ITranslator {
protected:
	ITranslator() noexcept = default;

public:
	ITranslator(const ITranslator &)             = delete;
	ITranslator & operator=(const ITranslator &) = delete;

public:
	virtual ~ITranslator() = default;

public:
	[[nodiscard]] virtual LONG TranslatorFilter(std::unique_ptr<IState> & state, DWORD code, LPEXCEPTION_POINTERS records, std::string_view func) const noexcept = 0;
	[[noreturn]] virtual void  TranslatorHandler(std::unique_ptr<IState> & state, DWORD code, std::string_view func) const                                       = 0;
};

class DefaultTranslator
	: public ITranslator {
public:
	virtual ~DefaultTranslator() = default;

public:
	[[nodiscard]] LONG TranslatorFilter(std::unique_ptr<IState> & /* state */, DWORD /* code */, LPEXCEPTION_POINTERS /* records */, std::string_view /* func */) const noexcept final {
		return EXCEPTION_EXECUTE_HANDLER;
	}
	[[noreturn]] void TranslatorHandler(std::unique_ptr<IState> & /* state */, DWORD code, std::string_view func) const final {
		throw DriverCrash(code, func);
	}
};

class Driver
	: public IDriver {

private:
	ISystemDriver *              m_Driver     = nullptr;
	std::unique_ptr<ITranslator> m_Translator = nullptr;

private:
	template <typename Tfn>
	static auto TranslateSEtry(std::unique_ptr<ITranslator> & translator, std::unique_ptr<IState> & state, Tfn fn, std::string_view func) -> decltype(fn()) {
		__try
		{
			return fn();
		} __except (translator->TranslatorFilter(state, GetExceptionCode(), GetExceptionInformation(), func))
		{
			translator->TranslatorHandler(state, GetExceptionCode(), func);
		}
		throw DriverCrash(0, func);
	}

	template <typename Tfn>
	auto TranslateSE(Tfn fn, std::string_view func) -> decltype(fn()) {
		assert(m_Translator);
		std::unique_ptr<IState> state;
		return TranslateSEtry(m_Translator, state, fn, func);
	}

	template <typename Tfn>
	auto CallDriver(Tfn fn, std::string_view func) -> decltype(fn()) {
		return TranslateSE(fn, func);
	}

public:
	explicit Driver(CLSID clsid, HWND wnd, std::unique_ptr<ITranslator> translator = std::make_unique<DefaultTranslator>())
		: m_Translator(std::move(translator)) {
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

	~Driver() noexcept(false) override {
		closeDriver(m_Driver);
	}

private:
	ISystemDriver * openDriver(CLSID clsid) {
		return CallDriver([&]() { return OpenDriver(clsid); }, __func__);
	}

	ULONG closeDriver(ISystemDriver * driver) {
		return CallDriver([&]() { return CloseDriver(driver); }, __func__);
	}

private:
	Bool initDriver(HWND sysHandle) {
		return static_cast<Bool>(CallDriver([&]() { return m_Driver->init(reinterpret_cast<SysHandle>(sysHandle)); }, __func__));
	}

public:
	void getDriverName(DriverName * name) final {
		return CallDriver([&]() { return m_Driver->getDriverName(name); }, __func__);
	}
	[[nodiscard]] Long getDriverVersion() final {
		return CallDriver([&]() { return m_Driver->getDriverVersion(); }, __func__);
	}
	void getErrorMessage(ErrorMessage * string) final {
		return CallDriver([&]() { return m_Driver->getErrorMessage(string); }, __func__);
	}
	[[nodiscard]] ErrorCode start() final {
		return CallDriver([&]() { return m_Driver->start(); }, __func__);
	}
	[[nodiscard]] ErrorCode stop() final {
		return CallDriver([&]() { return m_Driver->stop(); }, __func__);
	}
	[[nodiscard]] ErrorCode getChannels(Long * numInputChannels, Long * numOutputChannels) final {
		return CallDriver([&]() { return m_Driver->getChannels(numInputChannels, numOutputChannels); }, __func__);
	}
	[[nodiscard]] ErrorCode getLatencies(Long * inputLatency, Long * outputLatency) final {
		return CallDriver([&]() { return m_Driver->getLatencies(inputLatency, outputLatency); }, __func__);
	}
	[[nodiscard]] ErrorCode getBufferSize(Long * minSize, Long * maxSize, Long * preferredSize, Long * granularity) final {
		return CallDriver([&]() { return m_Driver->getBufferSize(minSize, maxSize, preferredSize, granularity); }, __func__);
	}
	[[nodiscard]] ErrorCode canSampleRate(SampleRate sampleRate) final {
		return CallDriver([&]() { return m_Driver->canSampleRate(sampleRate); }, __func__);
	}
	[[nodiscard]] ErrorCode getSampleRate(SampleRate * sampleRate) final {
		return CallDriver([&]() { return m_Driver->getSampleRate(sampleRate); }, __func__);
	}
	[[nodiscard]] ErrorCode setSampleRate(SampleRate sampleRate) final {
		return CallDriver([&]() { return m_Driver->setSampleRate(sampleRate); }, __func__);
	}
	[[nodiscard]] ErrorCode getClockSources(ClockSource * clocks, Long * numSources) final {
		return CallDriver([&]() { return m_Driver->getClockSources(clocks, numSources); }, __func__);
	}
	[[nodiscard]] ErrorCode setClockSource(Long reference) final {
		return CallDriver([&]() { return m_Driver->setClockSource(reference); }, __func__);
	}
	[[nodiscard]] ErrorCode getSamplePosition(HiLoLongLong * samplePosition, HiLoLongLong * timeStamp) final {
		return CallDriver([&]() { return m_Driver->getSamplePosition(samplePosition, timeStamp); }, __func__);
	}
	[[nodiscard]] ErrorCode getChannelInfo(ChannelInfo * info) final {
		return CallDriver([&]() { return m_Driver->getChannelInfo(info); }, __func__);
	}
	[[nodiscard]] ErrorCode createBuffers(BufferInfo * bufferInfos, Long numChannels, Long bufferSize, const Callbacks * callbacks) final {
		return CallDriver([&]() { return m_Driver->createBuffers(bufferInfos, numChannels, bufferSize, callbacks); }, __func__);
	}
	[[nodiscard]] ErrorCode disposeBuffers() final {
		return CallDriver([&]() { return m_Driver->disposeBuffers(); }, __func__);
	}
	[[nodiscard]] ErrorCode controlPanel() final {
		return CallDriver([&]() { return m_Driver->controlPanel(); }, __func__);
	}
	[[nodiscard]] ErrorCode future(FutureSelector selector, void * opt) final {
		return CallDriver([&]() { return m_Driver->future(selector, opt); }, __func__);
	}
	[[nodiscard]] ErrorCode outputReady() final {
		return CallDriver([&]() { return m_Driver->outputReady(); }, __func__);
	}
};



} // namespace SEH



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
