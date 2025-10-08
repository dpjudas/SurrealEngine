/*
 * tuning.cpp
 * ----------
 * Purpose: Alternative sample tuning.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#include "tuning.h"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "../common/serialization_utils.h"
#include "../common/misc_util.h"
#include <string>
#include <cmath>


OPENMPT_NAMESPACE_BEGIN


namespace Tuning {

static RATIOTYPE SanitizeGroupRatio(RATIOTYPE ratio)
{
	return std::clamp(std::abs(ratio), 1e-15f, 1e+07f);
}

namespace CTuningS11n
{
	void ReadStr(std::istream &iStrm, mpt::ustring &ustr, const std::size_t dummy, mpt::Charset charset);
	void ReadNoteMap(std::istream &iStrm, std::map<NOTEINDEXTYPE, mpt::ustring> &m, const std::size_t dummy, mpt::Charset charset);
	void ReadRatioTable(std::istream& iStrm, std::vector<RATIOTYPE>& v, const size_t);

	void WriteNoteMap(std::ostream &oStrm, const std::map<NOTEINDEXTYPE, mpt::ustring> &m);
	void WriteStr(std::ostream &oStrm, const mpt::ustring &ustr);

	struct RatioWriter
	{
		RatioWriter(uint16 nWriteCount = s_nDefaultWriteCount) : m_nWriteCount(nWriteCount) {}

		void operator()(std::ostream& oStrm, const std::vector<float>& v);
		uint16 m_nWriteCount;
		enum : uint16 { s_nDefaultWriteCount = (uint16_max >> 2) };
	};
}

using namespace CTuningS11n;


/*
Version history:
	4->5: Lots of changes, finestep interpretation revamp, fileformat revamp.
	3->4: Changed sizetypes in serialisation from size_t(uint32) to
			smaller types (uint8, USTEPTYPE) (March 2007)
*/
/*
Version changes:
	3->4: Finetune related internal structure and serialization revamp.
	2->3: The type for the size_type in the serialisation changed
		  from default(size_t, uint32) to unsigned STEPTYPE. (March 2007)
*/


static_assert(CTuning::s_RatioTableFineSizeMaxDefault < static_cast<USTEPINDEXTYPE>(FINESTEPCOUNT_MAX));


CTuning::CTuning()
{
#if MPT_GCC_AT_LEAST(12, 0, 0) && MPT_GCC_BEFORE(12, 3, 0)
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=109455
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100366
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
	m_RatioTable.resize(s_RatioTableSizeDefault, 1);
#if MPT_GCC_AT_LEAST(12, 0, 0) && MPT_GCC_BEFORE(12, 3, 0)
#pragma GCC diagnostic pop
#endif
}


bool CTuning::operator==(const CTuning &other) const noexcept
{
	return m_TuningType == other.m_TuningType
		&& m_NoteMin == other.m_NoteMin
		&& m_GroupSize == other.m_GroupSize
		&& m_GroupRatio == other.m_GroupRatio
		&& m_FineStepCount == other.m_FineStepCount
		&& m_RatioTable == other.m_RatioTable
		&& m_RatioTableFine == other.m_RatioTableFine
		&& m_TuningName == other.m_TuningName
		&& m_NoteNameMap == other.m_NoteNameMap;
}


bool CTuning::CreateGroupGeometric(const NOTEINDEXTYPE &s, const RATIOTYPE &r, const NOTEINDEXTYPE &startindex)
{
	if(s < 1 || !IsValidRatio(r) || startindex < GetNoteRange().first)
	{
		return false;
	}
	std::vector<RATIOTYPE> v;
	v.reserve(s);
	for(NOTEINDEXTYPE i = startindex; i < startindex + s; i++)
	{
		v.push_back(GetRatio(i));
	}
	return CreateGroupGeometric(v, r, GetNoteRange(), startindex);
}


bool CTuning::CreateGroupGeometric(const std::vector<RATIOTYPE> &v, const RATIOTYPE &r, const NoteRange &range, const NOTEINDEXTYPE &ratiostartpos)
{
	if(range.first > range.last || v.size() == 0)
	{
		return false;
	}
	if(ratiostartpos < range.first || range.last < ratiostartpos || static_cast<UNOTEINDEXTYPE>(range.last - ratiostartpos) < static_cast<UNOTEINDEXTYPE>(v.size() - 1))
	{
		return false;
	}
	if(GetFineStepCount() > FINESTEPCOUNT_MAX)
	{
		return false;
	}
	for(size_t i = 0; i < v.size(); i++)
	{
		if(v[i] < 0)
		{
			return false;
		}
	}
	if(r <= 0)
	{
		return false;
	}
	m_TuningType = Type::GROUPGEOMETRIC;
	m_NoteMin = range.first;
	m_GroupSize = mpt::saturate_cast<NOTEINDEXTYPE>(v.size());
	m_GroupRatio = std::fabs(r);
	m_RatioTable.resize(range.last - range.first + 1);
	std::copy(v.begin(), v.end(), m_RatioTable.begin() + (ratiostartpos - range.first));
	for(int32 i = ratiostartpos - 1; i >= m_NoteMin && ratiostartpos > NOTEINDEXTYPE_MIN; i--)
	{
		m_RatioTable[i - m_NoteMin] = m_RatioTable[i - m_NoteMin + m_GroupSize] / m_GroupRatio;
	}
	for(int32 i = ratiostartpos + m_GroupSize; i <= range.last && ratiostartpos <= (NOTEINDEXTYPE_MAX - m_GroupSize); i++)
	{
		m_RatioTable[i - m_NoteMin] = m_GroupRatio * m_RatioTable[i - m_NoteMin - m_GroupSize];
	}
	UpdateFineStepTable();
	return true;
}


