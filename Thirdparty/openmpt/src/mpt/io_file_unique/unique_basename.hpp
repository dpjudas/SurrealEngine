/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_IO_FILE_UNIQUE_UNIQUE_BASENAME_HPP
#define MPT_IO_FILE_UNIQUE_UNIQUE_BASENAME_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/path/os_path.hpp"
#include "mpt/random/default_engines.hpp"
#include "mpt/random/device.hpp"
#include "mpt/random/seed.hpp"
#include "mpt/string_transcode/transcode.hpp"
#include "mpt/uuid/uuid.hpp"



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace IO {



class unique_basename {

private:
	mpt::os_path m_Basename;

private:
	static mpt::good_engine make_prng() {
		mpt::sane_random_device rd;
		return mpt::make_prng<mpt::good_engine>(rd);
	}

	static mpt::UUID make_uuid() {
		mpt::good_engine rng = make_prng();
		return mpt::UUID::Generate(rng);
	}

public:
	explicit unique_basename(mpt::UUID uuid)
		: m_Basename(mpt::transcode<mpt::os_path>(uuid.ToUString())) {
		return;
	}

	explicit unique_basename(const mpt::os_path & prefix, mpt::UUID uuid)
		: m_Basename((prefix.empty() ? prefix : prefix + MPT_OS_PATH("-")) + mpt::transcode<mpt::os_path>(uuid.ToUString())) {
		return;
	}

	template <typename Trng>
	explicit unique_basename(Trng & rng)
		: m_Basename(mpt::transcode<mpt::os_path>(mpt::UUID::Generate(rng).ToUString())) {
		return;
	}

	template <typename Trng>
	explicit unique_basename(const mpt::os_path & prefix, Trng & rng)
		: m_Basename((prefix.empty() ? prefix : prefix + MPT_OS_PATH("-")) + mpt::transcode<mpt::os_path>(mpt::UUID::Generate(rng).ToUString())) {
		return;
	}

	explicit unique_basename()
		: m_Basename(mpt::transcode<mpt::os_path>(make_uuid().ToUString())) {
		return;
	}

public:
	operator mpt::os_path() const {
		return m_Basename;
	}

}; // class unique_basename




} // namespace IO



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_IO_FILE_UNIQUE_UNIQUE_BASENAME_HPP
