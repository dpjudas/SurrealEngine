/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_RANDOM_DEVICE_HPP
#define MPT_RANDOM_DEVICE_HPP



#include "mpt/base/bit.hpp"
#include "mpt/base/detect.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/math.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/crc/crc.hpp"
#include "mpt/endian/integer.hpp"
#include "mpt/mutex/mutex.hpp"
#include "mpt/out_of_memory/out_of_memory.hpp"
#include "mpt/random/engine.hpp"
#include "mpt/random/engine_lcg.hpp"
#include "mpt/random/random.hpp"

#if !defined(MPT_LIBCXX_QUIRK_NO_CHRONO)
#include <chrono>
#endif // !MPT_LIBCXX_QUIRK_NO_CHRONO
#include <iterator>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <cmath>
#include <cstring>
#if defined(MPT_LIBCXX_QUIRK_NO_CHRONO)
#include <ctime>
#endif // MPT_LIBCXX_QUIRK_NO_CHRONO



namespace mpt {
inline namespace MPT_INLINE_NS {


inline constexpr uint32 DETERMINISTIC_RNG_SEED = 3141592653u; // pi



template <typename T>
struct default_random_seed_hash {
};

template <>
struct default_random_seed_hash<uint8> {
	using type = mpt::crc16;
};

template <>
struct default_random_seed_hash<uint16> {
	using type = mpt::crc16;
};

template <>
struct default_random_seed_hash<uint32> {
	using type = mpt::crc32c;
};

template <>
struct default_random_seed_hash<uint64> {
	using type = mpt::crc64_jones;
};


class prng_random_device_time_seeder {

public:
	template <typename T>
	inline T generate_seed() {
		// Note: CRC is actually not that good a choice here, but it is simple and we
		// already have an implementaion available. Better choices for mixing entropy
		// would be a hash function with proper avalanche characteristics or a block
		// or stream cipher with any pre-choosen random key and IV. The only aspect we
		// really need here is whitening of the bits.
		typename mpt::default_random_seed_hash<T>::type hash;

#if !defined(MPT_LIBCXX_QUIRK_NO_CHRONO)
		{
			uint64be time;
			time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock().now().time_since_epoch()).count();
			std::byte bytes[sizeof(time)];
			std::memcpy(bytes, &time, sizeof(time));
			hash(std::begin(bytes), std::end(bytes));
		}
#if !defined(MPT_COMPILER_QUIRK_CHRONO_NO_HIGH_RESOLUTION_CLOCK)
		// Avoid std::chrono::high_resolution_clock on Emscripten because availability is problematic in AudioWorklet context.
		{
			uint64be time;
			time = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock().now().time_since_epoch()).count();
			std::byte bytes[sizeof(time)];
			std::memcpy(bytes, &time, sizeof(time));
			hash(std::begin(bytes), std::end(bytes));
		}
#endif // !MPT_COMPILER_QUIRK_CHRONO_NO_HIGH_RESOLUTION_CLOCK
#else  // MPT_LIBCXX_QUIRK_NO_CHRONO
		{
			uint64be time;
			time = static_cast<uint64>(std::time(nullptr));
			std::byte bytes[sizeof(time)];
			std::memcpy(bytes, &time, sizeof(time));
			hash(std::begin(bytes), std::end(bytes));
		}
#endif // !MPT_LIBCXX_QUIRK_NO_CHRONO

		return static_cast<T>(hash.result());
	}

public:
	prng_random_device_time_seeder() = default;
};