bool CTuning::CreateGeometric(const UNOTEINDEXTYPE &p, const RATIOTYPE &r)
{
	return CreateGeometric(p, r, GetNoteRange());
}


bool CTuning::CreateGeometric(const UNOTEINDEXTYPE &s, const RATIOTYPE &r, const NoteRange &range)
{
	if(range.first > range.last)
	{
		return false;
	}
	if(s < 1 || !IsValidRatio(r))
	{
		return false;
	}
	if(range.last - range.first + 1 > NOTEINDEXTYPE_MAX)
	{
		return false;
	}
	m_TuningType = Type::GEOMETRIC;
	m_RatioTable.clear();
	m_NoteMin = s_NoteMinDefault;
	m_RatioTable.resize(s_RatioTableSizeDefault, static_cast<RATIOTYPE>(1.0));
	m_GroupSize = 0;
	m_GroupRatio = 0;
	m_RatioTableFine.clear();
	m_NoteMin = range.first;
	m_GroupSize = mpt::saturate_cast<NOTEINDEXTYPE>(s);
	m_GroupRatio = std::fabs(r);
	const RATIOTYPE stepRatio = std::pow(m_GroupRatio, static_cast<RATIOTYPE>(1.0) / static_cast<RATIOTYPE>(m_GroupSize));
	m_RatioTable.resize(range.last - range.first + 1);
	for(int32 i = range.first; i <= range.last; i++)
	{
		m_RatioTable[i - m_NoteMin] = std::pow(stepRatio, static_cast<RATIOTYPE>(i));
	}
	UpdateFineStepTable();
	return true;
}


mpt::ustring CTuning::GetNoteName(const NOTEINDEXTYPE &x, bool addOctave) const
{
	if(!IsValidNote(x))
	{
		return mpt::ustring();
	}
	if(GetGroupSize() < 1)
	{
		const auto i = m_NoteNameMap.find(x);
		if(i != m_NoteNameMap.end())
			return i->second;
		else
			return mpt::ufmt::val(x);
	}
	else
	{
		const NOTEINDEXTYPE pos = static_cast<NOTEINDEXTYPE>(mpt::wrapping_modulo(x, m_GroupSize));
		const NOTEINDEXTYPE middlePeriodNumber = 5;
		mpt::ustring rValue;
		const auto nmi = m_NoteNameMap.find(pos);
		if(nmi != m_NoteNameMap.end())
		{
			rValue = nmi->second;
			if(addOctave)
			{
				rValue += mpt::ufmt::val(middlePeriodNumber + mpt::wrapping_divide(x, m_GroupSize));
			}
		}
		else
		{
			//By default, using notation nnP for notes; nn <-> note character starting
			//from 'A' with char ':' as fill char, and P is period integer. For example:
			//C:5, D:3, R:7
			if(m_GroupSize <= 26)
			{
				rValue = mpt::ToUnicode(mpt::Charset::UTF8, std::string(1, static_cast<char>(pos + 'A')));
				rValue += UL_(":");
			} else
			{
				rValue = mpt::ufmt::HEX0<1>(pos % 16) + mpt::ufmt::HEX0<1>((pos / 16) % 16);
				if(pos > 0xff)
				{
					rValue = mpt::ToUnicode(mpt::Charset::UTF8, mpt::ToLowerCaseAscii(mpt::ToCharset(mpt::Charset::UTF8, rValue)));
				}
			}
			if(addOctave)
			{
				rValue += mpt::ufmt::val(middlePeriodNumber + mpt::wrapping_divide(x, m_GroupSize));
			}
		}
		return rValue;
	}
}


void CTuning::SetNoteName(const NOTEINDEXTYPE &n, const mpt::ustring &str)
{
	const NOTEINDEXTYPE pos = (GetGroupSize() < 1) ? n : static_cast<NOTEINDEXTYPE>(mpt::wrapping_modulo(n, m_GroupSize));
	if(!str.empty())
	{
		m_NoteNameMap[pos] = str;
	} else
	{
		const auto iter = m_NoteNameMap.find(pos);
		if(iter != m_NoteNameMap.end())
		{
			m_NoteNameMap.erase(iter);
		}
	}
}


