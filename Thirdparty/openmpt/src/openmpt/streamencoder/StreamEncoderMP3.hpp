/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/random/any_engine.hpp"
#include "mpt/string/types.hpp"

#include "openmpt/base/Types.hpp"
#include "openmpt/soundfile_data/tags.hpp"
#include "openmpt/streamencoder/StreamEncoder.hpp"

#include <iosfwd>
#include <memory>


OPENMPT_NAMESPACE_BEGIN


enum MP3EncoderType
{
	MP3EncoderLame,
	MP3EncoderLameCompatible,
};

class MP3Encoder : public EncoderFactoryBase
{

private:

	MP3EncoderType m_Type;

public:

	std::unique_ptr<IAudioStreamEncoder> ConstructStreamEncoder(std::ostream &file, const Encoder::Settings &settings, const FileTags &tags, mpt::any_engine<uint64> &prng) const override;
	mpt::ustring DescribeQuality(float quality) const override;
	mpt::ustring DescribeBitrateABR(int bitrate) const override;
	bool IsAvailable() const override;

public:

	MP3Encoder(MP3EncoderType type);
};


OPENMPT_NAMESPACE_END
