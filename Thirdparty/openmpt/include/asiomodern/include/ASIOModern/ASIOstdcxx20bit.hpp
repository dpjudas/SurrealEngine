
#ifndef ASIO_ASIOSTDCXX20BIT_HPP
#define ASIO_ASIOSTDCXX20BIT_HPP



#include "ASIOVersion.hpp"
#include "ASIOConfig.hpp"



#if (ASIO_CXX_VERSION >= 202002L)
#include <bit>
namespace ASIO {
inline namespace ASIO_VERSION_NAMESPACE {
namespace stdcxx20 {
using std::bit_cast;
using std::endian;
} // namespace stdcxx20
} // namespace ASIO_VERSION_NAMESPACE
} // namespace ASIO
#else // !C++20
#include <type_traits>
#include <cstring>
namespace ASIO {
inline namespace ASIO_VERSION_NAMESPACE {
namespace stdcxx20 {
template <class To, class From>
typename inline std::enable_if<(sizeof(To) == sizeof(From)) && std::is_trivially_copyable<From>::value && std::is_trivial<To>::value, To>::type bit_cast(const From & src) noexcept {
	To dst;
	std::memcpy(&dst, &src, sizeof(To));
	return dst;
}
enum class endian {
#if ASIO_SYSTEM_WINDOWS
	little = 0,
	big    = 1,
	native = little,
#elif ASIO_COMPILER_CLANG || ASIO_COMPILER_GCC
	little = __ORDER_LITTLE_ENDIAN__,
	big    = __ORDER_BIG_ENDIAN__,
	native = __BYTE_ORDER__,
#endif
};
} // namespace stdcxx20
} // namespace ASIO_VERSION_NAMESPACE
} // namespace ASIO
#endif // C++20



#endif // ASIO_ASIOSTDCXX20BIT_HPP
