/*
 * ModSequence.cpp
 * ---------------
 * Purpose: Order and sequence handling.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "ModSequence.h"
#include "mod_specifications.h"
#include "Sndfile.h"
#include "../common/version.h"
#include "../common/serialization_utils.h"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"

OPENMPT_NAMESPACE_BEGIN


ModSequence::ModSequence(CSoundFile &sndFile)
	: m_sndFile{sndFile}
{
}


ModSequence& ModSequence::operator=(const ModSequence &other)
{
	if(&other == this)
		return *this;
	std::vector<PATTERNINDEX>::assign(other.begin(), other.end());
	m_name = other.m_name;
	m_restartPos = other.m_restartPos;
	m_defaultTempo = other.m_defaultTempo;
	m_defaultSpeed = other.m_defaultSpeed;
	return *this;
}


bool ModSequence::operator== (const ModSequence &other) const noexcept
{
	return static_cast<const std::vector<PATTERNINDEX> &>(*this) == other
		&& m_name == other.m_name
		&& m_restartPos == other.m_restartPos
		&& m_defaultTempo == other.m_defaultTempo
		&& m_defaultSpeed == other.m_defaultSpeed;
}


void ModSequence::SetDefaultTempo(TEMPO tempo) noexcept
{
	if(!tempo.GetInt())
		tempo.Set(125);
	else
		LimitMax(tempo, TEMPO{uint16_max, 0});

	m_defaultTempo = tempo;
}


bool ModSequence::NeedsExtraDatafield() const noexcept
{
	return (m_sndFile.GetType() == MOD_TYPE_MPT && m_sndFile.Patterns.GetNumPatterns() > 0xFD);
}


void ModSequence::AdjustToNewModType(const MODTYPE oldtype)
{
	auto &specs = m_sndFile.GetModSpecifications();

	if(oldtype != MOD_TYPE_NONE)
	{
		// If not supported, remove "+++" separator order items.
		if(!specs.hasIgnoreIndex)
		{
			RemovePattern(PATTERNINDEX_SKIP);
		}
		// If not supported, remove "---" items between patterns.
		if(!specs.hasStopIndex)
		{
			RemovePattern(PATTERNINDEX_INVALID);
		}
	}

	//Resize orderlist if needed.
	if(specs.ordersMax < size())
	{
		// Order list too long? Remove "unnecessary" order items first.
		if(oldtype != MOD_TYPE_NONE && specs.ordersMax < GetLengthTailTrimmed())
		{
			erase(std::remove_if(begin(), end(), [&] (PATTERNINDEX pat) { return !m_sndFile.Patterns.IsValidPat(pat); }), end());
			if(GetLengthTailTrimmed() > specs.ordersMax)
			{
				m_sndFile.AddToLog(LogWarning, U_("WARNING: Order list has been trimmed!"));
			}
		}
		resize(specs.ordersMax);
	}
}


ORDERINDEX ModSequence::GetLengthTailTrimmed() const noexcept
{
	if(empty())
		return 0;
	auto last = std::find_if(rbegin(), rend(), [] (PATTERNINDEX pat) { return pat != PATTERNINDEX_INVALID; });
	return static_cast<ORDERINDEX>(std::distance(begin(), last.base()));
}


ORDERINDEX ModSequence::GetLengthFirstEmpty() const noexcept
{
	return static_cast<ORDERINDEX>(std::distance(begin(), std::find(begin(), end(), PATTERNINDEX_INVALID)));
}


ORDERINDEX ModSequence::GetRemainingCapacity(ORDERINDEX startingFrom, bool enforceFormatLimits) const noexcept
{
	const ORDERINDEX ordersMax = enforceFormatLimits ? m_sndFile.GetModSpecifications().ordersMax : MAX_ORDERS;
	ORDERINDEX length = GetLengthTailTrimmed();
	if(startingFrom != ORDERINDEX_INVALID && startingFrom > length)
		length = startingFrom;
	if(length >= ordersMax)
		return 0;
	else
		return ordersMax - length;
}


ORDERINDEX ModSequence::GetNextOrderIgnoringSkips(const ORDERINDEX start) const noexcept
{
	if(empty())
		return 0;
	auto length = GetLength();
	ORDERINDEX next = std::min(ORDERINDEX(length - 1), ORDERINDEX(start + 1));
	while(next + 1 < length && (*this)[next] == PATTERNINDEX_SKIP)
		next++;
	return next;
}


ORDERINDEX ModSequence::GetPreviousOrderIgnoringSkips(const ORDERINDEX start) const noexcept
{
	const ORDERINDEX last = GetLastIndex();
	if(start == 0 || last == 0)
		return 0;
	ORDERINDEX prev = std::min(ORDERINDEX(start - 1), last);
	while(prev > 0 && (*this)[prev] == PATTERNINDEX_SKIP)
		prev--;
	return prev;
}


ORDERINDEX ModSequence::GetFirstValidIndex() const noexcept
{
	const ORDERINDEX length = GetLength();
	for(ORDERINDEX ord = 0; ord < length; ord++)
	{
		if(IsValidPat(ord))
			return ord;
	}
	return ORDERINDEX_INVALID;
}


void ModSequence::Remove(ORDERINDEX posBegin, ORDERINDEX posEnd) noexcept
{
	if(posEnd < posBegin || posEnd >= size())
		return;
	erase(begin() + posBegin, begin() + posEnd + 1);
}


// Remove all references to a given pattern index from the order list. Jump commands are updated accordingly.
void ModSequence::RemovePattern(PATTERNINDEX pat)
{
	// First, calculate the offset that needs to be applied to jump commands
	const ORDERINDEX orderLength = GetLengthTailTrimmed();
	std::vector<ORDERINDEX> newPosition(orderLength);
	ORDERINDEX maxJump = 0;
	for(ORDERINDEX i = 0; i < orderLength; i++)
	{
		newPosition[i] = i - maxJump;
		if((*this)[i] == pat)
		{
			maxJump++;
		}
	}
	if(!maxJump)
	{
		return;
	}

	erase(std::remove(begin(), end(), pat), end());

	// Only apply to patterns actually found in this sequence
	for(auto p : *this)
	{
		if(!m_sndFile.Patterns.IsValidPat(p))
			continue;

		for(auto &m : m_sndFile.Patterns[p])
		{
			if(m.command == CMD_POSITIONJUMP && m.param < newPosition.size())
			{
				m.param = static_cast<ModCommand::PARAM>(newPosition[m.param]);
			}
		}
	}
	if(m_restartPos < newPosition.size())
	{
		m_restartPos = newPosition[m_restartPos];
	}
}


void ModSequence::assign(ORDERINDEX newSize, PATTERNINDEX pat)
{
	LimitMax(newSize, m_sndFile.GetModSpecifications().ordersMax);
	std::vector<PATTERNINDEX>::assign(newSize, pat);
}


ORDERINDEX ModSequence::insert(ORDERINDEX pos, ORDERINDEX count, PATTERNINDEX fill, bool enforceFormatLimits)
{
	const ORDERINDEX ordersMax = enforceFormatLimits ? m_sndFile.GetModSpecifications().ordersMax : MAX_ORDERS;
	// Limit number of orders to be inserted so that we don't exceed the format limit or drop items at the end of the order list.
	LimitMax(count, GetRemainingCapacity(pos, enforceFormatLimits));
	if(pos >= ordersMax || GetLengthTailTrimmed() >= ordersMax || count == 0)
		return 0;
	reserve(std::max(pos, GetLength()) + count);
	// Inserting past the end of the container?
	if(pos > size())
		resize(pos);
	std::vector<PATTERNINDEX>::insert(begin() + pos, count, fill);
	// Did we overgrow? Remove patterns at end.
	if(size() > ordersMax)
		resize(ordersMax);
	return count;
}


ORDERINDEX ModSequence::insert(ORDERINDEX pos, const mpt::span<const PATTERNINDEX> orders, bool enforceFormatLimits)
{
	MPT_ASSERT(reinterpret_cast<uintptr_t>(orders.data()) < reinterpret_cast<uintptr_t>(data()) || reinterpret_cast<uintptr_t>(orders.data()) > reinterpret_cast<uintptr_t>(data() + size()));
	ORDERINDEX count = insert(pos, mpt::saturate_cast<ORDERINDEX>(orders.size()), 0, enforceFormatLimits);
	std::copy(orders.begin(), orders.begin() + count, begin() + pos);
	return count;
}


bool ModSequence::IsValidPat(ORDERINDEX ord) const noexcept
{
	if(ord < size())
		return m_sndFile.Patterns.IsValidPat((*this)[ord]);
	return false;
}


CPattern *ModSequence::PatternAt(ORDERINDEX ord) const noexcept
{
	if(!IsValidPat(ord))
		return nullptr;
	return &m_sndFile.Patterns[(*this)[ord]];
}


ORDERINDEX ModSequence::FindOrder(PATTERNINDEX pat, ORDERINDEX startSearchAt, bool searchForward) const noexcept
{
	const ORDERINDEX length = GetLength();
	if(startSearchAt >= length)
		return ORDERINDEX_INVALID;
	ORDERINDEX ord = startSearchAt;
	for(ORDERINDEX p = 0; p < length; p++)
	{
		if((*this)[ord] == pat)
		{
			return ord;
		}
		if(searchForward)
		{
			if(++ord >= length)
				ord = 0;
		} else
		{
			if(ord-- == 0)
				ord = length - 1;
		}
	}
	return ORDERINDEX_INVALID;
}


PATTERNINDEX ModSequence::EnsureUnique(ORDERINDEX ord)
{
	if(ord >= size())
		return PATTERNINDEX_INVALID;
	PATTERNINDEX pat = (*this)[ord];
	if(!IsValidPat(ord))
		return pat;

	for(const auto &sequence : m_sndFile.Order)
	{
		ORDERINDEX ords = sequence.GetLength();
		for(ORDERINDEX o = 0; o < ords; o++)
		{
			if(sequence[o] == pat && (o != ord || &sequence != this))
			{
				// Found duplicate usage.
				PATTERNINDEX newPat = m_sndFile.Patterns.Duplicate(pat);
				if(newPat != PATTERNINDEX_INVALID)
				{
					(*this)[ord] = newPat;
					return newPat;
				}
			}
		}
	}
	return pat;
}


/////////////////////////////////////
// ModSequenceSet
/////////////////////////////////////


ModSequenceSet::ModSequenceSet(CSoundFile &sndFile)
	: m_sndFile(sndFile)
{
	Initialize();
}


ModSequenceSet& ModSequenceSet::operator=(const ModSequenceSet &other)
{
	if(&other == this)
		return *this;
	m_Sequences = other.m_Sequences;
	if(m_Sequences.size() > m_sndFile.GetModSpecifications().sequencesMax)
		m_Sequences.resize(m_sndFile.GetModSpecifications().sequencesMax, ModSequence{m_sndFile});
	if(m_currentSeq >= m_Sequences.size())
		m_currentSeq = 0;
	return *this;
}


void ModSequenceSet::Initialize()
{
	m_currentSeq = 0;
	m_Sequences.assign(1, ModSequence(m_sndFile));
}


void ModSequenceSet::SetSequence(SEQUENCEINDEX n) noexcept
{
	if(n < m_Sequences.size())
		m_currentSeq = n;
}


SEQUENCEINDEX ModSequenceSet::AddSequence()
{
	if(GetNumSequences() >= MAX_SEQUENCES)
		return SEQUENCEINDEX_INVALID;
	m_Sequences.push_back(ModSequence{m_sndFile});
	SetSequence(GetNumSequences() - 1);
	return GetNumSequences() - 1;
}


void ModSequenceSet::RemoveSequence(SEQUENCEINDEX i)
{
	// Do nothing if index is invalid or if there's only one sequence left.
	if(i >= m_Sequences.size() || m_Sequences.size() <= 1)
		return;
	m_Sequences.erase(m_Sequences.begin() + i);
	if(i < m_currentSeq || m_currentSeq >= GetNumSequences())
		m_currentSeq--;
}


#ifdef MODPLUG_TRACKER

bool ModSequenceSet::Rearrange(const std::vector<SEQUENCEINDEX> &newOrder)
{
	if(newOrder.empty() || newOrder.size() > MAX_SEQUENCES)
		return false;

	const auto oldSequences = std::move(m_Sequences);
	m_Sequences.assign(newOrder.size(), ModSequence{m_sndFile});
	for(size_t i = 0; i < newOrder.size(); i++)
	{
		if(newOrder[i] < oldSequences.size())
			m_Sequences[i] = oldSequences[newOrder[i]];
	}

	if(m_currentSeq > m_Sequences.size())
		m_currentSeq = GetNumSequences() - 1u;
	return true;
}


void ModSequenceSet::OnModTypeChanged(MODTYPE oldType)
{
	for(auto &seq : m_Sequences)
	{
		seq.AdjustToNewModType(oldType);
	}
	if(m_sndFile.GetModSpecifications(oldType).sequencesMax > 1 && m_sndFile.GetModSpecifications().sequencesMax <= 1)
		MergeSequences();
}


bool ModSequenceSet::CanSplitSubsongs() const noexcept
{
	return GetNumSequences() == 1 && m_sndFile.GetModSpecifications().sequencesMax > 1 && m_Sequences[0].HasSubsongs();
}


bool ModSequenceSet::SplitSubsongsToMultipleSequences()
{
	if(!CanSplitSubsongs())
		return false;

	bool modified = false;
	const ORDERINDEX length = m_Sequences[0].GetLengthTailTrimmed();

	for(ORDERINDEX ord = 0; ord < length; ord++)
	{
		// End of subsong?
		if(!m_Sequences[0].IsValidPat(ord) && m_Sequences[0][ord] != PATTERNINDEX_SKIP)
		{
			// Remove all separator patterns between current and next subsong first
			while(ord < length && !m_sndFile.Patterns.IsValidPat(m_Sequences[0][ord]))
			{
				m_Sequences[0][ord] = PATTERNINDEX_INVALID;
				ord++;
				modified = true;
			}
			if(ord >= length)
				break;

			const SEQUENCEINDEX newSeq = AddSequence();
			if(newSeq == SEQUENCEINDEX_INVALID)
				break;

			const ORDERINDEX startOrd = ord;
			m_Sequences[newSeq].reserve(length - startOrd);
			modified = true;

			// Now, move all following orders to the new sequence
			while(ord < length && m_Sequences[0][ord] != PATTERNINDEX_INVALID)
			{
				PATTERNINDEX copyPat = m_Sequences[0][ord];
				m_Sequences[newSeq].push_back(copyPat);
				m_Sequences[0][ord] = PATTERNINDEX_INVALID;
				ord++;

				// Is this a valid pattern? adjust pattern jump commands, if necessary.
				if(m_sndFile.Patterns.IsValidPat(copyPat))
				{
					for(auto &m : m_sndFile.Patterns[copyPat])
					{
						if(m.command == CMD_POSITIONJUMP && m.param >= startOrd)
						{
							m.param = static_cast<ModCommand::PARAM>(m.param - startOrd);
						}
					}
				}
			}
			ord--;
		}
	}
	SetSequence(0);
	return modified;
}


// Convert the sequence's restart position and tempo information to a pattern command.
bool ModSequenceSet::WriteGlobalsToPattern(SEQUENCEINDEX seq, bool writeRestartPos, bool writeTempo)
{
	bool result = true;
	auto length = m_sndFile.GetLength(eNoAdjust, GetLengthTarget(true).StartPos(seq, 0, 0));
	ModSequence &order = m_Sequences[seq];
	for(const auto &subSong : length)
	{
		if(writeRestartPos && subSong.endOrder != ORDERINDEX_INVALID && subSong.endRow != ROWINDEX_INVALID)
		{
			if(mpt::in_range<ModCommand::PARAM>(order.GetRestartPos()))
			{
				PATTERNINDEX writePat = order.EnsureUnique(subSong.endOrder);
				result &= m_sndFile.Patterns[writePat].WriteEffect(
					EffectWriter(CMD_POSITIONJUMP, static_cast<ModCommand::PARAM>(order.GetRestartPos())).Row(subSong.endRow).RetryNextRow());
			} else
			{
				result = false;
			}
		}
		if(writeTempo && subSong.startOrder != ORDERINDEX_INVALID && subSong.startRow != ORDERINDEX_INVALID)
		{
			PATTERNINDEX writePat = order.EnsureUnique(subSong.startOrder);
			result &= m_sndFile.Patterns[writePat].WriteEffect(
				EffectWriter(CMD_TEMPO, mpt::saturate_round<ModCommand::PARAM>(order.GetDefaultTempo().ToDouble())).Row(subSong.startRow).RetryNextRow());
			result &= m_sndFile.Patterns[writePat].WriteEffect(
				EffectWriter(CMD_SPEED, mpt::saturate_cast<ModCommand::PARAM>(order.GetDefaultSpeed())).Row(subSong.startRow).RetryNextRow());
		}
	}
	order.SetRestartPos(0);
	return result;
}


bool ModSequenceSet::MergeSequences()
{
	if(GetNumSequences() <= 1)
		return false;

	ModSequence &firstSeq = m_Sequences[0];
	firstSeq.resize(firstSeq.GetLengthTailTrimmed());
	std::vector<SEQUENCEINDEX> patternsFixed(m_sndFile.Patterns.Size(), SEQUENCEINDEX_INVALID); // pattern fixed by other sequence already?
	// Mark patterns handled in first sequence
	for(auto pat : firstSeq)
	{
		if(m_sndFile.Patterns.IsValidPat(pat))
			patternsFixed[pat] = 0;
	}

	for(SEQUENCEINDEX seqNum = 1; seqNum < GetNumSequences(); seqNum++)
	{
		ModSequence &sourceSeq = m_Sequences[seqNum];
		const ORDERINDEX firstOrder = firstSeq.GetLength() + 1; // +1 for separator item
		const ORDERINDEX lengthTrimmed = sourceSeq.GetLengthTailTrimmed();
		if(firstOrder + lengthTrimmed > m_sndFile.GetModSpecifications().ordersMax)
		{
			m_sndFile.AddToLog(LogWarning, MPT_UFORMAT("WARNING: Cannot merge Sequence {} (too long!)")(seqNum + 1));
			continue;
		}
		firstSeq.reserve(firstOrder + lengthTrimmed);
		firstSeq.push_back(); // Separator item
		WriteGlobalsToPattern(seqNum, true, sourceSeq.GetDefaultTempo() != firstSeq.GetDefaultTempo() || sourceSeq.GetDefaultSpeed() != firstSeq.GetDefaultSpeed());
		patternsFixed.resize(m_sndFile.Patterns.Size(), SEQUENCEINDEX_INVALID);  // Previous line might have increased pattern count
		for(ORDERINDEX ord = 0; ord < lengthTrimmed; ord++)
		{
			PATTERNINDEX pat = sourceSeq[ord];
			firstSeq.push_back(pat);

			// Try to fix pattern jump commands
			if(!m_sndFile.Patterns.IsValidPat(pat)) continue;

			auto m = m_sndFile.Patterns[pat].begin();
			for(size_t len = 0; len < m_sndFile.Patterns[pat].GetNumRows() * m_sndFile.GetNumChannels(); m++, len++)
			{
				if(m->command == CMD_POSITIONJUMP)
				{
					if(patternsFixed[pat] != SEQUENCEINDEX_INVALID && patternsFixed[pat] != seqNum)
					{
						// Oops, some other sequence uses this pattern already.
						const PATTERNINDEX newPat = m_sndFile.Patterns.Duplicate(pat, true);
						if(newPat != PATTERNINDEX_INVALID)
						{
							// Could create new pattern - copy data over and continue from here.
							firstSeq[firstOrder + ord] = newPat;
							m = m_sndFile.Patterns[newPat].begin() + len;
							if(newPat >= patternsFixed.size())
								patternsFixed.resize(newPat + 1, SEQUENCEINDEX_INVALID);
							pat = newPat;
						} else
						{
							// Cannot create new pattern: notify the user
							m_sndFile.AddToLog(LogWarning, MPT_UFORMAT("CONFLICT: Pattern break commands in Pattern {} might be broken since it has been used in several sequences!")(pat));
						}
					}
					m->param = static_cast<ModCommand::PARAM>(m->param + firstOrder);
					patternsFixed[pat] = seqNum;
				}
			}
		}
	}
	m_Sequences.erase(m_Sequences.begin() + 1, m_Sequences.end());
	m_currentSeq = 0;
	firstSeq.SetName({});
	return true;
}


// Check if a playback position is currently locked (inaccessible)
bool ModSequence::IsPositionLocked(ORDERINDEX position) const noexcept
{
	return(m_sndFile.m_lockOrderStart != ORDERINDEX_INVALID
		&& (position < m_sndFile.m_lockOrderStart || position > m_sndFile.m_lockOrderEnd));
}


bool ModSequence::HasSubsongs() const noexcept
{
	const auto endPat = begin() + GetLengthTailTrimmed();
	return std::find_if(begin(), endPat,
		[&](PATTERNINDEX pat) { return pat != PATTERNINDEX_SKIP && !m_sndFile.Patterns.IsValidPat(pat); }) != endPat;
}
#endif // MODPLUG_TRACKER


/////////////////////////////////////
// Read/Write
/////////////////////////////////////


#ifndef MODPLUG_NO_FILESAVE
size_t ModSequence::WriteAsByte(std::ostream &f, const ORDERINDEX count, uint8 stopIndex, uint8 ignoreIndex) const
{
	const size_t limit = std::min(count, GetLength());

	for(size_t i = 0; i < limit; i++)
	{
		const PATTERNINDEX pat = (*this)[i];
		uint8 temp = static_cast<uint8>(pat);

		if(pat == PATTERNINDEX_INVALID)
			temp = stopIndex;
		else if(pat == PATTERNINDEX_SKIP || pat > 0xFF)
			temp = ignoreIndex;
		mpt::IO::WriteIntLE<uint8>(f, temp);
	}
	// Fill non-existing order items with stop indices
	for(size_t i = limit; i < count; i++)
	{
		mpt::IO::WriteIntLE<uint8>(f, stopIndex);
	}
	return count; //Returns the number of bytes written.
}
#endif // MODPLUG_NO_FILESAVE


void ReadModSequenceOld(std::istream& iStrm, ModSequenceSet& seq, const size_t)
{
	uint16 size;
	mpt::IO::ReadIntLE<uint16>(iStrm, size);
	if(size > ModSpecs::mptm.ordersMax)
	{
		seq.m_sndFile.AddToLog(LogWarning, MPT_UFORMAT("Module has sequence of length {}; it will be truncated to maximum supported length, {}.")(size, ModSpecs::mptm.ordersMax));
		size = ModSpecs::mptm.ordersMax;
	}
	seq(0).resize(size);
	for(auto &pat : seq(0))
	{
		uint16 temp;
		mpt::IO::ReadIntLE<uint16>(iStrm, temp);
		pat = temp;
	}
}


#ifndef MODPLUG_NO_FILESAVE
void WriteModSequenceOld(std::ostream& oStrm, const ModSequenceSet& seq)
{
	const uint16 size = seq().GetLength();
	mpt::IO::WriteIntLE<uint16>(oStrm, size);
	for(auto pat : seq())
	{
		mpt::IO::WriteIntLE<uint16>(oStrm, static_cast<uint16>(pat));
	}
}
#endif // MODPLUG_NO_FILESAVE


#ifndef MODPLUG_NO_FILESAVE
void WriteModSequence(std::ostream& oStrm, const ModSequence& seq)
{
	srlztn::SsbWrite ssb(oStrm);
	ssb.BeginWrite(FileIdSequence, Version::Current().GetRawVersion());
	int8 useUTF8 = 1;
	ssb.WriteItem(useUTF8, "u");
	ssb.WriteItem(mpt::ToCharset(mpt::Charset::UTF8, seq.GetName()), "n");
	const uint16 length = seq.GetLengthTailTrimmed();
	ssb.WriteItem<uint16>(length, "l");
	ssb.WriteItem(seq, "a", srlztn::VectorWriter<uint16>(length));
	if(seq.GetRestartPos() > 0)
		ssb.WriteItem<uint16>(seq.GetRestartPos(), "r");
	ssb.WriteItem<TEMPO::store_t>(seq.GetDefaultTempo().GetRaw(), "t");
	ssb.WriteItem<uint32>(seq.GetDefaultSpeed(), "s");
	ssb.FinishWrite();
}
#endif // MODPLUG_NO_FILESAVE


void ReadModSequence(std::istream& iStrm, ModSequence& seq, const size_t, mpt::Charset defaultCharset)
{
	srlztn::SsbRead ssb(iStrm);
	ssb.BeginRead(FileIdSequence, Version::Current().GetRawVersion());
	if(ssb.HasFailed())
	{
		return;
	}
	int8 useUTF8 = 0;
	ssb.ReadItem(useUTF8, "u");
	std::string str;
	ssb.ReadItem(str, "n");
	seq.SetName(mpt::ToUnicode(useUTF8 ? mpt::Charset::UTF8 : defaultCharset, str));
	ORDERINDEX nSize = 0;
	ssb.ReadItem(nSize, "l");
	LimitMax(nSize, ModSpecs::mptm.ordersMax);
	ssb.ReadItem(seq, "a", srlztn::VectorReader<uint16>(nSize));

	ORDERINDEX restartPos = ORDERINDEX_INVALID;
	if(ssb.ReadItem(restartPos, "r") && restartPos < nSize)
	{
		seq.SetRestartPos(restartPos);
	}

	TEMPO::store_t defaultTempo = 0;
	if(ssb.ReadItem(defaultTempo, "t") && defaultTempo > 0)
	{
		seq.SetDefaultTempo(TEMPO{}.SetRaw(defaultTempo));
	}

	uint32 defaultSpeed = 0;
	if(ssb.ReadItem(defaultSpeed, "s") && defaultSpeed > 0)
	{
		seq.SetDefaultSpeed(defaultSpeed);
	}
}


#ifndef MODPLUG_NO_FILESAVE
void WriteModSequences(std::ostream& oStrm, const ModSequenceSet& seq)
{
	srlztn::SsbWrite ssb(oStrm);
	ssb.BeginWrite(FileIdSequences, Version::Current().GetRawVersion());
	const uint8 nSeqs = seq.GetNumSequences();
	const uint8 nCurrent = seq.GetCurrentSequenceIndex();
	ssb.WriteItem(nSeqs, "n");
	ssb.WriteItem(nCurrent, "c");
	for(uint8 i = 0; i < nSeqs; i++)
	{
		ssb.WriteItem(seq(i), srlztn::ID::FromInt<uint8>(i), &WriteModSequence);
	}
	ssb.FinishWrite();
}
#endif // MODPLUG_NO_FILESAVE


void ReadModSequences(std::istream& iStrm, ModSequenceSet& seq, const size_t, mpt::Charset defaultCharset)
{
	srlztn::SsbRead ssb(iStrm);
	ssb.BeginRead(FileIdSequences, Version::Current().GetRawVersion());
	if(ssb.HasFailed())
	{
		return;
	}
	SEQUENCEINDEX seqs = 0;
	uint8 currentSeq = 0;
	ssb.ReadItem(seqs, "n");
	if (seqs == 0)
		return;
	LimitMax(seqs, MAX_SEQUENCES);
	ssb.ReadItem(currentSeq, "c");
	if (seq.GetNumSequences() < seqs)
		seq.m_Sequences.resize(seqs, ModSequence(seq.m_sndFile));

	// There used to be only one restart position / tempo / speed for all sequences
	const auto legacyRestartPos = seq(0).GetRestartPos();
	const auto legacyTempo = seq(0).GetDefaultTempo();
	const auto legacySpeed = seq(0).GetDefaultSpeed();

	for(SEQUENCEINDEX i = 0; i < seqs; i++)
	{
		seq(i).SetRestartPos(legacyRestartPos);
		seq(i).SetDefaultTempo(legacyTempo);
		seq(i).SetDefaultSpeed(legacySpeed);
		ssb.ReadItem(seq(i), srlztn::ID::FromInt<uint8>(i), [defaultCharset](std::istream &iStrm, ModSequence &seq, std::size_t dummy) { return ReadModSequence(iStrm, seq, dummy, defaultCharset); });
	}
	seq.m_currentSeq = (currentSeq < seq.GetNumSequences()) ? currentSeq : 0;
}


OPENMPT_NAMESPACE_END