// Without finetune
RATIOTYPE CTuning::GetRatio(const NOTEINDEXTYPE note) const
{
	if(!IsValidNote(note))
	{
		return s_DefaultFallbackRatio;
	}
	const auto ratio = m_RatioTable[note - m_NoteMin];
	if(ratio <= 1e-15f)
	{
		return s_DefaultFallbackRatio;
	}
	return ratio;
}


// With finetune
RATIOTYPE CTuning::GetRatio(const NOTEINDEXTYPE baseNote, const STEPINDEXTYPE baseFineSteps) const
{
	const STEPINDEXTYPE fineStepCount = static_cast<STEPINDEXTYPE>(GetFineStepCount());
	if(fineStepCount == 0 || baseFineSteps == 0)
	{
		return GetRatio(static_cast<NOTEINDEXTYPE>(baseNote + baseFineSteps));
	}

	// If baseFineSteps is more than the number of finesteps between notes, note is increased.
	// So first figuring out what note and fineStep values to actually use.
	// Interpreting finestep==-1 on note x so that it is the same as finestep==fineStepCount on note x-1.
	// Note: If fineStepCount is n, n+1 steps are needed to get to next note.
	const NOTEINDEXTYPE note = static_cast<NOTEINDEXTYPE>(baseNote + mpt::wrapping_divide(baseFineSteps, (fineStepCount + 1)));
	const STEPINDEXTYPE fineStep = mpt::wrapping_modulo(baseFineSteps, (fineStepCount + 1));
	if(!IsValidNote(note))
	{
		return s_DefaultFallbackRatio;
	}
	if(fineStep == 0)
	{
		return m_RatioTable[note - m_NoteMin];
	}

	RATIOTYPE fineRatio = static_cast<RATIOTYPE>(1.0);
	if(GetType() == Type::GEOMETRIC && m_RatioTableFine.size() > 0)
	{
		fineRatio = m_RatioTableFine[fineStep - 1];
	} else if(GetType() == Type::GROUPGEOMETRIC && m_RatioTableFine.size() > 0)
	{
		fineRatio = m_RatioTableFine[GetRefNote(note) * fineStepCount + fineStep - 1];
	} else
	{
		// Geometric finestepping
		fineRatio = std::pow(GetRatio(note + 1) / GetRatio(note), static_cast<RATIOTYPE>(fineStep) / static_cast<RATIOTYPE>(fineStepCount + 1));
	}
	return m_RatioTable[note - m_NoteMin] * fineRatio;
}


bool CTuning::SetRatio(const NOTEINDEXTYPE& s, const RATIOTYPE& r)
{
	if(GetType() != Type::GROUPGEOMETRIC && GetType() != Type::GENERAL)
	{
		return false;
	}
	//Creating ratio table if doesn't exist.
	if(m_RatioTable.empty())
	{
		m_RatioTable.assign(s_RatioTableSizeDefault, 1);
		m_NoteMin = s_NoteMinDefault;
	}
	if(!IsValidNote(s))
	{
		return false;
	}
	m_RatioTable[s - m_NoteMin] = std::fabs(r);
	if(GetType() == Type::GROUPGEOMETRIC)
	{ // update other groups
		for(NOTEINDEXTYPE n = m_NoteMin; n < m_NoteMin + static_cast<NOTEINDEXTYPE>(m_RatioTable.size()); ++n)
		{
			if(n == s)
			{
				// nothing
			} else if(std::abs(n - s) % m_GroupSize == 0)
			{
				m_RatioTable[n - m_NoteMin] = std::pow(m_GroupRatio, static_cast<RATIOTYPE>(n - s) / static_cast<RATIOTYPE>(m_GroupSize)) * m_RatioTable[s - m_NoteMin];
			}
		}
		UpdateFineStepTable();
	}
	return true;
}


void CTuning::SetFineStepCount(const USTEPINDEXTYPE& fs)
{
	m_FineStepCount = std::clamp(mpt::saturate_cast<STEPINDEXTYPE>(fs), STEPINDEXTYPE(0), FINESTEPCOUNT_MAX);
	UpdateFineStepTable();
}


