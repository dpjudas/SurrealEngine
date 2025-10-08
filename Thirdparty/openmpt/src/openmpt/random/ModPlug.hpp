/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */

#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/bit.hpp"
#include "mpt/random/random.hpp"
#include "openmpt/base/Types.hpp"

#include <limits>
#include <type_traits>


OPENMPT_NAMESPACE_BEGIN


namespace mpt
{


namespace rng
{


template <typename Tstate, typename Tvalue, Tstate x1, Tstate x2, Tstate x3, Tstate x4, int rol1, int rol2>
class modplug
{
public:
	typedef Tstate state_type;
	typedef Tvalue result_type;

private:
	state_type state1;
	state_type state2;

public:
	template <typename Trng>
	explicit inline modplug(Trng &rd)
		: state1(mpt::random<state_type>(rd))
		, state2(mpt::random<state_type>(rd))
	{
	}
	explicit inline modplug(state_type seed1, state_type seed2)
		: state1(seed1)
		, state2(seed2)
	{
	}

public:
	static MPT_CONSTEXPRINLINE result_type min()
	{
		return static_cast<result_type>(0);
	}
	static MPT_CONSTEXPRINLINE result_type max()
	{
		return std::numeric_limits<result_type>::max();
	}
	static MPT_CONSTEXPRINLINE int result_bits()
	{
		static_assert(std::is_integral<result_type>::value);
		static_assert(std::is_unsigned<result_type>::value);
		return std::numeric_limits<result_type>::digits;
	}
	inline result_type operator()()
	{
		state_type a = state1;
		state_type b = state2;
		a = mpt::rotl(a, rol1);
		a ^= x1;
		a += x2 + (b * x3);
		b += mpt::rotl(a, rol2) * x4;
		state1 = a;
		state2 = b;
		result_type result = static_cast<result_type>(b);
		return result;
	}
};

typedef modplug<uint32, uint32, 0x10204080u, 0x78649E7Du, 4, 5, 1, 16> modplug_dither;


}  // namespace rng


}  // namespace mpt


OPENMPT_NAMESPACE_END
