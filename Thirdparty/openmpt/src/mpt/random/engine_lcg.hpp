/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_RANDOM_ENGINE_LCG_HPP
#define MPT_RANDOM_ENGINE_LCG_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/numeric.hpp"

#include <array>
#include <random>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {


#if MPT_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4724) // potential mod by 0
#endif                          // MPT_COMPILER_MSVC

template <typename Tstate, typename Tvalue, Tstate m, Tstate a, Tstate c, Tstate result_mask, int result_shift, int result_bits_>
class lcg_engine {
public:
	typedef Tstate state_type;
	typedef Tvalue result_type;
	static inline constexpr std::size_t seed_bits = sizeof(state_type) * 8;

private:
	state_type state;

private:
	static inline state_type seed_state(std::seed_seq & seed) {
		state_type result = 0;
		std::array<unsigned int, mpt::align_up<std::size_t>(seed_bits, sizeof(unsigned int) * 8) / (sizeof(unsigned int) * 8)> seeds = {};
		seed.generate(seeds.begin(), seeds.end());
		for (const auto & seed_value : seeds) {
			result <<= 16;
			result <<= 16;
			result |= static_cast<state_type>(seed_value);
		}
		return result;
	}

public:
	explicit inline lcg_engine(std::seed_seq & seed)
		: state(seed_state(seed)) {
		operator()(); // we return results from the current state and update state after returning. results in better pipelining.
	}
	explicit inline lcg_engine(state_type seed)
		: state(seed) {
		operator()(); // we return results from the current state and update state after returning. results in better pipelining.
	}

public:
	static MPT_CONSTEXPRINLINE result_type min() {
		return static_cast<result_type>(0);
	}
	static MPT_CONSTEXPRINLINE result_type max() {
		static_assert(((result_mask >> result_shift) << result_shift) == result_mask);
		return static_cast<result_type>(result_mask >> result_shift);
	}
	static MPT_CONSTEXPRINLINE int result_bits() {
		static_assert(((static_cast<Tstate>(1) << result_bits_) - 1) == (result_mask >> result_shift));
		return result_bits_;
	}
	inline result_type operator()() {
		// we return results from the current state and update state after returning. results in better pipelining.
		state_type s = state;
		result_type result = static_cast<result_type>((s & result_mask) >> result_shift);
		s = mpt::modulo_if_not_zero<state_type, m>((a * s) + c);
		state = s;
		return result;
	}
};

#if MPT_COMPILER_MSVC
#pragma warning(pop)
#endif // MPT_COMPILER_MSVC

typedef lcg_engine<uint32, uint16, 0u, 214013u, 2531011u, 0x7fff0000u, 16, 15> lcg_msvc;
typedef lcg_engine<uint32, uint16, 0x80000000u, 1103515245u, 12345u, 0x7fff0000u, 16, 15> lcg_c99;
typedef lcg_engine<uint64, uint32, 0ull, 6364136223846793005ull, 1ull, 0xffffffff00000000ull, 32, 32> lcg_musl;



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_RANDOM_ENGINE_LCG_HPP