void CTuning::UpdateFineStepTable()
{
	if(m_FineStepCount <= 0)
	{
		m_RatioTableFine.clear();
		return;
	}
	if(GetType() == Type::GEOMETRIC)
	{
		if(m_FineStepCount > s_RatioTableFineSizeMaxDefault)
		{
			m_RatioTableFine.clear();
			return;
		}
		m_RatioTableFine.resize(m_FineStepCount);
		const RATIOTYPE q = GetRatio(GetNoteRange().first + 1) / GetRatio(GetNoteRange().first);
		const RATIOTYPE rFineStep = std::pow(q, static_cast<RATIOTYPE>(1) / static_cast<RATIOTYPE>(m_FineStepCount + 1));
		for(USTEPINDEXTYPE i = 1; i<=m_FineStepCount; i++)
			m_RatioTableFine[i-1] = std::pow(rFineStep, static_cast<RATIOTYPE>(i));
		return;
	}
	if(GetType() == Type::GROUPGEOMETRIC)
	{
		const UNOTEINDEXTYPE p = GetGroupSize();
		if(p > s_RatioTableFineSizeMaxDefault / m_FineStepCount)
		{
			//In case fineratiotable would become too large, not using
			//table for it.
			m_RatioTableFine.clear();
			return;
		}
		else
		{
			//Creating 'geometric' finestepping between notes.
			m_RatioTableFine.resize(p * m_FineStepCount);
			const NOTEINDEXTYPE startnote = GetRefNote(GetNoteRange().first);
			for(UNOTEINDEXTYPE i = 0; i<p; i++)
			{
				const NOTEINDEXTYPE refnote = GetRefNote(static_cast<NOTEINDEXTYPE>(startnote + i));
				const RATIOTYPE rFineStep = std::pow(GetRatio(refnote+1) / GetRatio(refnote), static_cast<RATIOTYPE>(1) / static_cast<RATIOTYPE>(m_FineStepCount + 1));
				for(UNOTEINDEXTYPE j = 1; j<=m_FineStepCount; j++)
				{
					m_RatioTableFine[m_FineStepCount * refnote + (j-1)] = std::pow(rFineStep, static_cast<RATIOTYPE>(j));
				}
			}
			return;
		}

	}
	if(GetType() == Type::GENERAL)
	{
		//Not using table with tuning of type general.
		m_RatioTableFine.clear();
		return;
	}

	//Should not reach here.
	m_RatioTableFine.clear();
	m_FineStepCount = 0;
}


bool CTuning::Multiply(const RATIOTYPE r)
{
	if(!IsValidRatio(r))
	{
		return false;
	}
	for(auto & ratio : m_RatioTable)
	{
		ratio *= r;
	}
	return true;
}


bool CTuning::ChangeGroupsize(const NOTEINDEXTYPE& s)
{
	if(s < 1)
		return false;

	if(m_TuningType == Type::GROUPGEOMETRIC)
		return CreateGroupGeometric(s, GetGroupRatio(), 0);

	if(m_TuningType == Type::GEOMETRIC)
		return CreateGeometric(s, GetGroupRatio());

	return false;
}


bool CTuning::ChangeGroupRatio(const RATIOTYPE& r)
{
	if(!IsValidRatio(r))
		return false;

	if(m_TuningType == Type::GROUPGEOMETRIC)
		return CreateGroupGeometric(GetGroupSize(), r, 0);

	if(m_TuningType == Type::GEOMETRIC)
		return CreateGeometric(GetGroupSize(), r);

	return false;
}


SerializationResult CTuning::InitDeserialize(std::istream &iStrm, mpt::Charset defaultCharset)
{
	// Note: OpenMPT since at least r323 writes version number (4<<24)+4 while it
	// reads version number (5<<24)+4 or earlier.
	// We keep this behaviour.

	if(iStrm.fail())
		return SerializationResult::Failure;

	srlztn::SsbRead ssb(iStrm);
	ssb.BeginRead("CTB244RTI", (5 << 24) + 4); // version
	int8 use_utf8 = 0;
	ssb.ReadItem(use_utf8, "UTF8");
	const mpt::Charset charset = use_utf8 ? mpt::Charset::UTF8 : defaultCharset;
	ssb.ReadItem(m_TuningName, "0", [charset](std::istream &iStrm, mpt::ustring &ustr, const std::size_t dummy){ return ReadStr(iStrm, ustr, dummy, charset); });
	uint16 dummyEditMask = 0xffff;
	ssb.ReadItem(dummyEditMask, "1");
	std::underlying_type<Type>::type type = 0;
	ssb.ReadItem(type, "2");
	m_TuningType = static_cast<Type>(type);
	ssb.ReadItem(m_NoteNameMap, "3", [charset](std::istream &iStrm, std::map<NOTEINDEXTYPE, mpt::ustring> &m, const std::size_t dummy){ return ReadNoteMap(iStrm, m, dummy, charset); });
	ssb.ReadItem(m_FineStepCount, "4");

	// RTI entries.
	ssb.ReadItem(m_RatioTable, "RTI0", ReadRatioTable);
	ssb.ReadItem(m_NoteMin, "RTI1");
	ssb.ReadItem(m_GroupSize, "RTI2");
	ssb.ReadItem(m_GroupRatio, "RTI3");
	UNOTEINDEXTYPE ratiotableSize = 0;
	ssb.ReadItem(ratiotableSize, "RTI4");

	m_GroupRatio = SanitizeGroupRatio(m_GroupRatio);
	if(!std::isfinite(m_GroupRatio))
	{
		return SerializationResult::Failure;
	}
	for(auto ratio : m_RatioTable)
	{
		if(!std::isfinite(ratio))
			return SerializationResult::Failure;
	}

	// If reader status is ok and m_NoteMin is somewhat reasonable, process data.
	if(ssb.HasFailed() || (m_NoteMin < -300) || (m_NoteMin > 300))
	{
		return SerializationResult::Failure;
	}

	// reject unknown types
	if(m_TuningType != Type::GENERAL && m_TuningType != Type::GROUPGEOMETRIC && m_TuningType != Type::GEOMETRIC)
	{
		return SerializationResult::Failure;
	}
	if(m_GroupSize < 0)
	{
		return SerializationResult::Failure;
	}
	m_FineStepCount = std::clamp(mpt::saturate_cast<STEPINDEXTYPE>(m_FineStepCount), STEPINDEXTYPE(0), FINESTEPCOUNT_MAX);
	if(m_RatioTable.size() > static_cast<size_t>(NOTEINDEXTYPE_MAX))
	{
		return SerializationResult::Failure;
	}
	if((GetType() == Type::GROUPGEOMETRIC) || (GetType() == Type::GEOMETRIC))
	{
		if(ratiotableSize < 1 || ratiotableSize > NOTEINDEXTYPE_MAX)
		{
			return SerializationResult::Failure;
		}
		if(GetType() == Type::GEOMETRIC)
		{
			if(!CreateGeometric(GetGroupSize(), GetGroupRatio(), NoteRange{m_NoteMin, static_cast<NOTEINDEXTYPE>(m_NoteMin + ratiotableSize - 1)}))
			{
				return SerializationResult::Failure;
			}
		} else
		{
			if(!CreateGroupGeometric(m_RatioTable, GetGroupRatio(), NoteRange{m_NoteMin, static_cast<NOTEINDEXTYPE>(m_NoteMin + ratiotableSize - 1)}, m_NoteMin))
			{
				return SerializationResult::Failure;
			}
		}
	} else
	{
		UpdateFineStepTable();
	}
	return SerializationResult::Success;
}


