/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_SECURE_HPP
#define MPT_BASE_SECURE_HPP



#include "mpt/base/integer.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"

#include <atomic>
#include <utility>
#include <vector>

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



// C23 memset_explicit
inline MPT_NOINLINE void * memset_explicit(void * const dst, int const value, std::size_t const len) noexcept {
	std::atomic_thread_fence(std::memory_order_seq_cst);
	volatile unsigned char * volatile p = static_cast<volatile unsigned char *>(dst);
	std::atomic_thread_fence(std::memory_order_seq_cst);
	for (volatile std::size_t i = 0; i < len; ++i) {
		p[i] = static_cast<unsigned char>(value);
	}
	std::atomic_thread_fence(std::memory_order_seq_cst);
	return dst;
}



namespace secure {



inline MPT_NOINLINE void memzero(std::byte * const dst, std::size_t const len) noexcept {
	std::atomic_thread_fence(std::memory_order_seq_cst);
	volatile std::byte * volatile p = static_cast<volatile std::byte *>(dst);
	std::atomic_thread_fence(std::memory_order_seq_cst);
	for (volatile std::size_t i = 0; i < len; ++i) {
		p[i] = std::byte{0};
	}
	std::atomic_thread_fence(std::memory_order_seq_cst);
}

inline MPT_NOINLINE void memzero(void * const dst, std::size_t const len) noexcept {
	std::atomic_thread_fence(std::memory_order_seq_cst);
	volatile std::byte * volatile p = static_cast<volatile std::byte *>(dst);
	std::atomic_thread_fence(std::memory_order_seq_cst);
	for (volatile std::size_t i = 0; i < len; ++i) {
		p[i] = std::byte{0};
	}
	std::atomic_thread_fence(std::memory_order_seq_cst);
}

inline MPT_NOINLINE void memzero(char * const dst, std::size_t const len) noexcept {
	std::atomic_thread_fence(std::memory_order_seq_cst);
	volatile std::byte * volatile p = reinterpret_cast<volatile std::byte *>(dst);
	std::atomic_thread_fence(std::memory_order_seq_cst);
	for (volatile std::size_t i = 0; i < len; ++i) {
		p[i] = std::byte{0};
	}
	std::atomic_thread_fence(std::memory_order_seq_cst);
}

inline MPT_NOINLINE void memzero(uint8 * const dst, std::size_t const len) noexcept {
	std::atomic_thread_fence(std::memory_order_seq_cst);
	volatile std::byte * volatile p = reinterpret_cast<volatile std::byte *>(dst);
	std::atomic_thread_fence(std::memory_order_seq_cst);
	for (volatile std::size_t i = 0; i < len; ++i) {
		p[i] = std::byte{0};
	}
	std::atomic_thread_fence(std::memory_order_seq_cst);
}



template <typename T>
inline MPT_NOINLINE void clear(T & val) {
	std::atomic_signal_fence(std::memory_order_seq_cst);
	volatile T * volatile v = &val;
	std::atomic_thread_fence(std::memory_order_seq_cst);
	*v = T{};
	std::atomic_signal_fence(std::memory_order_seq_cst);
}



class byte {
private:
	std::byte value;

public:
	byte() noexcept
		: value(std::byte{0}) {
		return;
	}
	explicit byte(std::byte value) noexcept
		: value(value) {
		return;
	}
	byte(const byte & other) noexcept
		: value(other.value) {
		return;
	}
	byte(byte && other) noexcept
		: value(std::move(other.value)) {
		mpt::secure::clear(other.value);
	}
	byte & operator=(const byte & other) noexcept {
		if (&other == this) {
			return *this;
		}
		value = other.value;
		return *this;
	}
	byte & operator==(byte && other) noexcept {
		if (&other == this) {
			return *this;
		}
		value = std::move(other.value);
		mpt::secure::clear(other.value);
		return *this;
	}
	explicit operator std::byte() const noexcept {
		return value;
	}
	~byte() {
		mpt::secure::clear(value);
	}
};



class buffer {
private:
	std::vector<std::byte> m_data;

public:
	buffer()
		: m_data(0) {
		return;
	}
	explicit buffer(const std::vector<std::byte> & data)
		: m_data(data) {
		return;
	}
	explicit buffer(const std::byte * beg, const std::byte * end)
		: m_data(beg, end) {
		return;
	}
	buffer(const buffer & other)
		: m_data(other.m_data) {
		return;
	}
	buffer(buffer && other) noexcept
		: m_data(std::move(other.m_data)) {
		mpt::secure::memzero(other.m_data.data(), other.m_data.size());
	}
	buffer & operator=(const buffer & other) {
		if (&other == this) {
			return *this;
		}
		m_data = other.m_data;
		return *this;
	}
	buffer & operator=(buffer && other) noexcept {
		if (&other == this) {
			return *this;
		}
		m_data = std::move(other.m_data);
		mpt::secure::memzero(other.m_data.data(), other.m_data.size());
		return *this;
	}
	~buffer() {
		mpt::secure::memzero(m_data.data(), m_data.size());
		m_data.resize(0);
		m_data.shrink_to_fit();
	}
	explicit operator std::vector<std::byte>() const {
		return m_data;
	}
	const std::byte * data() const {
		return m_data.data();
	}
	std::byte * data() {
		return m_data.data();
	}
	std::size_t size() const {
		return m_data.size();
	}
};



} // namespace secure



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_SECURE_HPP
