/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_RANDOM_DEFAULT_ENGINES_HPP
#define MPT_RANDOM_DEFAULT_ENGINES_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/random/engine.hpp"
#include "mpt/random/engine_lcg.hpp"

#include <random>



namespace mpt {
inline namespace MPT_INLINE_NS {


using deterministic_fast_engine = mpt::lcg_msvc;
using deterministic_good_engine = mpt::lcg_musl;

// We cannot use std::minstd_rand here because it has not a power-of-2 sized
// output domain which we rely upon.
using fast_engine = mpt::lcg_msvc; // about 3 ALU operations, ~32bit of state, suited for inner loops
using good_engine = std::ranlux48;



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_RANDOM_DEFAULT_ENGINES_HPP