template<class T, class SIZETYPE, class Tdst>
static bool VectorFromBinaryStream(std::istream& inStrm, std::vector<Tdst>& v, const SIZETYPE maxSize = (std::numeric_limits<SIZETYPE>::max)())
{
	if(!inStrm.good())
		return false;

	SIZETYPE size = 0;
	mpt::IO::ReadIntLE<SIZETYPE>(inStrm, size);

	if(size > maxSize)
		return false;

	v.resize(size);
	for(std::size_t i = 0; i<size; i++)
	{
		T tmp = T();
		mpt::IO::Read(inStrm, tmp);
		v[i] = tmp;
	}

	return inStrm.good();
}


SerializationResult CTuning::InitDeserializeOLD(std::istream &inStrm, mpt::Charset defaultCharset)
{
	if(!inStrm.good())
		return SerializationResult::Failure;

	const std::streamoff startPos = static_cast<std::streamoff>(inStrm.tellg());

	//First checking is there expected begin sequence.
	char begin[8];
	MemsetZero(begin);
	inStrm.read(begin, sizeof(begin));
	if(std::memcmp(begin, "CTRTI_B.", 8))
	{
		//Returning stream position if beginmarker was not found.
		inStrm.seekg(startPos, std::ios::beg);
		return SerializationResult::Failure;
	}

	//Version
	int16 version = 0;
	mpt::IO::ReadIntLE<int16>(inStrm, version);
	if(version != 2 && version != 3)
		return SerializationResult::Failure;

	char begin2[8];
	MemsetZero(begin2);
	inStrm.read(begin2, sizeof(begin2));
	if(std::memcmp(begin2, "CT<sfs>B", 8))
	{
		return SerializationResult::Failure;
	}

	int16 version2 = 0;
	mpt::IO::ReadIntLE<int16>(inStrm, version2);
	if(version2 != 3 && version2 != 4)
	{
		return SerializationResult::Failure;
	}

	//Tuning name
	if(version2 <= 3)
	{
		std::string tmpName;
		if(!mpt::IO::ReadSizedStringLE<uint32>(inStrm, tmpName, 0xffff))
		{
			return SerializationResult::Failure;
		}
		m_TuningName = mpt::ToUnicode(defaultCharset, tmpName);
	} else
	{
		std::string tmpName;
		if(!mpt::IO::ReadSizedStringLE<uint8>(inStrm, tmpName))
		{
			return SerializationResult::Failure;
		}
		m_TuningName = mpt::ToUnicode(defaultCharset, tmpName);
	}

	//Const mask
	int16 em = 0;
	mpt::IO::ReadIntLE<int16>(inStrm, em);

	//Tuning type
	int16 tt = 0;
	mpt::IO::ReadIntLE<int16>(inStrm, tt);
	m_TuningType = static_cast<Type>(tt);

	//Notemap
	uint16 size = 0;
	if(version2 <= 3)
	{
		uint32 tempsize = 0;
		mpt::IO::ReadIntLE<uint32>(inStrm, tempsize);
		if(tempsize > 0xffff)
		{
			return SerializationResult::Failure;
		}
		size = mpt::saturate_cast<uint16>(tempsize);
	} else
	{
		mpt::IO::ReadIntLE<uint16>(inStrm, size);
	}
	for(UNOTEINDEXTYPE i = 0; i<size; i++)
	{
		std::string str;
		int16 n = 0;
		mpt::IO::ReadIntLE<int16>(inStrm, n);
		if(version2 <= 3)
		{
			if(!mpt::IO::ReadSizedStringLE<uint32>(inStrm, str, 0xffff))
			{
				return SerializationResult::Failure;
			}
		} else
		{
			if(!mpt::IO::ReadSizedStringLE<uint8>(inStrm, str))
			{
				return SerializationResult::Failure;
			}
		}
		m_NoteNameMap[n] = mpt::ToUnicode(defaultCharset, str);
	}

	//End marker
	char end2[8];
	MemsetZero(end2);
	inStrm.read(end2, sizeof(end2));
	if(std::memcmp(end2, "CT<sfs>E", 8))
	{
		return SerializationResult::Failure;
	}

	// reject unknown types
	if(m_TuningType != Type::GENERAL && m_TuningType != Type::GROUPGEOMETRIC && m_TuningType != Type::GEOMETRIC)
	{
		return SerializationResult::Failure;
	}

	//Ratiotable
	if(version <= 2)
	{
		if(!VectorFromBinaryStream<IEEE754binary32LE, uint32>(inStrm, m_RatioTable, 0xffff))
		{
			return SerializationResult::Failure;
		}
	} else
	{
		if(!VectorFromBinaryStream<IEEE754binary32LE, uint16>(inStrm, m_RatioTable))
		{
			return SerializationResult::Failure;
		}
	}
	for(auto ratio : m_RatioTable)
	{
		if(!std::isfinite(ratio))
			return SerializationResult::Failure;
	}

	//Fineratios
	if(version <= 2)
	{
		if(!VectorFromBinaryStream<IEEE754binary32LE, uint32>(inStrm, m_RatioTableFine, 0xffff))
		{
			return SerializationResult::Failure;
		}
	} else
	{
		if(!VectorFromBinaryStream<IEEE754binary32LE, uint16>(inStrm, m_RatioTableFine))
		{
			return SerializationResult::Failure;
		}
	}
	for(auto ratio : m_RatioTableFine)
	{
		if(!std::isfinite(ratio))
			return SerializationResult::Failure;
	}
	m_FineStepCount = mpt::saturate_cast<USTEPINDEXTYPE>(m_RatioTableFine.size());

	// m_NoteMin
	int16 notemin = 0;
	mpt::IO::ReadIntLE<int16>(inStrm, notemin);
	m_NoteMin = notemin;
	if(m_NoteMin < -200 || m_NoteMin > 200)
	{
		return SerializationResult::Failure;
	}

	//m_GroupSize
	int16 groupsize = 0;
	mpt::IO::ReadIntLE<int16>(inStrm, groupsize);
	m_GroupSize = groupsize;
	if(m_GroupSize < 0)
	{
		return SerializationResult::Failure;
	}

	//m_GroupRatio
	IEEE754binary32LE groupratio = IEEE754binary32LE(0.0f);
	mpt::IO::Read(inStrm, groupratio);
	m_GroupRatio = SanitizeGroupRatio(groupratio);
	if(!std::isfinite(m_GroupRatio))
	{
		return SerializationResult::Failure;
	}

	std::array<char, 8> end = {};
	mpt::IO::Read(inStrm, end);
	if(std::memcmp(end.data(), "CTRTI_E.", 8))
	{
		return SerializationResult::Failure;
	}

	// reject corrupt tunings
	if(m_RatioTable.size() > static_cast<std::size_t>(NOTEINDEXTYPE_MAX))
	{
		return SerializationResult::Failure;
	}
	if((m_GroupSize <= 0 || m_GroupRatio <= 0) && m_TuningType != Type::GENERAL)
	{
		return SerializationResult::Failure;
	}
	if(m_TuningType == Type::GROUPGEOMETRIC || m_TuningType == Type::GEOMETRIC)
	{
		if(m_RatioTable.size() < static_cast<std::size_t>(m_GroupSize))
		{
			return SerializationResult::Failure;
		}
	}

	// convert old finestepcount
	if(m_FineStepCount > 0)
	{
		m_FineStepCount -= 1;
	}
	m_FineStepCount = std::clamp(mpt::saturate_cast<STEPINDEXTYPE>(m_FineStepCount), STEPINDEXTYPE(0), FINESTEPCOUNT_MAX);
	UpdateFineStepTable();

	if(m_TuningType == Type::GEOMETRIC)
	{
		// Convert old geometric to new groupgeometric because old geometric tunings
		// can have ratio(0) != 1.0, which would get lost when saving nowadays.
		if(mpt::saturate_cast<NOTEINDEXTYPE>(m_RatioTable.size()) >= m_GroupSize - m_NoteMin)
		{
			std::vector<RATIOTYPE> ratios;
			for(NOTEINDEXTYPE n = 0; n < m_GroupSize; ++n)
			{
				ratios.push_back(m_RatioTable[n - m_NoteMin]);
			}
			CreateGroupGeometric(ratios, m_GroupRatio, GetNoteRange(), 0);
		}
	}

	return SerializationResult::Success;
}


