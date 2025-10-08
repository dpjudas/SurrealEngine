/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_AUDIO_SAMPLE_HPP
#define MPT_AUDIO_SAMPLE_HPP



#include "mpt/base/float.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/namespace.hpp"

#include <type_traits>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



using audio_sample_int = int16;
using audio_sample_float = nativefloat;

using audio_sample = std::conditional<mpt::float_traits<audio_sample_float>::is_hard, audio_sample_float, audio_sample_int>::type;



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BINARY_HEX_HPP
