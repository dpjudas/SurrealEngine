/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_RANDOM_SEED_HPP
#define MPT_RANDOM_SEED_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/base/numeric.hpp"
#include "mpt/random/engine.hpp"
#include "mpt/random/random.hpp"

#include <array>
#include <memory>
#include <random>

#include <cstddef>


namespace mpt {
inline namespace MPT_INLINE_NS {



template <std::size_t N>
class seed_seq_values {
private:
	std::array<unsigned int, N> seeds;

public:
	template <typename Trd>
	explicit seed_seq_values(Trd & rd) {
		for (std::size_t i = 0; i < N; ++i) {
			seeds[i] = mpt::random<unsigned int>(rd);
		}
	}
	const unsigned int * begin() const {
		return seeds.data();
	}
	const unsigned int * end() const {
		return seeds.data() + N;
	}
};



template <typename Trng, typename Trd>
inline Trng make_prng(Trd & rd) {
	constexpr std::size_t num_seed_values = mpt::align_up<std::size_t>(mpt::engine_seed_traits<Trng>::seed_bits, sizeof(unsigned int) * 8) / (sizeof(unsigned int) * 8);
	if constexpr (num_seed_values > 128) {
		std::unique_ptr<mpt::seed_seq_values<num_seed_values>> values = std::make_unique<mpt::seed_seq_values<num_seed_values>>(rd);
		std::seed_seq seed(values.begin(), values.end());
		return Trng(seed);
	} else {
		mpt::seed_seq_values<num_seed_values> values(rd);
		std::seed_seq seed(values.begin(), values.end());
		return Trng(seed);
	}
}



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_RANDOM_SEED_HPP
