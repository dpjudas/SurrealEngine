/*
 * tuningCollection.cpp
 * --------------------
 * Purpose: Alternative sample tuning collection class.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "tuningcollection.h"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "../common/serialization_utils.h"
#include <algorithm>
#include "Loaders.h"
#ifdef MODPLUG_TRACKER
#include "../common/mptFileIO.h"
#include "mpt/fs/fs.hpp"
#include "mpt/io_file/outputfile.hpp"
#include "../mptrack/TrackerSettings.h"
#endif //MODPLUG_TRACKER


OPENMPT_NAMESPACE_BEGIN


namespace Tuning {


/*
Version history:
	2->3: Serialization revamp(August 2007)
	1->2: Sizetypes of string serialisation from size_t(uint32)
		  to uint8. (March 2007)
*/


namespace CTuningS11n
{
	void ReadStr(std::istream &iStrm, mpt::ustring &ustr, const std::size_t dummy, mpt::Charset charset);
	void WriteStr(std::ostream &oStrm, const mpt::ustring &ustr);
} // namespace CTuningS11n

using namespace CTuningS11n;


static void ReadTuning(std::istream &iStrm, CTuningCollection &Tc, const std::size_t dummy, mpt::Charset defaultCharset)
{
	MPT_UNREFERENCED_PARAMETER(dummy);
	Tc.AddTuning(iStrm, defaultCharset);
}

static void WriteTuning(std::ostream& oStrm, const CTuning& t)
{
	t.Serialize(oStrm);
}


CTuning* CTuningCollection::GetTuning(const mpt::ustring &name)
{
	for(std::size_t i = 0; i<m_Tunings.size(); i++)
	{
		if(m_Tunings[i]->GetName() == name)
		{
			return m_Tunings[i].get();
		}
	}
	return nullptr;
}

const CTuning* CTuningCollection::GetTuning(const mpt::ustring &name) const
{
	for(std::size_t i = 0; i<m_Tunings.size(); i++)
	{
		if(m_Tunings[i]->GetName() == name)
		{
			return m_Tunings[i].get();
		}
	}
	return nullptr;
}


const CTuning* CTuningCollection::FindIdenticalTuning(const CTuning &tuning) const
{
	auto result = std::find_if(m_Tunings.begin(), m_Tunings.end(), [&tuning](const std::unique_ptr<CTuning> &other)
	{
		return other && tuning == *other;
	});
	return (result != m_Tunings.end()) ? result->get() : nullptr;
}


CTuning* CTuningCollection::FindIdenticalTuning(const CTuning &tuning)
{
	auto result = std::find_if(m_Tunings.begin(), m_Tunings.end(), [&tuning](const std::unique_ptr<CTuning> &other)
	{
		return other && tuning == *other;
	});
	return (result != m_Tunings.end()) ? result->get() : nullptr;
}


Tuning::SerializationResult CTuningCollection::Serialize(std::ostream& oStrm, const mpt::ustring &name) const
{
	srlztn::SsbWrite ssb(oStrm);
	ssb.BeginWrite("TC", 3); // version
	ssb.WriteItem(int8(1), "UTF8");
	ssb.WriteItem(name, "0", &WriteStr);
	uint16 dummyEditMask = 0xffff;
	ssb.WriteItem(dummyEditMask, "1");

	const size_t tcount = m_Tunings.size();
	for(size_t i = 0; i<tcount; i++)
		ssb.WriteItem(*m_Tunings[i], "2", &WriteTuning);
	ssb.FinishWrite();
		
	if(ssb.HasFailed())
		return Tuning::SerializationResult::Failure;
	else
		return Tuning::SerializationResult::Success;
}


Tuning::SerializationResult CTuningCollection::Deserialize(std::istream &iStrm, mpt::ustring &name, mpt::Charset defaultCharset)
{
	std::streamoff startpos = static_cast<std::streamoff>(iStrm.tellg());
	
	const Tuning::SerializationResult oldLoadingResult = DeserializeOLD(iStrm, name, defaultCharset);

	if(oldLoadingResult == Tuning::SerializationResult::NoMagic)
	{	// An old version was not recognised - trying new version.
		iStrm.clear();
		iStrm.seekg(startpos, std::ios::beg);
		srlztn::SsbRead ssb(iStrm);
		ssb.BeginRead("TC", 3); // version
		int8 use_utf8 = 0;
		ssb.ReadItem(use_utf8, "UTF8");
		const mpt::Charset charset = use_utf8 ? mpt::Charset::UTF8 : defaultCharset;

		const srlztn::SsbRead::ReadIterator iterBeg = ssb.GetReadBegin();
		const srlztn::SsbRead::ReadIterator iterEnd = ssb.GetReadEnd();
		for(srlztn::SsbRead::ReadIterator iter = iterBeg; iter != iterEnd; iter++)
		{
			uint16 dummyEditMask = 0xffff;
			if(ssb.MatchesId(iter, "0"))
				ssb.ReadIterItem(iter, name, [charset](std::istream &iStrm, mpt::ustring &ustr, const std::size_t dummy){ return ReadStr(iStrm, ustr, dummy, charset); });
			else if(ssb.MatchesId(iter, "1"))
				ssb.ReadIterItem(iter, dummyEditMask);
			else if(ssb.MatchesId(iter, "2"))
				ssb.ReadIterItem(iter, *this, [charset](std::istream &iStrm, CTuningCollection &Tc, const std::size_t dummy){ return ReadTuning(iStrm, Tc, dummy, charset); });
		}

		if(ssb.HasFailed())
			return Tuning::SerializationResult::Failure;
		else
			return Tuning::SerializationResult::Success;
	}
	else
	{
		return oldLoadingResult;
	}
}


