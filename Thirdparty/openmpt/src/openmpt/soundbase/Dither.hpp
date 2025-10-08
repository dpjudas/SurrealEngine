/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/macros.hpp"
#include "mpt/random/default_engines.hpp"
#include "mpt/random/engine.hpp"
#include "mpt/random/seed.hpp"
#include "openmpt/soundbase/MixSample.hpp"

#include <vector>
#include <variant>

#include <cstddef>


OPENMPT_NAMESPACE_BEGIN


template <typename Tdither>
class MultiChannelDither
{
private:
	std::vector<Tdither> DitherChannels;
	typename Tdither::prng_type prng;

public:
	template <typename Trd>
	MultiChannelDither(Trd &rd, std::size_t channels)
		: DitherChannels(channels)
		, prng(Tdither::prng_init(rd))
	{
		return;
	}
	void Reset()
	{
		for(std::size_t channel = 0; channel < DitherChannels.size(); ++channel)
		{
			DitherChannels[channel] = Tdither{};
		}
	}
	std::size_t GetChannels() const
	{
		return DitherChannels.size();
	}
	template <uint32 targetbits>
	MPT_FORCEINLINE MixSampleInt process(std::size_t channel, MixSampleInt sample)
	{
		return DitherChannels[channel].template process<targetbits>(sample, prng);
	}
	template <uint32 targetbits>
	MPT_FORCEINLINE MixSampleFloat process(std::size_t channel, MixSampleFloat sample)
	{
		return DitherChannels[channel].template process<targetbits>(sample, prng);
	}
};


template <typename AvailableDithers, typename DitherNames, std::size_t defaultChannels, std::size_t defaultDither, std::size_t noDither, typename seeding_random_engine = mpt::good_engine>
class Dithers
	: public DitherNames
{

public:
	static constexpr std::size_t NoDither = noDither;
	static constexpr std::size_t DefaultDither = defaultDither;
	static constexpr std::size_t DefaultChannels = defaultChannels;

private:
	seeding_random_engine m_PRNG;
	AvailableDithers m_Dithers;

public:
	template <typename Trd>
	Dithers(Trd &rd, std::size_t mode = defaultDither, std::size_t channels = defaultChannels)
		: m_PRNG(mpt::make_prng<seeding_random_engine>(rd))
		, m_Dithers(std::in_place_index<defaultDither>, m_PRNG, channels)
	{
		SetMode(mode, channels);
	}

	AvailableDithers &Variant()
	{
		return m_Dithers;
	}

	static std::size_t GetNumDithers()
	{
		return std::variant_size<AvailableDithers>::value;
	}

	static std::size_t GetDefaultDither()
	{
		return defaultDither;
	}

	static std::size_t GetNoDither()
	{
		return noDither;
	}

private:
	template <std::size_t i = 0>
	void set_mode(std::size_t mode, std::size_t channels)
	{
		if constexpr(i < std::variant_size<AvailableDithers>::value)
		{
			if(mode == i)
			{
				m_Dithers.template emplace<i>(m_PRNG, channels);
			} else
			{
				this->template set_mode<i + 1>(mode, channels);
			}
		} else
		{
			MPT_UNUSED(mode);
			m_Dithers.template emplace<defaultDither>(m_PRNG, channels);
		}
	}

public:
	void SetMode(std::size_t mode, std::size_t channels)
	{
		if((mode == GetMode()) && (channels == GetChannels()))
		{
			std::visit([](auto &dither)
					   { return dither.Reset(); },
					   m_Dithers);
			return;
		}
		set_mode(mode, channels);
	}
	void SetMode(std::size_t mode)
	{
		if(mode == GetMode())
		{
			std::visit([](auto &dither)
					   { return dither.Reset(); },
					   m_Dithers);
			return;
		}
		set_mode(mode, GetChannels());
	}
	void SetChannels(std::size_t channels)
	{
		if(channels == GetChannels())
		{
			return;
		}
		set_mode(GetMode(), channels);
	}
	void Reset()
	{
		std::visit([](auto &dither)
				   { return dither.Reset(); },
				   m_Dithers);
	}
	std::size_t GetMode() const
	{
		return m_Dithers.index();
	}
	std::size_t GetChannels() const
	{
		return std::visit([](auto &dither)
						  { return dither.GetChannels(); },
						  m_Dithers);
	}
};


OPENMPT_NAMESPACE_END
