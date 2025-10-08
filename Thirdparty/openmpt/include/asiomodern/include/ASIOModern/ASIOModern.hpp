
#ifndef ASIO_ASIOMODERN_HPP
#define ASIO_ASIOMODERN_HPP



#include "ASIOVersion.hpp"
#include "ASIOConfig.hpp"
#include "ASIOCore.hpp"

#include <array>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <cassert>
#include <cstddef>
#include <cstdint>



namespace ASIO {



inline namespace ASIO_VERSION_NAMESPACE {



inline namespace Modern {



class Error
	: public std::runtime_error {
private:
	ErrorCode m_Code;

private:
	static constexpr std::string_view Message(ErrorCode ec) noexcept {
		std::string_view message = "";
		switch (ec) {
			case ErrorCode::OK:
				message = "OK";
				break;
			case ErrorCode::SUCCESS:
				message = "SUCCESS";
				break;
			case ErrorCode::NotPresent:
				message = "NotPresent";
				break;
			case ErrorCode::HWMalfunction:
				message = "HWMalfunction";
				break;
			case ErrorCode::InvalidParameter:
				message = "InvalidParameter";
				break;
			case ErrorCode::InvalidMode:
				message = "InvalidMode";
				break;
			case ErrorCode::SPNotAdvancing:
				message = "SPNotAdvancing";
				break;
			case ErrorCode::NoClock:
				message = "NoClock";
				break;
			case ErrorCode::NoMemory:
				message = "NoMemory";
				break;
			default:
				message = "";
				break;
		}
		return message;
	}

public:
	Error(ErrorCode ec)
		: std::runtime_error(std::string("ASIO Error ") + std::string(Message(ec)))
		, m_Code(ec) {
		return;
	}
	ErrorCode Code() const noexcept {
		return m_Code;
	}
};



struct CallbacksWithContext {
	void (*bufferSwitch)(void * context, Long doubleBufferIndex, Bool directProcess) noexcept                                          = nullptr;
	void (*sampleRateDidChange)(void * context, SampleRate sRate) noexcept                                                             = nullptr;
	Long (*asioMessage)(void * context, MessageSelector selector, const Long value, const void * message, const Double * opt) noexcept = nullptr;
	const Time * (*bufferSwitchTimeInfo)(void * context, const Time * params, Long doubleBufferIndex, Bool directProcess) noexcept     = nullptr;
};


struct CallbacksWrapperState {
	CallbacksWithContext callbacks;
	void *               context = nullptr;
};


#if ASIO_SYSTEM_WINDOWS && (ASIO_COMPILER_GCC || ASIO_COMPILER_CLANG)
#pragma push_macro("cdecl")
#ifdef cdecl
#undef cdecl
#endif
#endif // ASIO_SYSTEM_WINDOWS && (ASIO_COMPILER_GCC || ASIO_COMPILER_CLANG)
template <typename Tarray, Tarray * A, std::size_t I>
class CallbacksWrapper {
public:
	static constexpr Callbacks init() noexcept {
		Callbacks result = {&CallbackBufferSwitch, &CallbackSampleRateDidChange, &CallbackAsioMessage, &CallbackBufferSwitchTimeInfo};
		return result;
	}

public:
	static void ASIO_CALL CallbackBufferSwitch ASIO_ATTR_CALL(Long doubleBufferIndex, Bool directProcess) noexcept {
		return (*A)[I].callbacks.bufferSwitch((*A)[I].context, doubleBufferIndex, directProcess);
	}
	static void ASIO_CALL CallbackSampleRateDidChange ASIO_ATTR_CALL(SampleRate sRate) noexcept {
		return (*A)[I].callbacks.sampleRateDidChange((*A)[I].context, sRate);
	}
	static Long ASIO_CALL CallbackAsioMessage ASIO_ATTR_CALL(MessageSelector selector, Long value, const void * message, const Double * opt) noexcept {
		return (*A)[I].callbacks.asioMessage((*A)[I].context, selector, value, message, opt);
	}
	static const Time * ASIO_CALL CallbackBufferSwitchTimeInfo ASIO_ATTR_CALL(const Time * params, Long doubleBufferIndex, Bool directProcess) noexcept {
		return (*A)[I].callbacks.bufferSwitchTimeInfo((*A)[I].context, params, doubleBufferIndex, directProcess);
	}
};
#if ASIO_SYSTEM_WINDOWS && (ASIO_COMPILER_GCC || ASIO_COMPILER_CLANG)
#pragma pop_macro("cdecl")
#endif // ASIO_SYSTEM_WINDOWS && (ASIO_COMPILER_GCC || ASIO_COMPILER_CLANG)


namespace detail {

template <typename T, std::size_t N, typename Tx>
constexpr std::array<T, N> init_array(const Tx & x) {
	std::array<T, N> result{};
	for (std::size_t i = 0; i < N; ++i) {
		result[i] = x;
	}
	return result;
}

} // namespace detail


template <std::uint64_t AppID1, std::uint64_t AppID2, std::size_t MaxInstances>
class AsioCallbacksMultiplexerGlobalState {
private:
	template <typename Tarray, Tarray * a, std::size_t... Is>
	static constexpr auto construct_callbacks_array(std::index_sequence<Is...>) noexcept -> std::array<Callbacks, sizeof...(Is)> {
		return {CallbacksWrapper<Tarray, a, Is>::init()...};
	}

private:
	static inline std::mutex                                      s_AllocationMutex;
	static inline std::array<bool, MaxInstances>                  s_Allocation                = detail::init_array<bool, MaxInstances>(false);
	static inline std::array<CallbacksWrapperState, MaxInstances> s_AsioCallbackWrapperStates = detail::init_array<CallbacksWrapperState, MaxInstances>(CallbacksWrapperState{
		{nullptr, nullptr, nullptr, nullptr},
		nullptr
    });
	static constexpr inline std::array<Callbacks, MaxInstances>   s_AsioCallbacks             = construct_callbacks_array<std::array<CallbacksWrapperState, MaxInstances>, &s_AsioCallbackWrapperStates>(std::make_index_sequence<MaxInstances>());

private:
	static std::size_t Alloc() {
		std::lock_guard<std::mutex> guard(s_AllocationMutex);
		for (std::size_t i = 0; i < MaxInstances; ++i) {
			if (!s_Allocation[i]) {
				s_Allocation[i] = true;
				return i;
			}
		}
		throw Error(ErrorCode::NoMemory);
	}
	static void Free(std::size_t index) noexcept {
		std::lock_guard<std::mutex> guard(s_AllocationMutex);
		assert(s_Allocation[index]);
		s_Allocation[index] = false;
	}

public:
	static std::pair<std::size_t, Callbacks> Multiplex(void * context, CallbacksWithContext callbacks) {
		std::size_t cookie                  = Alloc();
		s_AsioCallbackWrapperStates[cookie] = {callbacks, context};
		return std::make_pair(cookie, s_AsioCallbacks[cookie]);
	}
	static void Unmultiplex(std::pair<std::size_t, Callbacks> state) noexcept {
		Free(state.first);
	}
};


class IMultiplexedCallbacks {
protected:
	IMultiplexedCallbacks() = default;

public:
	IMultiplexedCallbacks(const IMultiplexedCallbacks &)             = delete;
	IMultiplexedCallbacks & operator=(const IMultiplexedCallbacks &) = delete;

public:
	virtual ~IMultiplexedCallbacks() = default;

public:
	virtual operator const Callbacks *() const noexcept = 0;
	virtual operator const Callbacks &() const noexcept = 0;
	virtual operator Callbacks *() noexcept             = 0;
	virtual operator Callbacks &() noexcept             = 0;
};


template <std::uint64_t AppID1, std::uint64_t AppID2, std::size_t MaxInstances>
class MultiplexedCallbacks
	: public IMultiplexedCallbacks {
public:
	using GlobalState = AsioCallbacksMultiplexerGlobalState<AppID1, AppID2, MaxInstances>;
	using State       = std::unique_ptr<IMultiplexedCallbacks>;

private:
	std::pair<std::size_t, Callbacks> m_State;

public:
	MultiplexedCallbacks(void * context, CallbacksWithContext callbacks)
		: m_State(GlobalState::Multiplex(context, callbacks)) {
		if (!callbacks.bufferSwitch) {
			m_State.second.bufferSwitch = nullptr;
		}
		if (!callbacks.sampleRateDidChange) {
			m_State.second.sampleRateDidChange = nullptr;
		}
		if (!callbacks.asioMessage) {
			m_State.second.asioMessage = nullptr;
		}
		if (!callbacks.bufferSwitchTimeInfo) {
			m_State.second.bufferSwitchTimeInfo = nullptr;
		}
	}
	MultiplexedCallbacks(const MultiplexedCallbacks &)             = delete;
	MultiplexedCallbacks & operator=(const MultiplexedCallbacks &) = delete;
	~MultiplexedCallbacks() final {
		GlobalState::Unmultiplex(m_State);
	}

public:
	operator const Callbacks *() const noexcept final {
		return &m_State.second;
	}
	operator const Callbacks &() const noexcept final {
		return m_State.second;
	}
	operator Callbacks *() noexcept final {
		return &m_State.second;
	}
	operator Callbacks &() noexcept final {
		return m_State.second;
	}

public:
	static std::unique_ptr<MultiplexedCallbacks> make(void * context, CallbacksWithContext callbacks) {
		return std::make_unique<MultiplexedCallbacks>(context, callbacks);
	}
	static std::unique_ptr<MultiplexedCallbacks> null() {
		return std::unique_ptr<MultiplexedCallbacks>();
	}
};



struct Channels {
	Long Input  = 0;
	Long Output = 0;
};

struct Latencies {
	Long Input  = 0;
	Long Output = 0;
};

struct BufferSizes {
	Long Min         = 0;
	Long Max         = 0;
	Long Preferred   = 0;
	Long Granularity = 0;
};

struct SamplePosition {
	Samples   samplePosition = 0;
	TimeStamp systemTime     = 0;
};

struct BufferIndex {
	std::uint8_t Index : 1;
	constexpr BufferIndex() noexcept
		: Index(0) {
	}
	constexpr BufferIndex(Long doubleBufferIndex) noexcept
		: Index(static_cast<std::uint8_t>(static_cast<ULong>(doubleBufferIndex) & 1u)) {
	}
	constexpr BufferIndex(std::size_t bufferIndex) noexcept
		: Index(static_cast<std::uint8_t>(bufferIndex & 1u)) {
	}
	constexpr operator std::size_t() const noexcept {
		return Index;
	}
};

class Driver {

public:
	class ICallbackHandler {
	protected:
		ICallbackHandler() = default;

	public:
		ICallbackHandler(const ICallbackHandler &)             = delete;
		ICallbackHandler & operator=(const ICallbackHandler &) = delete;
		virtual ~ICallbackHandler()                            = default;

	public:
		virtual Long         CallbackMessage(Driver & driver, MessageSelector selector, Long value, const void * message, const Double * opt) noexcept = 0;
		virtual void         CallbackSampleRateDidChange(Driver & driver, SampleRate sRate) noexcept                                                   = 0;
		virtual void         CallbackBufferSwitch(Driver & driver, Long doubleBufferIndex, Bool directProcess) noexcept                                = 0;
		virtual const Time * CallbackBufferSwitchTimeInfo(Driver & driver, const Time * params, Long doubleBufferIndex, Bool directProcess) noexcept   = 0;
	};

	class CallbackHandler
		: public ICallbackHandler {
		/*
		void MessageResetRequest() noexcept override;
		bool MessageBufferSizeChange(ASIO::Long newSize) noexcept override;
		bool MessageResyncRequest() noexcept override;
		void MessageLatenciesChanged() noexcept override;
		ASIO::Long MessageMMCCommand(ASIO::Long value, const void * message, const ASIO::Double * opt) noexcept override;
		void MessageOverload() noexcept override;
		ASIO::Long MessageUnknown(ASIO::MessageSelector selector, ASIO::Long value, const void * message, const ASIO::Double * opt) noexcept override;
		void RealtimeSampleRateDidChange(ASIO::SampleRate sRate) noexcept override;
		void RealtimeRequestDeferredProcessing(bool value) noexcept override;
		void RealtimeTimeInfo(ASIO::Time time) noexcept override;
		void RealtimeBufferSwitch(ASIO::BufferIndex bufferIndex) noexcept override;
		*/
	public:
		CallbackHandler()                                    = default;
		CallbackHandler(const CallbackHandler &)             = delete;
		CallbackHandler & operator=(const CallbackHandler &) = delete;
		virtual ~CallbackHandler()                           = default;

	public:
		bool MessageSelectorSupported(MessageSelector selector) const noexcept {
			bool result = false;
			switch (selector) {
				case MessageSelector::SelectorSupported:
					result = true;
					break;
				case MessageSelector::EngineVersion:
					result = true;
					break;
				case MessageSelector::ResetRequest:
					result = true;
					break;
				case MessageSelector::BufferSizeChange:
					result = true;
					break;
				case MessageSelector::ResyncRequest:
					result = true;
					break;
				case MessageSelector::LatenciesChanged:
					result = true;
					break;
				case MessageSelector::SupportsTimeInfo:
					result = true;
					break;
				case MessageSelector::SupportsTimeCode:
					result = true;
					break;
				case MessageSelector::MMCCommand:
					result = true;
					break;
				case MessageSelector::SupportsInputMonitor:
					result = true;
					break;
				case MessageSelector::SupportsInputGain:
					result = true;
					break;
				case MessageSelector::SupportsInputMeter:
					result = true;
					break;
				case MessageSelector::SupportsOutputGain:
					result = true;
					break;
				case MessageSelector::SupportsOutputMeter:
					result = true;
					break;
				case MessageSelector::Overload:
					result = true;
					break;
			}
			return result;
		}
		Long MessageEngineVersion() const noexcept {
			return 2;
		}
		virtual void MessageResetRequest() noexcept = 0;
		virtual bool MessageBufferSizeChange(Long newSize) noexcept {
			static_cast<void>(newSize);
			return false;
		}
		virtual bool MessageResyncRequest() noexcept {
			return false;
		}
		virtual void MessageLatenciesChanged() noexcept {
			return;
		}
		bool MessageSupportsTimeInfo() const noexcept {
			return true;
		}
		bool MessageSupportsTimeCode() const noexcept {
			return true;
		}
		virtual Long MessageMMCCommand(Long value, const void * message, const Double * opt) noexcept {
			static_cast<void>(value);
			static_cast<void>(message);
			static_cast<void>(opt);
			return 0;
		}
		bool MessageSupportsInputMonitor() const noexcept {
			return true;
		}
		bool MessageSupportsInputGain() const noexcept {
			return true;
		}
		bool MessageSupportsInputMeter() const noexcept {
			return true;
		}
		bool MessageSupportsOutputGain() const noexcept {
			return true;
		}
		bool MessageSupportsOutputMeter() const noexcept {
			return true;
		}
		virtual void MessageOverload() noexcept {
			return;
		}
		virtual Long MessageUnknown(MessageSelector selector, Long value, const void * message, const Double * opt) noexcept {
			static_cast<void>(selector);
			static_cast<void>(value);
			static_cast<void>(message);
			static_cast<void>(opt);
			return 0;
		}
		virtual void RealtimeSampleRateDidChange(SampleRate sRate) noexcept {
			static_cast<void>(sRate);
		}
		virtual void RealtimeRequestDeferredProcessing(bool deferred) noexcept {
			static_cast<void>(deferred);
		}
		virtual void RealtimeTimeInfo(Time time) noexcept {
			static_cast<void>(time);
		}
		virtual void RealtimeBufferSwitch(ASIO::BufferIndex bufferIndex) noexcept = 0;

	public:
		Long CallbackMessage(Driver & driver, MessageSelector selector, Long value, const void * message, const Double * opt) noexcept final {
			static_cast<void>(driver);
			Long result = 0;
			switch (selector) {
				case MessageSelector::SelectorSupported:
					result = MessageSelectorSupported(static_cast<MessageSelector>(value)) ? 1 : 0;
					break;
				case MessageSelector::EngineVersion:
					result = MessageEngineVersion();
					break;
				case MessageSelector::ResetRequest:
					MessageResetRequest();
					result = 1;
					break;
				case MessageSelector::BufferSizeChange:
					result = MessageBufferSizeChange(value) ? 1 : 0;
					break;
				case MessageSelector::ResyncRequest:
					result = MessageResyncRequest() ? 1 : 0;
					break;
				case MessageSelector::LatenciesChanged:
					MessageLatenciesChanged();
					result = 1;
					break;
				case MessageSelector::SupportsTimeInfo:
					result = MessageSupportsTimeInfo() ? 1 : 0;
					break;
				case MessageSelector::SupportsTimeCode:
					result = MessageSupportsTimeCode() ? 1 : 0;
					break;
				case MessageSelector::MMCCommand:
					result = MessageMMCCommand(value, message, opt);
					break;
				case MessageSelector::SupportsInputMonitor:
					result = MessageSupportsInputMonitor() ? 1 : 0;
					break;
				case MessageSelector::SupportsInputGain:
					result = MessageSupportsInputGain() ? 1 : 0;
					break;
				case MessageSelector::SupportsInputMeter:
					result = MessageSupportsInputMeter() ? 1 : 0;
					break;
				case MessageSelector::SupportsOutputGain:
					result = MessageSupportsOutputGain() ? 1 : 0;
					break;
				case MessageSelector::SupportsOutputMeter:
					result = MessageSupportsOutputMeter() ? 1 : 0;
					break;
				case MessageSelector::Overload:
					MessageOverload();
					result = 1;
					break;
				default:
					result = MessageUnknown(selector, value, message, opt);
					break;
			}
			return result;
		}
		void CallbackSampleRateDidChange(Driver & driver, SampleRate sRate) noexcept final {
			static_cast<void>(driver);
			RealtimeSampleRateDidChange(sRate);
		}
		void CallbackBufferSwitch(Driver & driver, Long doubleBufferIndex, Bool directProcess) noexcept final {
			CallbackBufferSwitchTimeInfo(driver, nullptr, doubleBufferIndex, directProcess);
		}
		const Time * CallbackBufferSwitchTimeInfo(Driver & driver, const Time * params, Long doubleBufferIndex, Bool directProcess) noexcept final {
			Time time;
			if (params) {
				time = *params;
			} else {
				try {
					HiLoLongLong samplePosition = 0;
					HiLoLongLong systemTime     = 0;
					if (driver.realDriver().getSamplePosition(&samplePosition, &systemTime) == ErrorCode::OK) {
						time.timeInfo.flags |= TimeInfoFlagSamplePositionValid | TimeInfoFlagSystemTimeValid;
						time.timeInfo.samplePosition = samplePosition;
						time.timeInfo.systemTime     = systemTime;
						time.timeInfo.speed          = 1.0;
						SampleRate sampleRate        = 0.0;
						if (driver.realDriver().getSampleRate(&sampleRate) == ErrorCode::OK) {
							if (sampleRate >= 0.0) {
								time.timeInfo.flags |= TimeInfoFlagSampleRateValid;
								time.timeInfo.sampleRate = sampleRate;
							}
						}
					}
				} catch (...) {
					// nothing
				}
			}
			RealtimeRequestDeferredProcessing(!directProcess);
			RealtimeTimeInfo(time);
			RealtimeBufferSwitch(doubleBufferIndex);
			return params;
		}
	};


private:
	std::unique_ptr<IDriver> m_Driver;

	ICallbackHandler * m_CallbackHandler = nullptr;

	std::unique_ptr<IMultiplexedCallbacks> m_Callbacks;

private:
	const IDriver & realDriver() const noexcept {
		return *m_Driver;
	}
	IDriver & realDriver() noexcept {
		return *m_Driver;
	}

public:
	explicit Driver(std::unique_ptr<IDriver> driver)
		: m_Driver(std::move(driver)) {
		return;
	}

	Driver(const Driver &) = delete;

	Driver & operator=(const Driver &) = delete;

	~Driver() {
		return;
	}

private:
	[[nodiscard]] static ErrorCode CheckResultOutOfMemory(ErrorCode ec) {
		if (ec == ErrorCode::NoMemory) {
			throw std::bad_alloc();
		}
		return ec;
	}

	static void CheckResult(ErrorCode expected, ErrorCode ec) {
		ec = CheckResultOutOfMemory(ec);
		if (ec != expected) {
			throw Error(ec);
		}
	}

	static void CheckResultNoOutOfMemory(ErrorCode expected, ErrorCode ec) {
		if (ec != expected) {
			throw Error(ec);
		}
	}

	static void CheckOK(ErrorCode ec) {
		CheckResult(ErrorCode::OK, ec);
	}

	static void CheckSUCCESS(ErrorCode ec) {
		CheckResult(ErrorCode::SUCCESS, ec);
	}

	static void CheckOKNoOutOfMemory(ErrorCode ec) {
		CheckResultNoOutOfMemory(ErrorCode::OK, ec);
	}

	static void CheckSUCCESSNoOutOfMemory(ErrorCode ec) {
		CheckResultNoOutOfMemory(ErrorCode::SUCCESS, ec);
	}

private:
	static Driver * ThisFromVoid(void * context) noexcept {
		return reinterpret_cast<Driver *>(context);
	}

	void * MyContext() noexcept {
		return this;
	}

	static constexpr CallbacksWithContext MyCallbacks() noexcept {
		CallbacksWithContext result = {&CallbackBufferSwitch, &CallbackSampleRateDidChange, &CallbackAsioMessage, &CallbackBufferSwitchTimeInfo};
		return result;
	}

private:
	static void CallbackBufferSwitch(void * context, Long doubleBufferIndex, Bool directProcess) noexcept {
		assert(context);
		assert(ThisFromVoid(context)->m_CallbackHandler);
		return ThisFromVoid(context)->m_CallbackHandler->CallbackBufferSwitch(*ThisFromVoid(context), doubleBufferIndex, directProcess);
	}
	static void CallbackSampleRateDidChange(void * context, SampleRate sRate) noexcept {
		assert(context);
		assert(ThisFromVoid(context)->m_CallbackHandler);
		return ThisFromVoid(context)->m_CallbackHandler->CallbackSampleRateDidChange(*ThisFromVoid(context), sRate);
	}
	static Long CallbackAsioMessage(void * context, MessageSelector selector, Long value, const void * message, const Double * opt) noexcept {
		assert(context);
		assert(ThisFromVoid(context)->m_CallbackHandler);
		return ThisFromVoid(context)->m_CallbackHandler->CallbackMessage(*ThisFromVoid(context), selector, value, message, opt);
	}
	static const Time * CallbackBufferSwitchTimeInfo(void * context, const Time * params, Long doubleBufferIndex, Bool directProcess) noexcept {
		assert(context);
		assert(ThisFromVoid(context)->m_CallbackHandler);
		return ThisFromVoid(context)->m_CallbackHandler->CallbackBufferSwitchTimeInfo(*ThisFromVoid(context), params, doubleBufferIndex, directProcess);
	}

public:
	std::string getDriverName() const {
		DriverName name = "";
		m_Driver->getDriverName(&name);
		return name;
	}

	[[nodiscard]] Long getDriverVersion() const {
		return m_Driver->getDriverVersion();
	}

	std::string getErrorMessage() const {
		ErrorMessage string = "";
		m_Driver->getErrorMessage(&string);
		return string;
	}

	void start() {
		CheckOK(m_Driver->start());
	}

	void stop() {
		CheckOK(m_Driver->stop());
	}

	Channels getChannels() const {
		Channels result;
		CheckOK(m_Driver->getChannels(&result.Input, &result.Output));
		return result;
	}

	Latencies getLatencies() const {
		Latencies result;
		CheckOK(m_Driver->getLatencies(&result.Input, &result.Output));
		return result;
	}

	BufferSizes getBufferSizes() const {
		BufferSizes result;
		CheckOK(m_Driver->getBufferSize(&result.Min, &result.Max, &result.Preferred, &result.Granularity));
		return result;
	}

	bool canSampleRate(SampleRate sampleRate) const {
		ErrorCode ec = m_Driver->canSampleRate(sampleRate);
		if ((ec != ErrorCode::OK) && (ec != ErrorCode::NoClock)) {
			CheckOK(ec);
		}
		return (ec == ErrorCode::OK);
	}

	SampleRate getSampleRate() const {
		SampleRate sampleRate = 0.0;
		ErrorCode  ec         = m_Driver->getSampleRate(&sampleRate);
		if ((ec != ErrorCode::OK) && (ec != ErrorCode::NoClock)) {
			CheckOK(ec);
		}
		return sampleRate;
	}

	void setSampleRate(SampleRate sampleRate) {
		CheckOK(m_Driver->setSampleRate(sampleRate));
	}

	std::vector<ClockSource> getClockSources() const {
		std::vector<ClockSource> clocks(1);
		Long                     numSources = 1;
		CheckOK(m_Driver->getClockSources(clocks.data(), &numSources));
		if (numSources > 1) {
			clocks.resize(numSources);
			CheckOK(m_Driver->getClockSources(clocks.data(), &numSources));
		}
		return clocks;
	}

	void setClockSource(Long reference) {
		CheckOK(m_Driver->setClockSource(reference));
	}

	SamplePosition getSamplePosition() const {
		HiLoLongLong samplePosition = 0;
		HiLoLongLong systemTime     = 0;
		CheckOK(m_Driver->getSamplePosition(&samplePosition, &systemTime));
		SamplePosition result;
		result.samplePosition = samplePosition;
		result.systemTime     = systemTime;
		return result;
	}

	ChannelInfo getChannelInfo(Long channel, Bool input) const {
		ChannelInfo info;
		info.channel = channel;
		info.isInput = input;
		CheckOK(m_Driver->getChannelInfo(&info));
		return info;
	}

	template <std::uint64_t AppID1, std::uint64_t AppID2, std::size_t MaxInstances = 256>
	void createBuffers(std::vector<BufferInfo> & bufferInfos, Long bufferSize, ICallbackHandler & handler) {
		assert(!m_CallbackHandler);
		assert(!m_Callbacks);
		m_CallbackHandler = &handler;
		m_Callbacks       = MultiplexedCallbacks<AppID1, AppID2, MaxInstances>::make(MyContext(), MyCallbacks());
		try {
			CheckOKNoOutOfMemory(m_Driver->createBuffers(bufferInfos.data(), static_cast<Long>(bufferInfos.size()), bufferSize, *m_Callbacks));
		} catch (...) {
			m_Callbacks       = nullptr;
			m_CallbackHandler = nullptr;
			throw;
		}
	}

	void disposeBuffers() {
		assert(m_CallbackHandler);
		assert(m_Callbacks);
		try {
			CheckOK(m_Driver->disposeBuffers());
		} catch (...) {
			m_Callbacks       = nullptr;
			m_CallbackHandler = nullptr;
			throw;
		}
		m_Callbacks       = nullptr;
		m_CallbackHandler = nullptr;
	}

	bool controlPanel() {
		ErrorCode ec = m_Driver->controlPanel();
		if ((ec != ErrorCode::OK) && (ec != ErrorCode::NotPresent)) {
			CheckOK(ec);
		}
		return (ec == ErrorCode::OK);
	}

	[[deprecated]] [[nodiscard]] ErrorCode future(FutureSelector selector, void * opt) {
		return CheckResultOutOfMemory(m_Driver->future(selector, opt));
	}

	void enableTimeCodeRead() {
		CheckSUCCESS(m_Driver->future(FutureSelector::EnableTimeCodeRead, nullptr));
	}

	void disableTimeCodeRead() {
		CheckSUCCESS(m_Driver->future(FutureSelector::DisableTimeCodeRead, nullptr));
	}

	void setInputMonitor(InputMonitor & inputMonitor) {
		CheckSUCCESS(m_Driver->future(FutureSelector::SetInputMonitor, &inputMonitor));
	}

	void transport(TransportParameters & transportParameters) {
		CheckSUCCESS(m_Driver->future(FutureSelector::Transport, &transportParameters));
	}

	void setInputGain(Long channel, Bool input, Long gain) {
		ChannelControls channelControls;
		channelControls.channel = channel;
		channelControls.isInput = input;
		channelControls.gain    = gain;
		CheckSUCCESS(m_Driver->future(FutureSelector::SetInputGain, &channelControls));
	}

	Long getInputMeter(Long channel, Bool input) const {
		ChannelControls channelControls;
		channelControls.channel = channel;
		channelControls.isInput = input;
		CheckSUCCESS(m_Driver->future(FutureSelector::GetInputMeter, &channelControls));
		return channelControls.meter;
	}

	void setOutputGain(Long channel, Bool input, Long gain) {
		ChannelControls channelControls;
		channelControls.channel = channel;
		channelControls.isInput = input;
		channelControls.gain    = gain;
		CheckSUCCESS(m_Driver->future(FutureSelector::SetOutputGain, &channelControls));
	}

	Long getOutputMeter(Long channel, Bool input) const {
		ChannelControls channelControls;
		channelControls.channel = channel;
		channelControls.isInput = input;
		CheckSUCCESS(m_Driver->future(FutureSelector::GetOutputMeter, &channelControls));
		return channelControls.meter;
	}

	bool canInputMonitor() const {
		ErrorCode ec = CheckResultOutOfMemory(m_Driver->future(FutureSelector::CanInputMonitor, nullptr));
		return (ec == ErrorCode::SUCCESS);
	}

	bool canTimeInfo() const {
		ErrorCode ec = CheckResultOutOfMemory(m_Driver->future(FutureSelector::CanTimeInfo, nullptr));
		return (ec == ErrorCode::SUCCESS);
	}

	bool canTimeCode() const {
		ErrorCode ec = CheckResultOutOfMemory(m_Driver->future(FutureSelector::CanTimeCode, nullptr));
		return (ec == ErrorCode::SUCCESS);
	}

	bool canTransport() const {
		ErrorCode ec = CheckResultOutOfMemory(m_Driver->future(FutureSelector::CanTransport, nullptr));
		return (ec == ErrorCode::SUCCESS);
	}

	bool canInputGain() const {
		ErrorCode ec = CheckResultOutOfMemory(m_Driver->future(FutureSelector::CanInputGain, nullptr));
		return (ec == ErrorCode::SUCCESS);
	}

	bool canInputMeter() const {
		ErrorCode ec = CheckResultOutOfMemory(m_Driver->future(FutureSelector::CanInputMeter, nullptr));
		return (ec == ErrorCode::SUCCESS);
	}

	bool canOutputGain() const {
		ErrorCode ec = CheckResultOutOfMemory(m_Driver->future(FutureSelector::CanOutputGain, nullptr));
		return (ec == ErrorCode::SUCCESS);
	}

	bool canOutputMeter() const {
		ErrorCode ec = CheckResultOutOfMemory(m_Driver->future(FutureSelector::CanOutputMeter, nullptr));
		return (ec == ErrorCode::SUCCESS);
	}

	[[nodiscard]] ErrorCode optionalOne(void * param) {
		return CheckResultOutOfMemory(m_Driver->future(FutureSelector::OptionalOne, param));
	}

	void setIoFormat(IoFormatType type) {
		IoFormat ioFormat;
		ioFormat.FormatType = type;
		CheckSUCCESS(m_Driver->future(FutureSelector::SetIoFormat, &ioFormat));
	}

	IoFormatType getIoFormat() const {
		IoFormat ioFormat;
		CheckSUCCESS(m_Driver->future(FutureSelector::GetIoFormat, &ioFormat));
		return ioFormat.FormatType;
	}

	bool canDoIoFormat(IoFormatType type) {
		IoFormat ioFormat;
		ioFormat.FormatType = type;
		ErrorCode ec        = CheckResultOutOfMemory(m_Driver->future(FutureSelector::CanDoIoFormat, &ioFormat));
		return (ec == ErrorCode::SUCCESS);
	}

	bool canReportOverload() const {
		ErrorCode ec = CheckResultOutOfMemory(m_Driver->future(FutureSelector::CanReportOverload, nullptr));
		return (ec == ErrorCode::SUCCESS);
	}

	InternalBufferInfo getInternalBufferSamples() const {
		InternalBufferInfo internalBufferInfo;
		ErrorCode          ec = CheckResultOutOfMemory(m_Driver->future(FutureSelector::CanReportOverload, &internalBufferInfo));
		if (ec != ErrorCode::SUCCESS) {
			return InternalBufferInfo();
		}
		return internalBufferInfo;
	}

	bool canOutputReady() const {
		ErrorCode ec = m_Driver->outputReady();
		if ((ec != ErrorCode::OK) && (ec != ErrorCode::NotPresent)) {
			CheckOK(ec);
		}
		return (ec == ErrorCode::OK);
	}

	void outputReady() {
		CheckOK(m_Driver->outputReady());
	}
};



} // namespace Modern



} // namespace ASIO_VERSION_NAMESPACE



} // namespace ASIO



#endif // ASIO_ASIOMODERN_HPP