Tuning::SerializationResult CTuning::Serialize(std::ostream& outStrm) const
{
	// Note: OpenMPT since at least r323 writes version number (4<<24)+4 while it
	// reads version number (5<<24)+4.
	// We keep this behaviour.
	srlztn::SsbWrite ssb(outStrm);
	ssb.BeginWrite("CTB244RTI", (4 << 24) + 4); // version
	ssb.WriteItem(int8(1), "UTF8");
	if (m_TuningName.length() > 0)
		ssb.WriteItem(m_TuningName, "0", WriteStr);
	uint16 dummyEditMask = 0xffff;
	ssb.WriteItem(dummyEditMask, "1");
	ssb.WriteItem(mpt::to_underlying(m_TuningType), "2");
	if (m_NoteNameMap.size() > 0)
		ssb.WriteItem(m_NoteNameMap, "3", WriteNoteMap);
	if (GetFineStepCount() > 0)
		ssb.WriteItem(m_FineStepCount, "4");

	const Tuning::Type tt = GetType();
	if (GetGroupRatio() > 0)
		ssb.WriteItem(m_GroupRatio, "RTI3");
	if (tt == Type::GROUPGEOMETRIC)
		ssb.WriteItem(m_RatioTable, "RTI0", RatioWriter(GetGroupSize()));
	if (tt == Type::GENERAL)
		ssb.WriteItem(m_RatioTable, "RTI0", RatioWriter());
	if (tt == Type::GEOMETRIC)
		ssb.WriteItem(m_GroupSize, "RTI2");

	if(tt == Type::GEOMETRIC || tt == Type::GROUPGEOMETRIC)
	{	//For Groupgeometric this data is the number of ratios in ratiotable.
		UNOTEINDEXTYPE ratiotableSize = static_cast<UNOTEINDEXTYPE>(m_RatioTable.size());
		ssb.WriteItem(ratiotableSize, "RTI4");
	}

	// m_NoteMin
	ssb.WriteItem(m_NoteMin, "RTI1");

	ssb.FinishWrite();

	return ssb.HasFailed() ? Tuning::SerializationResult::Failure : Tuning::SerializationResult::Success;
}


