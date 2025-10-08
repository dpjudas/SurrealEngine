
#ifndef ASIO_ASIOCORE_HPP
#define ASIO_ASIOCORE_HPP



#include "ASIOVersion.hpp"
#include "ASIOConfig.hpp"

#include <algorithm>
#include <string>
#include <type_traits>

#include <cstddef>
#include <cstdint>



namespace ASIO {



inline namespace ASIO_VERSION_NAMESPACE {



inline namespace Core {



using Samples = HiLoLongLong;

using TimeStamp = HiLoLongLong;

using SampleRate = Double;

enum class SampleType : Long {
	Int16MSB    = 0,
	Int24MSB    = 1,
	Int32MSB    = 2,
	Float32MSB  = 3,
	Float64MSB  = 4,
	Int32MSB16  = 8,
	Int32MSB18  = 9,
	Int32MSB20  = 10,
	Int32MSB24  = 11,
	Int16LSB    = 16,
	Int24LSB    = 17,
	Int32LSB    = 18,
	Float32LSB  = 19,
	Float64LSB  = 20,
	Int32LSB16  = 24,
	Int32LSB18  = 25,
	Int32LSB20  = 26,
	Int32LSB24  = 27,
	DSDInt8LSB1 = 32,
	DSDInt8MSB1 = 33,
	DSDInt8NER8 = 40,
};
static_assert(sizeof(SampleType) == SizeOfLong);

enum class ErrorCode : Long {
	OK               = 0,
	SUCCESS          = 0x3f4847a0,
	NotPresent       = -1000,
	HWMalfunction    = -999,
	InvalidParameter = -998,
	InvalidMode      = -997,
	SPNotAdvancing   = -996,
	NoClock          = -995,
	NoMemory         = -994,
};
static_assert(sizeof(ErrorCode) == SizeOfLong);

enum TimeCodeFlags : ULong {
	TimeCodeFlagValid      = 1 << 0,
	TimeCodeFlagRunning    = 1 << 1,
	TimeCodeFlagReverse    = 1 << 2,
	TimeCodeFlagOnspeed    = 1 << 3,
	TimeCodeFlagStill      = 1 << 4,
	TimeCodeFlagSpeedValid = 1 << 8,
};
constexpr inline TimeCodeFlags operator|(TimeCodeFlags a, TimeCodeFlags b) noexcept {
	return static_cast<TimeCodeFlags>(static_cast<std::underlying_type<TimeCodeFlags>::type>(a) | static_cast<std::underlying_type<TimeCodeFlags>::type>(b));
}
constexpr inline TimeCodeFlags operator&(TimeCodeFlags a, TimeCodeFlags b) noexcept {
	return static_cast<TimeCodeFlags>(static_cast<std::underlying_type<TimeCodeFlags>::type>(a) & static_cast<std::underlying_type<TimeCodeFlags>::type>(b));
}
constexpr inline TimeCodeFlags operator^(TimeCodeFlags a, TimeCodeFlags b) noexcept {
	return static_cast<TimeCodeFlags>(static_cast<std::underlying_type<TimeCodeFlags>::type>(a) ^ static_cast<std::underlying_type<TimeCodeFlags>::type>(b));
}
constexpr inline TimeCodeFlags operator~(TimeCodeFlags a) noexcept {
	return static_cast<TimeCodeFlags>(~static_cast<std::underlying_type<TimeCodeFlags>::type>(a));
}
constexpr inline TimeCodeFlags & operator|=(TimeCodeFlags & a, TimeCodeFlags b) noexcept {
	return a = a | b;
}
constexpr inline TimeCodeFlags & operator&=(TimeCodeFlags & a, TimeCodeFlags b) noexcept {
	return a = a & b;
}
constexpr inline TimeCodeFlags & operator^=(TimeCodeFlags & a, TimeCodeFlags b) noexcept {
	return a = a ^ b;
}
static_assert(sizeof(TimeCodeFlags) == SizeOfLong);

#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(push, 4)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
struct TimeCode {
	Double        speed           = 0.0;
	HiLoLongLong  timeCodeSamples = 0;
	TimeCodeFlags flags           = static_cast<TimeCodeFlags>(0);
	Padding1      future[64]      = {0};
};
#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
static_assert(sizeof(TimeCode) == (SizeOfDouble + SizeOfLongLong + SizeOfLong + 64));

enum TimeInfoFlags : ULong {
	TimeInfoFlagSystemTimeValid     = 1 << 0,
	TimeInfoFlagSamplePositionValid = 1 << 1,
	TimeInfoFlagSampleRateValid     = 1 << 2,
	TimeInfoFlagSpeedValid          = 1 << 3,
	TimeInfoFlagSampleRateChanged   = 1 << 4,
	TimeInfoFlagClockSourceChanged  = 1 << 5,
};
constexpr inline TimeInfoFlags operator|(TimeInfoFlags a, TimeInfoFlags b) noexcept {
	return static_cast<TimeInfoFlags>(static_cast<std::underlying_type<TimeInfoFlags>::type>(a) | static_cast<std::underlying_type<TimeInfoFlags>::type>(b));
}
constexpr inline TimeInfoFlags operator&(TimeInfoFlags a, TimeInfoFlags b) noexcept {
	return static_cast<TimeInfoFlags>(static_cast<std::underlying_type<TimeInfoFlags>::type>(a) & static_cast<std::underlying_type<TimeInfoFlags>::type>(b));
}
constexpr inline TimeInfoFlags operator^(TimeInfoFlags a, TimeInfoFlags b) noexcept {
	return static_cast<TimeInfoFlags>(static_cast<std::underlying_type<TimeInfoFlags>::type>(a) ^ static_cast<std::underlying_type<TimeInfoFlags>::type>(b));
}
constexpr inline TimeInfoFlags operator~(TimeInfoFlags a) noexcept {
	return static_cast<TimeInfoFlags>(~static_cast<std::underlying_type<TimeInfoFlags>::type>(a));
}
constexpr inline TimeInfoFlags & operator|=(TimeInfoFlags & a, TimeInfoFlags b) noexcept {
	return a = a | b;
}
constexpr inline TimeInfoFlags & operator&=(TimeInfoFlags & a, TimeInfoFlags b) noexcept {
	return a = a & b;
}
constexpr inline TimeInfoFlags & operator^=(TimeInfoFlags & a, TimeInfoFlags b) noexcept {
	return a = a ^ b;
}
static_assert(sizeof(TimeInfoFlags) == SizeOfLong);

#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(push, 4)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
struct TimeInfo {
	Double        speed          = 0.0;
	HiLoLongLong  systemTime     = 0;
	HiLoLongLong  samplePosition = 0;
	SampleRate    sampleRate     = 0.0;
	TimeInfoFlags flags          = static_cast<TimeInfoFlags>(0);
	Padding1      reserved[12]   = {0};
};
#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
static_assert(sizeof(TimeInfo) == (SizeOfDouble + SizeOfLongLong + SizeOfLongLong + SizeOfDouble + SizeOfLong + 12));

#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(push, 4)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
struct Time {
	PaddingLong reserved[4] = {0};
	TimeInfo    timeInfo;
	TimeCode    timeCode;
};
#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
static_assert(sizeof(Time) == (4 * SizeOfLong + sizeof(TimeInfo) + sizeof(TimeCode)));

enum class MessageSelector : Long {
	SelectorSupported    = 1,
	EngineVersion        = 2,
	ResetRequest         = 3,
	BufferSizeChange     = 4,
	ResyncRequest        = 5,
	LatenciesChanged     = 6,
	SupportsTimeInfo     = 7,
	SupportsTimeCode     = 8,
	MMCCommand           = 9,
	SupportsInputMonitor = 10,
	SupportsInputGain    = 11,
	SupportsInputMeter   = 12,
	SupportsOutputGain   = 13,
	SupportsOutputMeter  = 14,
	Overload             = 15,
};
static_assert(sizeof(MessageSelector) == SizeOfLong);

#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(push, 4)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#if ASIO_SYSTEM_WINDOWS && (ASIO_COMPILER_GCC || ASIO_COMPILER_CLANG)
#pragma push_macro("cdecl")
#ifdef cdecl
#undef cdecl
#endif
#endif // ASIO_SYSTEM_WINDOWS && (ASIO_COMPILER_GCC || ASIO_COMPILER_CLANG)
struct Callbacks {
	void(ASIO_CALL * bufferSwitch ASIO_ATTR_CALL)(Long doubleBufferIndex, Bool directProcess) noexcept                                      = nullptr;
	void(ASIO_CALL * sampleRateDidChange ASIO_ATTR_CALL)(SampleRate sRate) noexcept                                                         = nullptr;
	Long(ASIO_CALL * asioMessage ASIO_ATTR_CALL)(MessageSelector selector, Long value, const void * message, const Double * opt) noexcept   = nullptr;
	const Time *(ASIO_CALL * bufferSwitchTimeInfo ASIO_ATTR_CALL)(const Time * params, Long doubleBufferIndex, Bool directProcess) noexcept = nullptr;
};
#if ASIO_SYSTEM_WINDOWS && (ASIO_COMPILER_GCC || ASIO_COMPILER_CLANG)
#pragma pop_macro("cdecl")
#endif // ASIO_SYSTEM_WINDOWS && (ASIO_COMPILER_GCC || ASIO_COMPILER_CLANG)
#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
static_assert(sizeof(Callbacks) == (4 * sizeof(void (*)(void))));

#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(push, 4)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
struct ClockSource {
	Long        index             = 0;
	Long        associatedChannel = 0;
	Long        associatedGroup   = 0;
	Bool        isCurrentSource   = false;
	CharBuf<32> name              = nullptr;
};
#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
static_assert(sizeof(ClockSource) == (SizeOfLong + SizeOfLong + SizeOfLong + SizeOfBool + 32));

#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(push, 4)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
struct ChannelInfo {
	Long        channel      = 0;
	Bool        isInput      = false;
	Bool        isActive     = false;
	Long        channelGroup = 0;
	SampleType  type         = static_cast<SampleType>(0);
	CharBuf<32> name         = nullptr;
};
#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
static_assert(sizeof(ChannelInfo) == (SizeOfLong + SizeOfBool + SizeOfBool + SizeOfLong + sizeof(SampleType) + 32));

#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(push, 4)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
struct BufferInfo {
	Bool   isInput    = false;
	Long   channelNum = 0;
	void * buffers[2] = {nullptr, nullptr};
};
#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
static_assert(sizeof(BufferInfo) == (SizeOfBool + SizeOfLong + 2 * sizeof(void *)));

enum class FutureSelector : Long {
	EnableTimeCodeRead       = 1,
	DisableTimeCodeRead      = 2,
	SetInputMonitor          = 3,
	Transport                = 4,
	SetInputGain             = 5,
	GetInputMeter            = 6,
	SetOutputGain            = 7,
	GetOutputMeter           = 8,
	CanInputMonitor          = 9,
	CanTimeInfo              = 10,
	CanTimeCode              = 11,
	CanTransport             = 12,
	CanInputGain             = 13,
	CanInputMeter            = 14,
	CanOutputGain            = 15,
	CanOutputMeter           = 16,
	OptionalOne              = 17,
	SetIoFormat              = 0x23111961,
	GetIoFormat              = 0x23111983,
	CanDoIoFormat            = 0x23112004,
	CanReportOverload        = 0x24042012,
	GetInternalBufferSamples = 0x25042012,
};
static_assert(sizeof(FutureSelector) == SizeOfLong);

#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(push, 4)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
struct InputMonitor {
	Long input  = 0;
	Long output = 0;
	Long gain   = 0;
	Bool state  = false;
	Long pan    = 0;
};
#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
static_assert(sizeof(InputMonitor) == (SizeOfLong + SizeOfLong + SizeOfLong + SizeOfBool + SizeOfLong));

#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(push, 4)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
struct ChannelControls {
	Long     channel    = 0;
	Bool     isInput    = false;
	Long     gain       = 0;
	Long     meter      = 0;
	Padding1 future[32] = {0};
};
#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
static_assert(sizeof(ChannelControls) == (SizeOfLong + SizeOfBool + SizeOfLong + SizeOfLong + 32));

enum class TransportCommand : Long {
	Start      = 1,
	Stop       = 2,
	Locate     = 3,
	PunchIn    = 4,
	PunchOut   = 5,
	ArmOn      = 6,
	ArmOff     = 7,
	MonitorOn  = 8,
	MonitorOff = 9,
	Arm        = 10,
	Monitor    = 11,
};
static_assert(sizeof(TransportCommand) == SizeOfLong);

#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(push, 4)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
struct TransportParameters {
	TransportCommand command           = static_cast<TransportCommand>(0);
	HiLoLongLong     samplePosition    = 0;
	Long             track             = 0;
	Byte             trackSwitches[64] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	Padding1         future[64]        = {0};
};
#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
static_assert(sizeof(TransportParameters) == (sizeof(TransportCommand) + SizeOfLongLong + SizeOfLong + 64 + 64));

enum class IoFormatType : Long {
	Invalid = -1,
	PCM     = 0,
	DSD     = 1,
};
static_assert(sizeof(IoFormatType) == SizeOfLong);

#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(push, 4)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
struct IoFormat {
	IoFormatType FormatType                         = static_cast<IoFormatType>(0);
	Padding1     future[512 - sizeof(IoFormatType)] = {0};
};
#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
static_assert(sizeof(IoFormat) == 512);

#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(push, 4)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
struct InternalBufferInfo {
	Long inputSamples  = 0;
	Long outputSamples = 0;
};
#if ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
#pragma pack(pop)
#endif // ASIO_SYSTEM_WINDOWS && ASIO_HAVE_PRAGMA_PACK
static_assert(sizeof(InternalBufferInfo) == (SizeOfLong + SizeOfLong));



typedef ASIO_INTERFACE ISystemDriver ISystemDriver;

using DriverName   = CharBuf<32>;
using ErrorMessage = CharBuf<124>;

#if ASIO_SYSTEM_WINDOWS && (ASIO_COMPILER_GCC || ASIO_COMPILER_CLANG)
#pragma push_macro("thiscall")
#ifdef thiscall
#undef thiscall
#endif
#endif // ASIO_SYSTEM_WINDOWS && (ASIO_COMPILER_GCC || ASIO_COMPILER_CLANG)
ASIO_INTERFACE ISystemDriver : public IUnknown {
	[[nodiscard]] virtual ResultBool ASIO_DRIVERCALL init             ASIO_ATTR_DRIVERCALL(SysHandle sysHandle)                                                                      = 0;
	virtual void ASIO_DRIVERCALL getDriverName                        ASIO_ATTR_DRIVERCALL(DriverName * name)                                                                        = 0;
	[[nodiscard]] virtual Long ASIO_DRIVERCALL getDriverVersion       ASIO_ATTR_DRIVERCALL()                                                                                         = 0;
	virtual void ASIO_DRIVERCALL getErrorMessage                      ASIO_ATTR_DRIVERCALL(ErrorMessage * string)                                                                    = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL start             ASIO_ATTR_DRIVERCALL()                                                                                         = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL stop              ASIO_ATTR_DRIVERCALL()                                                                                         = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL getChannels       ASIO_ATTR_DRIVERCALL(Long * numInputChannels, Long * numOutputChannels)                                        = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL getLatencies      ASIO_ATTR_DRIVERCALL(Long * inputLatency, Long * outputLatency)                                                = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL getBufferSize     ASIO_ATTR_DRIVERCALL(Long * minSize, Long * maxSize, Long * preferredSize, Long * granularity)                 = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL canSampleRate     ASIO_ATTR_DRIVERCALL(SampleRate sampleRate)                                                                    = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL getSampleRate     ASIO_ATTR_DRIVERCALL(SampleRate * sampleRate)                                                                  = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL setSampleRate     ASIO_ATTR_DRIVERCALL(SampleRate sampleRate)                                                                    = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL getClockSources   ASIO_ATTR_DRIVERCALL(ClockSource * clocks, Long * numSources)                                                  = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL setClockSource    ASIO_ATTR_DRIVERCALL(Long reference)                                                                           = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL getSamplePosition ASIO_ATTR_DRIVERCALL(HiLoLongLong * samplePosition, HiLoLongLong * timeStamp)                                  = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL getChannelInfo    ASIO_ATTR_DRIVERCALL(ChannelInfo * info)                                                                       = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL createBuffers     ASIO_ATTR_DRIVERCALL(BufferInfo * bufferInfos, Long numChannels, Long bufferSize, const Callbacks * callbacks) = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL disposeBuffers    ASIO_ATTR_DRIVERCALL()                                                                                         = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL controlPanel      ASIO_ATTR_DRIVERCALL()                                                                                         = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL future            ASIO_ATTR_DRIVERCALL(FutureSelector selector, void * opt)                                                      = 0;
	[[nodiscard]] virtual ErrorCode ASIO_DRIVERCALL outputReady       ASIO_ATTR_DRIVERCALL()                                                                                         = 0;
};
#if ASIO_SYSTEM_WINDOWS && (ASIO_COMPILER_GCC || ASIO_COMPILER_CLANG)
#pragma pop_macro("thiscall")
#endif // ASIO_SYSTEM_WINDOWS && (ASIO_COMPILER_GCC || ASIO_COMPILER_CLANG)



class IDriver {
protected:
	IDriver() = default;

public:
	virtual ~IDriver() noexcept(false) { }

public:
	virtual void                    getDriverName(DriverName * name)                                                                        = 0;
	[[nodiscard]] virtual Long      getDriverVersion()                                                                                      = 0;
	virtual void                    getErrorMessage(ErrorMessage * string)                                                                  = 0;
	[[nodiscard]] virtual ErrorCode start()                                                                                                 = 0;
	[[nodiscard]] virtual ErrorCode stop()                                                                                                  = 0;
	[[nodiscard]] virtual ErrorCode getChannels(Long * numInputChannels, Long * numOutputChannels)                                          = 0;
	[[nodiscard]] virtual ErrorCode getLatencies(Long * inputLatency, Long * outputLatency)                                                 = 0;
	[[nodiscard]] virtual ErrorCode getBufferSize(Long * minSize, Long * maxSize, Long * preferredSize, Long * granularity)                 = 0;
	[[nodiscard]] virtual ErrorCode canSampleRate(SampleRate sampleRate)                                                                    = 0;
	[[nodiscard]] virtual ErrorCode getSampleRate(SampleRate * sampleRate)                                                                  = 0;
	[[nodiscard]] virtual ErrorCode setSampleRate(SampleRate sampleRate)                                                                    = 0;
	[[nodiscard]] virtual ErrorCode getClockSources(ClockSource * clocks, Long * numSources)                                                = 0;
	[[nodiscard]] virtual ErrorCode setClockSource(Long reference)                                                                          = 0;
	[[nodiscard]] virtual ErrorCode getSamplePosition(HiLoLongLong * samplePosition, HiLoLongLong * timeStamp)                              = 0;
	[[nodiscard]] virtual ErrorCode getChannelInfo(ChannelInfo * info)                                                                      = 0;
	[[nodiscard]] virtual ErrorCode createBuffers(BufferInfo * bufferInfos, Long numChannels, Long bufferSize, const Callbacks * callbacks) = 0;
	[[nodiscard]] virtual ErrorCode disposeBuffers()                                                                                        = 0;
	[[nodiscard]] virtual ErrorCode controlPanel()                                                                                          = 0;
	[[nodiscard]] virtual ErrorCode future(FutureSelector selector, void * opt)                                                             = 0;
	[[nodiscard]] virtual ErrorCode outputReady()                                                                                           = 0;
};



} // namespace Core



} // namespace ASIO_VERSION_NAMESPACE



} // namespace ASIO



#endif // ASIO_ASIOCORE_HPP
