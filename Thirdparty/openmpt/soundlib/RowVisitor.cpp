/*
 * RowVisitor.cpp
 * --------------
 * Purpose: Class for recording which rows of a song has already been visited, used for detecting when a module starts to loop.
 * Notes  : The class keeps track of rows that have been visited by the player before.
 *          This way, we can tell when the module starts to loop, i.e. we can determine the song length,
 *          or find out that a given point of the module can never be reached.
 *
 *          In some module formats, infinite loops can be achieved through pattern loops (e.g. E60 / E61 / E61 in one channel of a ProTracker MOD).
 *          To detect such loops, we store a set of loop counts across all channels encountered for each row.
 *          As soon as a set of loop counts is encountered twice for a specific row, we know that the track ends up in an infinite loop.
 *          As a result of this design, it is safe to evaluate pattern loops in CSoundFile::GetLength.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "RowVisitor.h"
#include "Sndfile.h"

OPENMPT_NAMESPACE_BEGIN

RowVisitor::LoopState::LoopState(const ChannelStates &chnState, const bool ignoreRow)
{
	// Rather than storing the exact loop count vector, we compute an FNV-1a 64-bit hash of it.
	// This means we can store the loop state in a small and fixed amount of memory.
	// In theory there is the possibility of hash collisions for different loop states, but in practice,
	// the relevant inputs for the hashing algorithm are extremely unlikely to produce collisions.
	// There may be better hashing algorithms, but many of them are much more complex and cannot be applied easily in an incremental way.
	uint64 hash = FNV1a_BASIS;
	if(ignoreRow)
	{
		hash = (hash ^ 0xFFu) * FNV1a_PRIME;
#ifdef MPT_VERIFY_ROWVISITOR_LOOPSTATE
		m_counts.emplace_back(uint8(0xFF), uint8(0xFF));
#endif
	}

	for(size_t chn = 0; chn < chnState.size(); chn++)
	{
		if(chnState[chn].nPatternLoopCount)
		{
			static_assert(MAX_BASECHANNELS <= 256, "Channel index cannot be used as byte input for hash generator");
			static_assert(sizeof(chnState[0].nPatternLoopCount) <= sizeof(uint8), "Loop count cannot be used as byte input for hash generator");
			hash = (hash ^ chn) * FNV1a_PRIME;
			hash = (hash ^ chnState[chn].nPatternLoopCount) * FNV1a_PRIME;
#ifdef MPT_VERIFY_ROWVISITOR_LOOPSTATE
			m_counts.emplace_back(static_cast<uint8>(chn), chnState[chn].nPatternLoopCount);
#endif
		}
	}
	m_hash = hash;
}


RowVisitor::RowVisitor(const CSoundFile &sndFile, SEQUENCEINDEX sequence)
    : m_sndFile(sndFile)
    , m_sequence(sequence)
{
	Initialize(true);
}


void RowVisitor::MoveVisitedRowsFrom(RowVisitor &other) noexcept
{
	m_visitedRows = std::move(other.m_visitedRows);
	m_visitedLoopStates = std::move(other.m_visitedLoopStates);
}


const ModSequence &RowVisitor::Order() const
{
	if(m_sequence >= m_sndFile.Order.GetNumSequences())
		return m_sndFile.Order();
	else
		return m_sndFile.Order(m_sequence);
}


// Resize / clear the row vector.
// If reset is true, the vector is not only resized to the required dimensions, but also completely cleared (i.e. all visited rows are reset).
void RowVisitor::Initialize(bool reset)
{
	auto &order = Order();
	const ORDERINDEX endOrder = order.GetLengthTailTrimmed();
	bool reserveLoopStates = true;
	m_visitedRows.resize(endOrder);
	if(reset)
	{
		reserveLoopStates = m_visitedLoopStates.empty();
		for(auto &loopState : m_visitedLoopStates)
		{
			loopState.second.clear();
		}
		m_rowsSpentInLoops = 0;
	}

	std::vector<uint8> loopCount;
	std::vector<ORDERINDEX> visitedPatterns(m_sndFile.Patterns.GetNumPatterns(), ORDERINDEX_INVALID);
	for(ORDERINDEX ord = 0; ord < endOrder; ord++)
	{
		const PATTERNINDEX pat = order[ord];
		const ROWINDEX numRows = VisitedRowsVectorSize(pat);
		auto &visitedRows = m_visitedRows[ord];

		if(reset)
			visitedRows.assign(numRows, false);
		else
			visitedRows.resize(numRows, false);

		if(!reserveLoopStates || !order.IsValidPat(ord))
			continue;

		const ROWINDEX startRow = std::min(static_cast<ROWINDEX>(reset ? 0 : visitedRows.size()), numRows);
		auto insertionHint = m_visitedLoopStates.end();

		if(visitedPatterns[pat] != ORDERINDEX_INVALID)
		{
			// We visited this pattern before, copy over the results
			const auto begin = m_visitedLoopStates.lower_bound({visitedPatterns[pat], startRow});
			const auto end = (begin != m_visitedLoopStates.end()) ? m_visitedLoopStates.lower_bound({visitedPatterns[pat], numRows}) : m_visitedLoopStates.end();
			for(auto pos = begin; pos != end; ++pos)
			{
				LoopStateSet loopStates;
				loopStates.reserve(pos->second.capacity());
				insertionHint = ++m_visitedLoopStates.insert_or_assign(insertionHint, {ord, pos->first.second}, std::move(loopStates));
			}
			continue;
		}

		// Pre-allocate loop count state
		const auto &pattern = m_sndFile.Patterns[pat];
		loopCount.assign(pattern.GetNumChannels(), 0);
		for(ROWINDEX i = numRows; i != startRow; i--)
		{
			const ROWINDEX row = i - 1;
			uint32 maxLoopStates = 1;
			auto m = pattern.GetpModCommand(row, 0);
			// Break condition: If it's more than 16, it's probably wrong :) exact loop count depends on how loops overlap.
			for(CHANNELINDEX chn = 0; chn < pattern.GetNumChannels() && maxLoopStates < 16; chn++, m++)
			{
				auto count = loopCount[chn];
				if((m->command == CMD_S3MCMDEX && (m->param & 0xF0) == 0xB0) || (m->command == CMD_MODCMDEX && (m->param & 0xF0) == 0x60))
				{
					loopCount[chn] = (m->param & 0x0F);
					if(loopCount[chn])
						count = loopCount[chn];
				}
				if(count)
					maxLoopStates *= (count + 1);
			}
			if(maxLoopStates > 1)
			{
				LoopStateSet loopStates;
				loopStates.reserve(maxLoopStates);
				insertionHint = m_visitedLoopStates.insert_or_assign(insertionHint, {ord, row}, std::move(loopStates));
			}
		}
		// Only use this order as a blueprint for other orders using the same pattern if we fully parsed the pattern.
		if(startRow == 0)
			visitedPatterns[pat] = ord;
	}
}


// Mark an order/row combination as visited and returns true if it was visited before.
bool RowVisitor::Visit(ORDERINDEX ord, ROWINDEX row, const ChannelStates &chnState, bool ignoreRow)
{
	auto &order = Order();
	if(ord >= order.size() || row >= VisitedRowsVectorSize(order[ord]))
		return false;

	// The module might have been edited in the meantime - so we have to extend this a bit.
	if(ord >= m_visitedRows.size() || row >= m_visitedRows[ord].size())
	{
		Initialize(false);
		// If it's still past the end of the vector, this means that ord >= order.GetLengthTailTrimmed(), i.e. we are trying to play an empty order.
		if(ord >= m_visitedRows.size())
			return false;
	}

	MPT_ASSERT(chnState.size() >= m_sndFile.GetNumChannels());
	LoopState newState{chnState.first(m_sndFile.GetNumChannels()), ignoreRow};
	const auto rowLoopState = m_visitedLoopStates.find({ord, row});
	const bool oldHadLoops = (rowLoopState != m_visitedLoopStates.end() && !rowLoopState->second.empty());
	const bool newHasLoops = newState.HasLoops();
	const bool wasVisited = m_visitedRows[ord][row];
	
	// Check if new state is part of row state already. If so, we visited this row already and thus the module must be looping
	if(!oldHadLoops && !newHasLoops && wasVisited)
		return true;
	if(oldHadLoops && mpt::contains(rowLoopState->second, newState))
		return true;

	if(newHasLoops)
		m_rowsSpentInLoops++;

	if(oldHadLoops || newHasLoops)
	{
		// Convert to set representation if it isn't already
		if(!oldHadLoops && wasVisited)
			m_visitedLoopStates[{ord, row}].emplace_back();
		m_visitedLoopStates[{ord, row}].emplace_back(std::move(newState));
	}
	m_visitedRows[ord][row] = true;
	return false;
}


// Get the needed vector size for a given pattern.
ROWINDEX RowVisitor::VisitedRowsVectorSize(PATTERNINDEX pattern) const noexcept
{
	if(m_sndFile.Patterns.IsValidPat(pattern))
		return m_sndFile.Patterns[pattern].GetNumRows();
	else
		return 1;  // Non-existing patterns consist of a "fake" row.
}


// Find the first row that has not been played yet.
// The order and row is stored in the order and row variables on success, on failure they contain invalid values.
// If onlyUnplayedPatterns is true (default), only completely unplayed patterns are considered, otherwise a song can start on any unplayed row.
// Function returns true on success.
bool RowVisitor::GetFirstUnvisitedRow(ORDERINDEX &ord, ROWINDEX &row, bool onlyUnplayedPatterns) const
{
	const auto &order = Order();
	const ORDERINDEX endOrder = order.GetLengthTailTrimmed();
	for(ORDERINDEX o = 0; o < endOrder; o++)
	{
		if(!order.IsValidPat(o))
			continue;

		if(o >= m_visitedRows.size())
		{
			// Not yet initialized => unvisited
			ord = o;
			row = 0;
			return true;
		}

		const auto &visitedRows = m_visitedRows[o];
		ROWINDEX firstUnplayedRow = 0;
		for(; firstUnplayedRow < visitedRows.size(); firstUnplayedRow++)
		{
			if(visitedRows[firstUnplayedRow] == onlyUnplayedPatterns)
				break;
		}
		if(onlyUnplayedPatterns && firstUnplayedRow == visitedRows.size())
		{
			// No row of this pattern has been played yet.
			ord = o;
			row = 0;
			return true;
		} else if(!onlyUnplayedPatterns)
		{
			// Return the first unplayed row in this pattern
			if(firstUnplayedRow < visitedRows.size())
			{
				ord = o;
				row = firstUnplayedRow;
				return true;
			}
			if(visitedRows.size() < m_sndFile.Patterns[order[o]].GetNumRows())
			{
				// History is not fully initialized
				ord = o;
				row = static_cast<ROWINDEX>(visitedRows.size());
				return true;
			}
		}
	}

	// Didn't find anything :(
	ord = ORDERINDEX_INVALID;
	row = ROWINDEX_INVALID;
	return false;
}


OPENMPT_NAMESPACE_END
