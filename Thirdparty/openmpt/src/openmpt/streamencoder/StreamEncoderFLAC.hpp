/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/random/any_engine.hpp"

#include "openmpt/base/Types.hpp"
#include "openmpt/soundfile_data/tags.hpp"
#include "openmpt/streamencoder/StreamEncoder.hpp"

#include <iosfwd>
#include <memory>


OPENMPT_NAMESPACE_BEGIN


class FLACEncoder : public EncoderFactoryBase
{

public:

	std::unique_ptr<IAudioStreamEncoder> ConstructStreamEncoder(std::ostream &file, const Encoder::Settings &settings, const FileTags &tags, mpt::any_engine<uint64> &prng) const override;
	bool IsAvailable() const override;

public:

	FLACEncoder();
};


OPENMPT_NAMESPACE_END
