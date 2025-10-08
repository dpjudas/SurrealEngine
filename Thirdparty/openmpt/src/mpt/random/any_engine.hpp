/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_RANDOM_ANY_ENGINE_HPP
#define MPT_RANDOM_ANY_ENGINE_HPP



#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/random/random.hpp"

#include <limits>
#include <memory>

#include <cstddef>


namespace mpt {
inline namespace MPT_INLINE_NS {


template <typename Tvalue>
class any_engine {
public:
	using result_type = Tvalue;
protected:
	any_engine() = default;
public:
	static MPT_CONSTEXPRINLINE result_type min() {
		return static_cast<result_type>(std::numeric_limits<result_type>::min());
	}
	static MPT_CONSTEXPRINLINE result_type max() {
		return static_cast<result_type>(std::numeric_limits<result_type>::max());
	}
	static MPT_CONSTEXPRINLINE int result_bits() {
		return static_cast<int>(sizeof(result_type) * 8);
	}
public:
	virtual result_type operator()() = 0;
public:
	virtual ~any_engine() = default;
};


template <typename Trng, typename Tvalue = typename Trng::result_type>
class any_engine_wrapper : public any_engine<Tvalue> {
private:
	Trng & m_prng;
public:
	any_engine_wrapper(Trng & prng)
		: m_prng(prng) {
		return;
	}
public:
	typename any_engine<Tvalue>::result_type operator()() override {
		return mpt::random<typename any_engine<Tvalue>::result_type>(m_prng);
	}
public:
	virtual ~any_engine_wrapper() = default;
};


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_RANDOM_ANY_ENGINE_HPP
