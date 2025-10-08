/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: Olivier Lapicque */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/all/PlatformFixes.hpp"

#include "SoundDevice.hpp"

#include "mpt/base/alloc.hpp"
#include "mpt/binary/hex.hpp"
#include "mpt/format/join.hpp"
#include "mpt/format/message_macros.hpp"
#include "mpt/format/simple.hpp"
#include "mpt/parse/split.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string/utility.hpp"
#include "mpt/string_transcode/transcode.hpp"
#include "openmpt/base/Types.hpp"

#include <map>
#include <string>
#include <vector>


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice
{


SoundDevice::Type ParseType(const SoundDevice::Identifier &identifier)
{
	std::vector<mpt::ustring> tmp = mpt::split(identifier, MPT_USTRING("_"));
	if(tmp.size() == 0)
	{
		return SoundDevice::Type();
	}
	return tmp[0];
}


mpt::ustring Info::GetDisplayName() const
{
	mpt::ustring result = apiName + MPT_USTRING(" - ") + mpt::trim(name);
	switch(flags.usability)
	{
		case SoundDevice::Info::Usability::Experimental:
			result += MPT_USTRING(" [experimental]");
			break;
		case SoundDevice::Info::Usability::Deprecated:
			result += MPT_USTRING(" [deprecated]");
			break;
		case SoundDevice::Info::Usability::Broken:
			result += MPT_USTRING(" [broken]");
			break;
		case SoundDevice::Info::Usability::NotAvailable:
			result += MPT_USTRING(" [alien]");
			break;
		default:
			// nothing
			break;
	}
	if(default_ == SoundDevice::Info::Default::Named)
	{
		result += MPT_USTRING(" [default]");
	}
	if(apiPath.size() > 0)
	{
		result += MPT_USTRING(" (") + mpt::join(apiPath, MPT_USTRING("/")) + MPT_USTRING(")");
	}
	return result;
}


SoundDevice::Identifier Info::GetIdentifier() const
{
	if(!IsValid())
	{
		return mpt::ustring();
	}
	mpt::ustring result = mpt::ustring();
	result += type;
	result += MPT_USTRING("_");
	if(useNameAsIdentifier)
	{
		// UTF8-encode the name and convert the utf8 to hex.
		// This ensures that no special characters are contained in the configuration key.
		std::string utf8String = mpt::transcode<std::string>(mpt::common_encoding::utf8, name);
		mpt::ustring hexString = mpt::encode_hex(mpt::as_span(utf8String));
		result += hexString;
	} else
	{
		result += internalID;  // safe to not contain special characters
	}
	return result;
}


ChannelMapping::ChannelMapping(uint32 numHostChannels)
{
	ChannelToDeviceChannel.resize(numHostChannels);
	for(uint32 channel = 0; channel < numHostChannels; ++channel)
	{
		ChannelToDeviceChannel[channel] = channel;
	}
}


ChannelMapping::ChannelMapping(const std::vector<int32> &mapping)
{
	if(IsValid(mapping))
	{
		ChannelToDeviceChannel = mapping;
	}
}


ChannelMapping ChannelMapping::BaseChannel(uint32 channels, int32 baseChannel)
{
	SoundDevice::ChannelMapping result;
	result.ChannelToDeviceChannel.resize(channels);
	for(uint32 channel = 0; channel < channels; ++channel)
	{
		result.ChannelToDeviceChannel[channel] = channel + baseChannel;
	}
	return result;
}


bool ChannelMapping::IsValid(const std::vector<int32> &mapping)
{
	if(mapping.empty())
	{
		return true;
	}
	std::map<int32, uint32> inverseMapping;
	for(uint32 hostChannel = 0; hostChannel < mapping.size(); ++hostChannel)
	{
		int32 deviceChannel = mapping[hostChannel];
		if(deviceChannel < 0)
		{
			return false;
		}
		if(deviceChannel > MaxDeviceChannel)
		{
			return false;
		}
		inverseMapping[deviceChannel] = hostChannel;
	}
	if(inverseMapping.size() != mapping.size())
	{
		return false;
	}
	return true;
}


mpt::ustring ChannelMapping::ToUString() const
{
	return mpt::join_format<mpt::ustring, int32>(ChannelToDeviceChannel, MPT_USTRING(","));
}


ChannelMapping ChannelMapping::FromString(const mpt::ustring &str)
{
	return SoundDevice::ChannelMapping(mpt::split_parse<int32>(str, MPT_USTRING(",")));
}


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