//  C++11 std::random_device may be implemented as a deterministic PRNG.
//  There is no way to seed this PRNG and it is allowed to be seeded with the
// same value on each program invocation. This makes std::random_device
// completely useless even as a non-cryptographic entropy pool.
//  We fallback to time-seeded std::mt19937 if std::random_device::entropy() is
// 0 or less.
class sane_random_device {
private:
	mpt::mutex m;
	std::string token;
#if !defined(MPT_COMPILER_QUIRK_RANDOM_NO_RANDOM_DEVICE)
	std::unique_ptr<std::random_device> prd;
#endif // !MPT_COMPILER_QUIRK_RANDOM_NO_RANDOM_DEVICE
	std::unique_ptr<std::mt19937> rd_fallback;

public:
	using result_type = unsigned int;

private:
	void init_fallback() {
		if (!rd_fallback) {
			if (token.length() > 0) {
				uint64 seed_val = mpt::prng_random_device_time_seeder().generate_seed<uint64>();
				std::vector<unsigned int> seeds;
				seeds.push_back(static_cast<uint32>(seed_val >> 32));
				seeds.push_back(static_cast<uint32>(seed_val >> 0));
				for (std::size_t i = 0; i < token.length(); ++i) {
					seeds.push_back(static_cast<unsigned int>(static_cast<unsigned char>(token[i])));
				}
				std::seed_seq seed(seeds.begin(), seeds.end());
				rd_fallback = std::make_unique<std::mt19937>(seed);
			} else {
				uint64 seed_val = mpt::prng_random_device_time_seeder().generate_seed<uint64>();
				unsigned int seeds[2];
				seeds[0] = static_cast<uint32>(seed_val >> 32);
				seeds[1] = static_cast<uint32>(seed_val >> 0);
				std::seed_seq seed(seeds + 0, seeds + 2);
				rd_fallback = std::make_unique<std::mt19937>(seed);
			}
		}
	}

public:
	sane_random_device() {
#if !defined(MPT_COMPILER_QUIRK_RANDOM_NO_RANDOM_DEVICE)
		try {
			prd = std::make_unique<std::random_device>();
			if (!((*prd).entropy() > 0.0)) {
				init_fallback();
			}
		} catch (mpt::out_of_memory e) {
			mpt::rethrow_out_of_memory(e);
		} catch (const std::exception &) {
			init_fallback();
		}
#else  // MPT_COMPILER_QUIRK_RANDOM_NO_RANDOM_DEVICE
		init_fallback();
#endif // !MPT_COMPILER_QUIRK_RANDOM_NO_RANDOM_DEVICE
	}
	sane_random_device(const std::string & token_)
		: token(token_) {
#if !defined(MPT_COMPILER_QUIRK_RANDOM_NO_RANDOM_DEVICE)
		try {
			prd = std::make_unique<std::random_device>(token);
			if (!((*prd).entropy() > 0.0)) {
				init_fallback();
			}
		} catch (mpt::out_of_memory e) {
			mpt::rethrow_out_of_memory(e);
		} catch (const std::exception &) {
			init_fallback();
		}
#else  // MPT_COMPILER_QUIRK_RANDOM_NO_RANDOM_DEVICE
		init_fallback();
#endif // !MPT_COMPILER_QUIRK_RANDOM_NO_RANDOM_DEVICE
	}
	static MPT_CONSTEXPRINLINE result_type min() {
		return std::numeric_limits<result_type>::min();
	}
	static MPT_CONSTEXPRINLINE result_type max() {
		return std::numeric_limits<result_type>::max();
	}
	static MPT_CONSTEXPRINLINE int result_bits() {
		return sizeof(result_type) * 8;
	}
	result_type operator()() {
		mpt::lock_guard<mpt::mutex> l(m);
		result_type result = 0;
#if !defined(MPT_COMPILER_QUIRK_RANDOM_NO_RANDOM_DEVICE)
		if (prd) {
			try {
				if constexpr (std::random_device::min() != 0 || !mpt::is_mask(std::random_device::max())) {
					// insane std::random_device
					//  This implementation is not exactly uniformly distributed but good enough
					// for OpenMPT.
					constexpr double rd_min = static_cast<double>(std::random_device::min());
					constexpr double rd_max = static_cast<double>(std::random_device::max());
					constexpr double rd_range = rd_max - rd_min;
					constexpr double rd_size = rd_range + 1.0;
					const double rd_entropy = mpt::log2(rd_size);
					const int iterations = static_cast<int>(std::ceil(result_bits() / rd_entropy));
					double tmp = 0.0;
					for (int i = 0; i < iterations; ++i) {
						tmp = (tmp * rd_size) + (static_cast<double>((*prd)()) - rd_min);
					}
					double result_01 = std::floor(tmp / std::pow(rd_size, iterations));
					result = static_cast<result_type>(std::floor(result_01 * (static_cast<double>(max() - min()) + 1.0))) + min();
				} else {
					// sane std::random_device
					result = 0;
					std::size_t rd_bits = mpt::lower_bound_entropy_bits(std::random_device::max());
					for (std::size_t entropy = 0; entropy < (sizeof(result_type) * 8); entropy += rd_bits) {
						if (rd_bits < (sizeof(result_type) * 8)) {
							result = (result << rd_bits) | static_cast<result_type>((*prd)());
						} else {
							result = result | static_cast<result_type>((*prd)());
						}
					}
				}
			} catch (const std::exception &) {
				init_fallback();
			}
		}
		if (rd_fallback) {
			// std::random_device is unreliable
			//  XOR the generated random number with more entropy from the time-seeded
			// PRNG.
			//  Note: This is safe even if the std::random_device itself is implemented
			// as a std::mt19937 PRNG because we are very likely using a different
			// seed.
			result ^= mpt::random<result_type>(*rd_fallback);
		}
#else  // MPT_COMPILER_QUIRK_RANDOM_NO_RANDOM_DEVICE
		result ^= mpt::random<result_type>(*rd_fallback);
#endif // !MPT_COMPILER_QUIRK_RANDOM_NO_RANDOM_DEVICE
		return result;
	}
};


class prng_random_device_deterministic_seeder {
protected:
	template <typename T>
	constexpr T generate_seed() noexcept {
		return static_cast<T>(mpt::DETERMINISTIC_RNG_SEED);
	}

protected:
	prng_random_device_deterministic_seeder() = default;
};

template <typename Trng = mpt::lcg_musl, typename seeder = mpt::prng_random_device_time_seeder>
class prng_random_device
	: private seeder {
public:
	using result_type = unsigned int;

private:
	mpt::mutex m;
	Trng rng;

public:
	prng_random_device()
		: rng(seeder::template generate_seed<typename Trng::state_type>()) {
		return;
	}
	prng_random_device(const std::string &)
		: rng(seeder::template generate_seed<typename Trng::state_type>()) {
		return;
	}
	static MPT_CONSTEXPRINLINE result_type min() {
		return std::numeric_limits<unsigned int>::min();
	}
	static MPT_CONSTEXPRINLINE result_type max() {
		return std::numeric_limits<unsigned int>::max();
	}
	static MPT_CONSTEXPRINLINE int result_bits() {
		return sizeof(unsigned int) * 8;
	}
	result_type operator()() {
		mpt::lock_guard<mpt::mutex> l(m);
		return mpt::random<unsigned int>(rng);
	}
};


using deterministic_random_device = mpt::prng_random_device<mpt::lcg_musl, mpt::prng_random_device_deterministic_seeder>;


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_RANDOM_DEVICE_HPP