#ifdef MODPLUG_TRACKER

bool CTuning::WriteSCL(std::ostream &f, const mpt::PathString &filename) const
{
	mpt::IO::WriteTextCRLF(f, MPT_AFORMAT("! {}")(mpt::ToCharset(mpt::Charset::ISO8859_1, (filename.GetFilenameBase() + filename.GetFilenameExtension()).ToUnicode())));
	mpt::IO::WriteTextCRLF(f, "!");
	std::string name = mpt::ToCharset(mpt::Charset::ISO8859_1, GetName());
	for(auto & c : name) { if(static_cast<uint8>(c) < 32) c = ' '; } // remove control characters
	if(name.length() >= 1 && name[0] == '!') name[0] = '?'; // do not confuse description with comment
	mpt::IO::WriteTextCRLF(f, name);
	if(GetType() == Type::GEOMETRIC)
	{
		mpt::IO::WriteTextCRLF(f, MPT_AFORMAT(" {}")(m_GroupSize));
		mpt::IO::WriteTextCRLF(f, "!");
		for(NOTEINDEXTYPE n = 0; n < m_GroupSize; ++n)
		{
			double ratio = std::pow(static_cast<double>(m_GroupRatio), static_cast<double>(n + 1) / static_cast<double>(m_GroupSize));
			double cents = std::log2(ratio) * 1200.0;
			mpt::IO::WriteTextCRLF(f, MPT_AFORMAT(" {} ! {}")(
				mpt::afmt::fix(cents),
				mpt::ToCharset(mpt::Charset::ISO8859_1, GetNoteName((n + 1) % m_GroupSize, false))
				));
		}
	} else if(GetType() == Type::GROUPGEOMETRIC)
	{
		mpt::IO::WriteTextCRLF(f, MPT_AFORMAT(" {}")(m_GroupSize));
		mpt::IO::WriteTextCRLF(f, "!");
		for(NOTEINDEXTYPE n = 0; n < m_GroupSize; ++n)
		{
			bool last = (n == (m_GroupSize - 1));
			double baseratio = static_cast<double>(GetRatio(0));
			double ratio = static_cast<double>(last ? m_GroupRatio : GetRatio(n + 1)) / baseratio;
			double cents = std::log2(ratio) * 1200.0;
			mpt::IO::WriteTextCRLF(f, MPT_AFORMAT(" {} ! {}")(
				mpt::afmt::fix(cents),
				mpt::ToCharset(mpt::Charset::ISO8859_1, GetNoteName((n + 1) % m_GroupSize, false))
				));
		}
	} else if(GetType() == Type::GENERAL)
	{
		mpt::IO::WriteTextCRLF(f, MPT_AFORMAT(" {}")(m_RatioTable.size() + 1));
		mpt::IO::WriteTextCRLF(f, "!");
		double baseratio = 1.0;
		for(NOTEINDEXTYPE n = 0; n < mpt::saturate_cast<NOTEINDEXTYPE>(m_RatioTable.size()); ++n)
		{
			baseratio = std::min(baseratio, static_cast<double>(m_RatioTable[n]));
		}
		for(NOTEINDEXTYPE n = 0; n < mpt::saturate_cast<NOTEINDEXTYPE>(m_RatioTable.size()); ++n)
		{
			double ratio = static_cast<double>(m_RatioTable[n]) / baseratio;
			double cents = std::log2(ratio) * 1200.0;
			mpt::IO::WriteTextCRLF(f, MPT_AFORMAT(" {} ! {}")(
				mpt::afmt::fix(cents),
				mpt::ToCharset(mpt::Charset::ISO8859_1, GetNoteName(n + m_NoteMin, false))
				));
		}
		mpt::IO::WriteTextCRLF(f, MPT_AFORMAT(" {} ! {}")(
			mpt::afmt::val(1),
			std::string()
			));
	} else
	{
		return false;
	}
	return true;
}

