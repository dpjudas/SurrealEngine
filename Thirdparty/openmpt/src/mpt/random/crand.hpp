/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_RANDOM_CRAND_HPP
#define MPT_RANDOM_CRAND_HPP



#include "mpt/base/namespace.hpp"
#include "mpt/base/numeric.hpp"
#include "mpt/random/random.hpp"

#include <cstdlib>



namespace mpt {
inline namespace MPT_INLINE_NS {



class crand {
public:
	using state_type = void;
	using result_type = int;

private:
	static void reseed(uint32 seed) {
		std::srand(seed);
	}

public:
	template <typename Trd>
	static void reseed(Trd & rd) {
		reseed(mpt::random<uint32>(rd));
	}

public:
	crand() = default;
	explicit crand(const std::string &) {
		return;
	}

public:
	static MPT_CONSTEXPRINLINE result_type min() {
		return 0;
	}
	static MPT_CONSTEXPRINLINE result_type max() {
		return RAND_MAX;
	}
	static MPT_CONSTEXPRINLINE int result_bits() {
		return mpt::lower_bound_entropy_bits(RAND_MAX);
	}
	result_type operator()() {
		return std::rand();
	}
};


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_RANDOM_CRAND_HPP