Tuning::SerializationResult CTuningCollection::DeserializeOLD(std::istream &inStrm, mpt::ustring &uname, mpt::Charset defaultCharset)
{

	//1. begin marker:
	uint32 beginMarker = 0;
	mpt::IO::ReadIntLE<uint32>(inStrm, beginMarker);
	if(beginMarker != MagicBE("TCSH"))	// Magic is reversed in file, hence BE
		return Tuning::SerializationResult::NoMagic;

	//2. version
	int32 version = 0;
	mpt::IO::ReadIntLE<int32>(inStrm, version);
	if(version > 2 || version < 1)
		return Tuning::SerializationResult::Failure;

	//3. Name
	if(version < 2)
	{
		std::string name;
		if(!mpt::IO::ReadSizedStringLE<uint32>(inStrm, name, 256))
			return Tuning::SerializationResult::Failure;
		uname = mpt::ToUnicode(defaultCharset, name);
	}
	else
	{
		std::string name;
		if(!mpt::IO::ReadSizedStringLE<uint8>(inStrm, name))
			return Tuning::SerializationResult::Failure;
		uname = mpt::ToUnicode(defaultCharset, name);
	}

	//4. Editmask
	int16 em = 0;
	mpt::IO::ReadIntLE<int16>(inStrm, em);
	//Not assigning the value yet, for if it sets some property const,
	//further loading might fail.

	//5. Tunings
	{
		uint32 s = 0;
		mpt::IO::ReadIntLE<uint32>(inStrm, s);
		if(s > 50)
			return Tuning::SerializationResult::Failure;
		for(size_t i = 0; i<s; i++)
		{
			if(!AddTuning(inStrm, defaultCharset))
			{
				return Tuning::SerializationResult::Failure;
			}
		}
	}

	//6. End marker
	uint32 endMarker = 0;
	mpt::IO::ReadIntLE<uint32>(inStrm, endMarker);
	if(endMarker != MagicBE("TCSF"))	// Magic is reversed in file, hence BE
		return Tuning::SerializationResult::Failure;
	
	return Tuning::SerializationResult::Success;
}



bool CTuningCollection::Remove(const CTuning *pT)
{
	const auto it = std::find_if(m_Tunings.begin(), m_Tunings.end(),
		[&] (const std::unique_ptr<CTuning> & upT) -> bool
		{
			return upT.get() == pT;
		}
		);
	if(it == m_Tunings.end())
	{
		return false;
	}
	m_Tunings.erase(it);
	return true;
}


bool CTuningCollection::Remove(const std::size_t i)
{
	if(i >= m_Tunings.size())
	{
		return false;
	}
	m_Tunings.erase(m_Tunings.begin() + i);
	return true;
}


CTuning* CTuningCollection::AddTuning(std::unique_ptr<CTuning> pT)
{
	if(m_Tunings.size() >= s_nMaxTuningCount)
	{
		return nullptr;
	}
	if(!pT)
	{
		return nullptr;
	}
	CTuning *result = pT.get();
	m_Tunings.push_back(std::move(pT));
	return result;
}


CTuning* CTuningCollection::AddTuning(std::istream &inStrm, mpt::Charset defaultCharset)
{
	if(m_Tunings.size() >= s_nMaxTuningCount)
	{
		return nullptr;
	}
	if(!inStrm.good())
	{
		return nullptr;
	}
	std::unique_ptr<CTuning> pT = CTuning::CreateDeserializeOLD(inStrm, defaultCharset);
	if(!pT)
	{
		pT = CTuning::CreateDeserialize(inStrm, defaultCharset);
	}
	if(!pT)
	{
		return nullptr;
	}
	CTuning *result = pT.get();
	m_Tunings.push_back(std::move(pT));
	return result;
}


#ifdef MODPLUG_TRACKER


bool UnpackTuningCollection(const CTuningCollection &tc, const mpt::PathString &prefix)
{
	bool error = false;
	auto numberFmt = mpt::format_simple_spec<mpt::ustring>().Dec().FillNul().Width(1 + static_cast<int>(std::log10(tc.GetNumTunings())));
	for(std::size_t i = 0; i < tc.GetNumTunings(); ++i)
	{
		const CTuning & tuning = *(tc.GetTuning(i));
		mpt::PathString fn;
		fn += prefix;
		mpt::ustring tuningName = tuning.GetName();
		if(tuningName.empty())
		{
			tuningName = U_("untitled");
		}
		fn += mpt::PathString::FromUnicode(MPT_UFORMAT("{} - {}")(mpt::ufmt::fmt(i + 1, numberFmt), tuningName)).AsSanitizedComponent();
		fn += mpt::PathString::FromUTF8(CTuning::s_FileExtension);
		if(mpt::native_fs{}.exists(fn))
		{
			error = true;
		} else
		{
			mpt::IO::SafeOutputFile sfout(fn, std::ios::binary, mpt::IO::FlushModeFromBool(TrackerSettings::Instance().MiscFlushFileBuffersOnSave));
			if(tuning.Serialize(sfout) != Tuning::SerializationResult::Success)
			{
				error = true;
			}
		}
	}
	return !error;
}


#endif


} // namespace Tuning


OPENMPT_NAMESPACE_END