#endif


namespace CTuningS11n
{

void RatioWriter::operator()(std::ostream& oStrm, const std::vector<float>& v)
{
	const std::size_t nWriteCount = std::min(v.size(), static_cast<std::size_t>(m_nWriteCount));
	mpt::IO::WriteAdaptiveInt64LE(oStrm, nWriteCount);
	for(size_t i = 0; i < nWriteCount; i++)
		mpt::IO::Write(oStrm, IEEE754binary32LE(v[i]));
}


void ReadNoteMap(std::istream &iStrm, std::map<NOTEINDEXTYPE, mpt::ustring> &m, const std::size_t dummy, mpt::Charset charset)
{
	MPT_UNREFERENCED_PARAMETER(dummy);
	uint64 val;
	mpt::IO::ReadAdaptiveInt64LE(iStrm, val);
	LimitMax(val, 256u); // Read 256 at max.
	for(size_t i = 0; i < val; i++)
	{
		int16 key;
		mpt::IO::ReadIntLE<int16>(iStrm, key);
		std::string str;
		mpt::IO::ReadSizedStringLE<uint8>(iStrm, str);
		m[key] = mpt::ToUnicode(charset, str);
	}
}


void ReadRatioTable(std::istream& iStrm, std::vector<RATIOTYPE>& v, const size_t)
{
	uint64 val;
	mpt::IO::ReadAdaptiveInt64LE(iStrm, val);
	v.resize(std::min(mpt::saturate_cast<std::size_t>(val), std::size_t(256))); // Read 256 vals at max.
	for(size_t i = 0; i < v.size(); i++)
	{
		IEEE754binary32LE tmp(0.0f);
		mpt::IO::Read(iStrm, tmp);
		v[i] = tmp;
	}
}


void ReadStr(std::istream &iStrm, mpt::ustring &ustr, const std::size_t dummy, mpt::Charset charset)
{
	MPT_UNREFERENCED_PARAMETER(dummy);
	std::string str;
	uint64 val;
	mpt::IO::ReadAdaptiveInt64LE(iStrm, val);
	size_t nSize = (val > 255) ? 255 : static_cast<size_t>(val); // Read 255 characters at max.
	str.clear();
	str.resize(nSize);
	for(size_t i = 0; i < nSize; i++)
		mpt::IO::ReadIntLE(iStrm, str[i]);
	if(str.find_first_of('\0') != std::string::npos)
	{ // trim \0 at the end
		str.resize(str.find_first_of('\0'));
	}
	ustr = mpt::ToUnicode(charset, str);
}


void WriteNoteMap(std::ostream &oStrm, const std::map<NOTEINDEXTYPE, mpt::ustring> &m)
{
	mpt::IO::WriteAdaptiveInt64LE(oStrm, m.size());
	for(auto &mi : m)
	{
		mpt::IO::WriteIntLE<int16>(oStrm, mi.first);
		mpt::IO::WriteSizedStringLE<uint8>(oStrm, mpt::ToCharset(mpt::Charset::UTF8, mi.second));
	}
}


void WriteStr(std::ostream &oStrm, const mpt::ustring &ustr)
{
	std::string str = mpt::ToCharset(mpt::Charset::UTF8, ustr);
	mpt::IO::WriteAdaptiveInt64LE(oStrm, str.size());
	oStrm.write(str.c_str(), str.size());
}

} // namespace CTuningS11n.


} // namespace Tuning


OPENMPT_NAMESPACE_END
