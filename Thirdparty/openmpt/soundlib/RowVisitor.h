/*
 * RowVisitor.h
 * ------------
 * Purpose: Class for recording which rows of a song has already been visited, used for detecting when a module starts to loop.
 * Notes  : See implementation file.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/span.hpp"
#include "Snd_defs.h"

#include <map>

OPENMPT_NAMESPACE_BEGIN

#if defined(MPT_BUILD_DEBUG) || defined(MPT_BUILD_FUZZER)
#define MPT_VERIFY_ROWVISITOR_LOOPSTATE
#endif  // MPT_BUILD_DEBUG || MPT_BUILD_FUZZER

class CSoundFile;
class ModSequence;
struct ModChannel;

class RowVisitor
{
protected:
	using ChannelStates = mpt::span<const ModChannel>;

	class LoopState
	{
		static constexpr uint64 FNV1a_BASIS = 14695981039346656037ull;
		static constexpr uint64 FNV1a_PRIME = 1099511628211ull;
		uint64 m_hash = FNV1a_BASIS;
#ifdef MPT_VERIFY_ROWVISITOR_LOOPSTATE
		std::vector<std::pair<uint8, uint8>> m_counts;  // Actual loop counts to verify equality of hash-based implementation
#endif

	public:
		LoopState() = default;
		LoopState(const ChannelStates &chnState, const bool ignoreRow);
		LoopState(const LoopState &) = default;
		LoopState(LoopState&&) = default;
		LoopState &operator=(const LoopState &) = default;
		LoopState &operator=(LoopState&&) = default;

		[[nodiscard]] bool operator==(const LoopState &other) const noexcept
		{
#ifdef MPT_VERIFY_ROWVISITOR_LOOPSTATE
			if((m_counts == other.m_counts) != (m_hash == other.m_hash))
				std::abort();
#endif
			return m_hash == other.m_hash;
		}
		
		[[nodiscard]] bool HasLoops() const noexcept
		{
#ifdef MPT_VERIFY_ROWVISITOR_LOOPSTATE
			if(m_counts.empty() != (m_hash == FNV1a_BASIS))
				std::abort();
#endif
			return m_hash != FNV1a_BASIS;
		}
	};

	using LoopStateSet = std::vector<LoopState>;

	// Stores for every (order, row) combination in the sequence if it has been visited or not.
	std::vector<std::vector<bool>> m_visitedRows;
	// Map for each row that's part of a pattern loop which loop states have been visited. Held in a separate data structure because it is sparse data in typical modules.
	std::map<std::pair<ORDERINDEX, ROWINDEX>, LoopStateSet> m_visitedLoopStates;

	const CSoundFile &m_sndFile;
	ROWINDEX m_rowsSpentInLoops = 0;
	const SEQUENCEINDEX m_sequence;

public:
	RowVisitor(const CSoundFile &sndFile, SEQUENCEINDEX sequence = SEQUENCEINDEX_INVALID);
	
	void MoveVisitedRowsFrom(RowVisitor &other) noexcept;

	// Resize / Clear the row vector.
	// If reset is true, the vector is not only resized to the required dimensions, but also completely cleared (i.e. all visited rows are unset).
	void Initialize(bool reset);

	// Mark an order/row combination as visited and returns true if it was visited before.
	bool Visit(ORDERINDEX ord, ROWINDEX row, const ChannelStates &chnState, bool ignoreRow);

	// Find the first row that has not been played yet.
	// The order and row is stored in the order and row variables on success, on failure they contain invalid values.
	// If onlyUnplayedPatterns is true (default), only completely unplayed patterns are considered, otherwise a song can start anywhere.
	// Function returns true on success.
	[[nodiscard]] bool GetFirstUnvisitedRow(ORDERINDEX &order, ROWINDEX &row, bool onlyUnplayedPatterns) const;

	// Pattern loops can stack up exponentially, which can cause an effectively infinite amount of time to be spent on evaluating them.
	// If this function returns true, module evaluation should be aborted because the pattern loops appear to be too complex.
	[[nodiscard]] bool ModuleTooComplex(ROWINDEX threshold) const noexcept { return m_rowsSpentInLoops >= threshold; }
	void ResetComplexity() { m_rowsSpentInLoops = 0; }

protected:
	// Get the needed vector size for a given pattern.
	[[nodiscard]] ROWINDEX VisitedRowsVectorSize(PATTERNINDEX pattern) const noexcept;

	[[nodiscard]] const ModSequence &Order() const;
};

OPENMPT_NAMESPACE_END
