/*
 * mptFileType.cpp
 * ---------------
 * Purpose:
 * Notes  : Currently none.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"
#include "mptFileType.h"


OPENMPT_NAMESPACE_BEGIN



#ifdef MODPLUG_TRACKER



mpt::PathString FileType::AsFilterString(FlagSet<FileTypeFormat> format) const
{
	mpt::PathString filter;
	if(GetShortName().empty() || GetExtensions().empty())
	{
		return filter;
	}
	if(!GetDescription().empty())
	{
		filter += mpt::PathString::FromUnicode(GetDescription());
	} else
	{
		filter += mpt::PathString::FromUnicode(GetShortName());
	}
	const auto extensions = GetExtensions();
	if(format[FileTypeFormatShowExtensions])
	{
		filter += P_(" (");
		bool first = true;
		for(const auto &ext : extensions)
		{
			if(first)
			{
				first = false;
			} else
			{
				filter += P_(",");
			}
			filter += P_("*.");
			filter += ext;
		}
		filter += P_(")");
	}
	filter += P_("|");
	{
		bool first = true;
		for(const auto &ext : extensions)
		{
			if(first)
			{
				first = false;
			} else
			{
				filter += P_(";");
			}
			filter += P_("*.");
			filter += ext;
		}
	}
	filter += P_("|");
	return filter;
}


mpt::PathString FileType::AsFilterOnlyString() const
{
	mpt::PathString filter;
	const auto extensions = GetExtensions();
	{
		bool first = true;
		for(const auto &ext : extensions)
		{
			if(first)
			{
				first = false;
			} else
			{
				filter += P_(";");
			}
			filter += P_("*.");
			filter += ext;
		}
	}
	return filter;
}


mpt::PathString ToFilterString(const FileType &fileType, FlagSet<FileTypeFormat> format)
{
	return fileType.AsFilterString(format);
}


mpt::PathString ToFilterString(const std::vector<FileType> &fileTypes, FlagSet<FileTypeFormat> format)
{
	mpt::PathString filter;
	for(const auto &type : fileTypes)
	{
		filter += type.AsFilterString(format);
	}
	return filter;
}


mpt::PathString ToFilterOnlyString(const FileType &fileType, bool prependSemicolonWhenNotEmpty)
{
	mpt::PathString filter = fileType.AsFilterOnlyString();
	return filter.empty() ? filter : (prependSemicolonWhenNotEmpty ? P_(";") : P_("")) + filter;
}


mpt::PathString ToFilterOnlyString(const std::vector<FileType> &fileTypes, bool prependSemicolonWhenNotEmpty)
{
	mpt::PathString filter;
	for(const auto &type : fileTypes)
	{
		filter += type.AsFilterOnlyString();
	}
	return filter.empty() ? filter : (prependSemicolonWhenNotEmpty ? P_(";") : P_("")) + filter;
}



#else
MPT_MSVC_WORKAROUND_LNK4221(mptFileType)
#endif // MODPLUG_TRACKER



OPENMPT_NAMESPACE_END
