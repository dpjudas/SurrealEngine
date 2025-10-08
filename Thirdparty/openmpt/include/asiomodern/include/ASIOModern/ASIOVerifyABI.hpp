
#ifndef ASIO_ASIOVERIFY_ABI_HPP
#define ASIO_ASIOVERIFY_ABI_HPP



#include "ASIOVersion.hpp"
#include "ASIOConfig.hpp"
#include "ASIOCore.hpp"

#if __has_include(<iasiodrv.h>)
#define ASIO_ABI_VERIFIED 1
#include <iasiodrv.h>
#else
#define ASIO_ABI_VERIFIED 0
ASIO_WARNING("Warning: iasiodrv.h not found. ASIO ABI is not verified.")
#endif



namespace ASIO {



inline namespace ASIO_VERSION_NAMESPACE {



#if ASIO_ABI_VERIFIED



inline namespace VerifyABI {



static_assert(sizeof(ASIO::SysHandle) == sizeof(void *));
static_assert(sizeof(ASIO::Byte) == sizeof(char));
static_assert(sizeof(ASIO::Long) == sizeof(long));
static_assert(sizeof(ASIO::ULong) == sizeof(unsigned long));
static_assert(sizeof(ASIO::ULongLong) == sizeof(unsigned long long int));
static_assert(sizeof(ASIO::Double) == sizeof(double));
static_assert(sizeof(ASIO::Char) == sizeof(char));
static_assert(sizeof(ASIO::Padding1) == sizeof(char));
static_assert(sizeof(ASIO::PaddingLong) == sizeof(long));
static_assert(sizeof(ASIO::Bool) == sizeof(long));
static_assert(sizeof(ASIO::HiLoLongLong) == sizeof(long long int));
static_assert(sizeof(ASIO::ResultBool) == sizeof(long));
static_assert(sizeof(ASIO::CharBuf<1>) == sizeof(char[1]));
static_assert(sizeof(ASIO::Samples) == sizeof(::ASIOSamples));
static_assert(sizeof(ASIO::TimeStamp) == sizeof(::ASIOTimeStamp));
static_assert(sizeof(ASIO::SampleRate) == sizeof(::ASIOSampleRate));
static_assert(sizeof(ASIO::SampleType) == sizeof(::ASIOSampleType));
static_assert(sizeof(ASIO::ErrorCode) == sizeof(::ASIOError));
static_assert(sizeof(ASIO::TimeCodeFlags) == sizeof(::ASIOTimeCodeFlags));
static_assert(sizeof(ASIO::TimeCode) == sizeof(::ASIOTimeCode));
static_assert(sizeof(ASIO::TimeInfoFlags) == sizeof(::AsioTimeInfoFlags));
static_assert(sizeof(ASIO::TimeInfo) == sizeof(::AsioTimeInfo));
static_assert(sizeof(ASIO::Time) == sizeof(::ASIOTime));
static_assert(sizeof(ASIO::MessageSelector) == sizeof(long));
static_assert(sizeof(ASIO::Callbacks) == sizeof(::ASIOCallbacks));
static_assert(sizeof(ASIO::ClockSource) == sizeof(::ASIOClockSource));
static_assert(sizeof(ASIO::ChannelInfo) == sizeof(::ASIOChannelInfo));
static_assert(sizeof(ASIO::BufferInfo) == sizeof(::ASIOBufferInfo));
static_assert(sizeof(ASIO::FutureSelector) == sizeof(long));
static_assert(sizeof(ASIO::InputMonitor) == sizeof(::ASIOInputMonitor));
static_assert(sizeof(ASIO::ChannelControls) == sizeof(::ASIOChannelControls));
static_assert(sizeof(ASIO::TransportCommand) == sizeof(long));
static_assert(sizeof(ASIO::TransportParameters) == sizeof(::ASIOTransportParameters));
static_assert(sizeof(ASIO::IoFormatType) == sizeof(::ASIOIoFormatType));
static_assert(sizeof(ASIO::IoFormat) == sizeof(::ASIOIoFormat));
static_assert(sizeof(ASIO::InternalBufferInfo) == sizeof(::ASIOInternalBufferInfo));
static_assert(sizeof(ASIO::ISystemDriver) == sizeof(::IASIO));

static_assert(alignof(ASIO::SysHandle) == alignof(void *));
static_assert(alignof(ASIO::Byte) == alignof(char));
static_assert(alignof(ASIO::Long) == alignof(long));
static_assert(alignof(ASIO::ULong) == alignof(unsigned long));
static_assert(alignof(ASIO::ULongLong) == alignof(unsigned long long int));
static_assert(alignof(ASIO::Double) == alignof(double));
static_assert(alignof(ASIO::Char) == alignof(char));
static_assert(alignof(ASIO::Padding1) == alignof(char));
static_assert(alignof(ASIO::PaddingLong) == alignof(long));
static_assert(alignof(ASIO::Bool) == alignof(long));
static_assert((NATIVE_INT64 && (alignof(ASIO::HiLoLongLong) == alignof(long long int))) || (!NATIVE_INT64 && (alignof(ASIO::HiLoLongLong) == alignof(unsigned long[2]))));
static_assert(alignof(ASIO::ResultBool) == alignof(long));
static_assert(alignof(ASIO::CharBuf<1>) == alignof(char[1]));
static_assert(alignof(ASIO::Samples) == alignof(::ASIOSamples));
static_assert(alignof(ASIO::TimeStamp) == alignof(::ASIOTimeStamp));
static_assert(alignof(ASIO::SampleRate) == alignof(::ASIOSampleRate));
static_assert(alignof(ASIO::SampleType) == alignof(::ASIOSampleType));
static_assert(alignof(ASIO::ErrorCode) == alignof(::ASIOError));
static_assert(alignof(ASIO::TimeCodeFlags) == alignof(::ASIOTimeCodeFlags));
static_assert(alignof(ASIO::TimeCode) == alignof(::ASIOTimeCode));
static_assert(alignof(ASIO::TimeInfoFlags) == alignof(::AsioTimeInfoFlags));
static_assert(alignof(ASIO::TimeInfo) == alignof(::AsioTimeInfo));
static_assert(alignof(ASIO::Time) == alignof(::ASIOTime));
static_assert(alignof(ASIO::MessageSelector) == alignof(long));
static_assert(alignof(ASIO::Callbacks) == alignof(::ASIOCallbacks));
static_assert(alignof(ASIO::ClockSource) == alignof(::ASIOClockSource));
static_assert(alignof(ASIO::ChannelInfo) == alignof(::ASIOChannelInfo));
static_assert(alignof(ASIO::BufferInfo) == alignof(::ASIOBufferInfo));
static_assert(alignof(ASIO::FutureSelector) == alignof(long));
static_assert(alignof(ASIO::InputMonitor) == alignof(::ASIOInputMonitor));
static_assert(alignof(ASIO::ChannelControls) == alignof(::ASIOChannelControls));
static_assert(alignof(ASIO::TransportCommand) == alignof(long));
static_assert(alignof(ASIO::TransportParameters) == alignof(::ASIOTransportParameters));
static_assert(alignof(ASIO::IoFormatType) == alignof(::ASIOIoFormatType));
static_assert(alignof(ASIO::IoFormat) == alignof(::ASIOIoFormat));
static_assert(alignof(ASIO::InternalBufferInfo) == alignof(::ASIOInternalBufferInfo));
static_assert(alignof(ASIO::ISystemDriver) == alignof(::IASIO));



} // namespace VerifyABI



#endif // ASIO_ABI_VERIFIED



} // namespace ASIO_VERSION_NAMESPACE



} // namespace ASIO



#endif // ASIO_ASIOVERIFY_ABI_HPP
