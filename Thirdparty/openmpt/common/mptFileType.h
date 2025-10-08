/*
 * mptFileType.h
 * -------------
 * Purpose:
 * Notes  : Currently none.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/alloc.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/string/types.hpp"

#include "openmpt/base/FlagSet.hpp"

#include <vector>



OPENMPT_NAMESPACE_BEGIN



#if defined(MODPLUG_TRACKER)

enum FileTypeFormat
{
	FileTypeFormatNone           = 0   , // do not show extensions after description, i.e. "Foo Files"
	FileTypeFormatShowExtensions = 1<<0, // show extensions after descripten, i.e. "Foo Files (*.foo,*.bar)"
};
MPT_DECLARE_ENUM(FileTypeFormat)

class FileType
{
private:
	mpt::ustring m_ShortName; // "flac", "mod" (lowercase)
	mpt::ustring m_Description; // "FastTracker 2 Module"
	std::vector<std::string> m_MimeTypes; // "audio/ogg" (in ASCII)
	std::vector<mpt::PathString> m_Extensions; // "mod", "xm" (lowercase)
	std::vector<mpt::PathString> m_Prefixes; // "mod" for "mod.*"
public:
	FileType() { }
	FileType(const std::vector<FileType> &group)
	{
		for(const auto &type : group)
		{
			mpt::append(m_MimeTypes, type.m_MimeTypes);
			mpt::append(m_Extensions, type.m_Extensions);
			mpt::append(m_Prefixes, type.m_Prefixes);
		}
	}
	static FileType Any()
	{
		return FileType().ShortName(U_("*")).Description(U_("All Files")).AddExtension(P_("*"));
	}
public:
	FileType& ShortName(const mpt::ustring &shortName) { m_ShortName = shortName; return *this; }
	FileType& Description(const mpt::ustring &description) { m_Description = description; return *this; }
	FileType& MimeTypes(const std::vector<std::string> &mimeTypes) { m_MimeTypes = mimeTypes; return *this; }
	FileType& Extensions(const std::vector<mpt::PathString> &extensions) { m_Extensions = extensions; return *this; }
	FileType& Prefixes(const std::vector<mpt::PathString> &prefixes) { m_Prefixes = prefixes; return *this; }
	FileType& AddMimeType(const std::string &mimeType) { m_MimeTypes.push_back(mimeType); return *this; }
	FileType& AddExtension(const mpt::PathString &extension) { m_Extensions.push_back(extension); return *this; }
	FileType& AddPrefix(const mpt::PathString &prefix) { m_Prefixes.push_back(prefix); return *this; }
public:
	mpt::ustring GetShortName() const { return m_ShortName; }
	mpt::ustring GetDescription() const { return m_Description; }
	std::vector<std::string> GetMimeTypes() const { return m_MimeTypes; }
	std::vector<mpt::PathString> GetExtensions() const { return m_Extensions; }
	std::vector<mpt::PathString> GetPrefixes() const { return m_Prefixes; }
public:
	mpt::PathString AsFilterString(FlagSet<FileTypeFormat> format = FileTypeFormatNone) const;
	mpt::PathString AsFilterOnlyString() const;
}; // class FileType


// "Ogg Vorbis|*.ogg;*.oga|" // FileTypeFormatNone
// "Ogg Vorbis (*.ogg,*.oga)|*.ogg;*.oga|" // FileTypeFormatShowExtensions
mpt::PathString ToFilterString(const FileType &fileType, FlagSet<FileTypeFormat> format = FileTypeFormatNone);
mpt::PathString ToFilterString(const std::vector<FileType> &fileTypes, FlagSet<FileTypeFormat> format = FileTypeFormatNone);

// "*.ogg;*.oga" / ";*.ogg;*.oga"
mpt::PathString ToFilterOnlyString(const FileType &fileType, bool prependSemicolonWhenNotEmpty = false);
mpt::PathString ToFilterOnlyString(const std::vector<FileType> &fileTypes, bool prependSemicolonWhenNotEmpty = false);

#endif // MODPLUG_TRACKER



OPENMPT_NAMESPACE_END
