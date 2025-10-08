/*
 * mptRandom.h
 * -----------
 * Purpose: PRNG
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/bit.hpp"
#include "mpt/mutex/mutex.hpp"
#ifdef MODPLUG_TRACKER
#include "mpt/random/crand.hpp"
#endif // MODPLUG_TRACKER
#include "mpt/random/default_engines.hpp"
#include "mpt/random/device.hpp"
#include "mpt/random/engine.hpp"
#include "mpt/random/engine_lcg.hpp"
#include "mpt/random/seed.hpp"

#include <limits>
#include <random>



OPENMPT_NAMESPACE_BEGIN


// NOTE:
//  We implement our own PRNG and distribution functions as the implementations
// of std::uniform_int_distribution is either wrong (not uniform in MSVC2010) or
// not guaranteed to be livelock-free for bad PRNGs (in GCC, Clang, boost).
// We resort to a simpler implementation with only power-of-2 result ranges for
// both the underlying PRNG and our interface function. This saves us from
// complicated code having to deal with partial bits of entropy.
//  Our interface still somewhat follows the mindset of C++11 <random> (with the
// addition of a simple wrapper function mpt::random which saves the caller from
// instantiating distribution objects for the common uniform distribution case.
//  We are still using std::random_device for initial seeding when avalable and
// after working around its set of problems.


namespace mpt
{



template <typename Trng>
class thread_safe_prng
	: private Trng
{
private:
	mpt::mutex m;
public:
	using result_type = typename Trng::result_type;
public:
	template <typename Trd>
	explicit thread_safe_prng(Trd & rd)
		: Trng(mpt::make_prng<Trng>(rd))
	{
		return;
	}
	thread_safe_prng(Trng rng)
		: Trng(rng)
	{
		return;
	}
public:
	static MPT_CONSTEXPRINLINE typename engine_traits<Trng>::result_type min()
	{
		return Trng::min();
	}
	static MPT_CONSTEXPRINLINE typename engine_traits<Trng>::result_type max()
	{
		return Trng::max();
	}
	static MPT_CONSTEXPRINLINE int result_bits()
	{
		return engine_traits<Trng>::result_bits();
	}
public:
	typename engine_traits<Trng>::result_type operator()()
	{
		mpt::lock_guard<mpt::mutex> l(m);
		return Trng::operator()();
	}
};


#ifdef MPT_BUILD_FUZZER

//  Use deterministic seeding
using random_device = deterministic_random_device;

#else // !MPT_BUILD_FUZZER

// mpt::random_device always generates 32 bits of entropy
using random_device = mpt::sane_random_device;

#endif // MPT_BUILD_FUZZER


#ifdef MPT_BUILD_FUZZER

// Use fast PRNGs in order to not waste time fuzzing more complex PRNG
// implementations.
using fast_prng = deterministic_fast_engine;
using good_prng = deterministic_good_engine;

#else  // !MPT_BUILD_FUZZER

// We cannot use std::minstd_rand here because it has not a power-of-2 sized
// output domain which we rely upon.
using fast_prng = fast_engine;  // about 3 ALU operations, ~32bit of state, suited for inner loops
using good_prng = good_engine;

#endif  // MPT_BUILD_FUZZER


using default_prng = mpt::good_prng;


mpt::random_device & global_random_device();
mpt::thread_safe_prng<mpt::default_prng> & global_prng();

#ifdef MPT_BUILD_FUZZER
void reinit_global_random();
#endif  // MPT_BUILD_FUZZER

#if defined(MODPLUG_TRACKER) && !defined(MPT_BUILD_WINESUPPORT)
void set_global_random_device(mpt::random_device *rd);
void set_global_prng(mpt::thread_safe_prng<mpt::default_prng> *rng);
#endif // MODPLUG_TRACKER && !MPT_BUILD_WINESUPPORT


} // namespace mpt


OPENMPT_NAMESPACE_END
