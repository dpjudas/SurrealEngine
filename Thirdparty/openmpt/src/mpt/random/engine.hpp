/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_RANDOM_ENGINE_HPP
#define MPT_RANDOM_ENGINE_HPP



#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"

#include <random>

#include <cstddef>


namespace mpt {
inline namespace MPT_INLINE_NS {


template <typename Trng>
struct engine_seed_traits {
	static inline constexpr std::size_t seed_bits = Trng::seed_bits;
};

template <typename Trng>
struct engine_traits {
	using result_type = typename Trng::result_type;
	static MPT_CONSTEXPRINLINE int result_bits() {
		return Trng::result_bits();
	}
};

// C++11 random does not provide any sane way to determine the amount of entropy
// required to seed a particular engine. VERY STUPID.
// List the ones we are likely to use.

template <>
struct engine_seed_traits<std::mt19937> {
	static inline constexpr std::size_t seed_bits = sizeof(std::mt19937::result_type) * 8 * std::mt19937::state_size;
};

template <>
struct engine_traits<std::mt19937> {
	using rng_type = std::mt19937;
	using result_type = rng_type::result_type;
	static MPT_CONSTEXPRINLINE int result_bits() {
		return rng_type::word_size;
	}
};

template <>
struct engine_seed_traits<std::mt19937_64> {
	static inline constexpr std::size_t seed_bits = sizeof(std::mt19937_64::result_type) * 8 * std::mt19937_64::state_size;
};

template <>
struct engine_traits<std::mt19937_64> {
	using rng_type = std::mt19937_64;
	using result_type = rng_type::result_type;
	static MPT_CONSTEXPRINLINE int result_bits() {
		return rng_type::word_size;
	}
};

template <>
struct engine_seed_traits<std::ranlux24_base> {
	static inline constexpr std::size_t seed_bits = std::ranlux24_base::word_size;
};

template <>
struct engine_traits<std::ranlux24_base> {
	using rng_type = std::ranlux24_base;
	using result_type = rng_type::result_type;
	static MPT_CONSTEXPRINLINE int result_bits() {
		return rng_type::word_size;
	}
};

template <>
struct engine_seed_traits<std::ranlux48_base> {
	static inline constexpr std::size_t seed_bits = std::ranlux48_base::word_size;
};

template <>
struct engine_traits<std::ranlux48_base> {
	using rng_type = std::ranlux48_base;
	using result_type = rng_type::result_type;
	static MPT_CONSTEXPRINLINE int result_bits() {
		return rng_type::word_size;
	}
};

template <>
struct engine_seed_traits<std::ranlux24> {
	static inline constexpr std::size_t seed_bits = std::ranlux24_base::word_size;
};

template <>
struct engine_traits<std::ranlux24> {
	using rng_type = std::ranlux24;
	using result_type = rng_type::result_type;
	static MPT_CONSTEXPRINLINE int result_bits() {
		return std::ranlux24_base::word_size;
	}
};

template <>
struct engine_seed_traits<std::ranlux48> {
	static inline constexpr std::size_t seed_bits = std::ranlux48_base::word_size;
};

template <>
struct engine_traits<std::ranlux48> {
	using rng_type = std::ranlux48;
	using result_type = rng_type::result_type;
	static MPT_CONSTEXPRINLINE int result_bits() {
		return std::ranlux48_base::word_size;
	}
};



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_RANDOM_ENGINE_HPP
