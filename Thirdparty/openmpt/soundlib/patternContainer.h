/*
 * PatternContainer.h
 * ------------------
 * Purpose: Container class for managing patterns.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "pattern.h"

#include <algorithm>

OPENMPT_NAMESPACE_BEGIN

class CSoundFile;

class CPatternContainer
{
public:
	CPattern& operator[](const int pat) { return m_Patterns[pat]; }
	const CPattern& operator[](const int pat) const { return m_Patterns[pat]; }

public:
	CPatternContainer(CSoundFile &sndFile) : m_rSndFile{sndFile} { }
	CPatternContainer(const CPatternContainer &) = default;
	CPatternContainer(CPatternContainer &&) noexcept = default;
	CPatternContainer &operator=(const CPatternContainer &other);
	CPatternContainer &operator=(CPatternContainer &&other) noexcept;

	// Delete all patterns.
	void DestroyPatterns();
	
	// Insert (default)pattern to given position. If pattern already exists at that position,
	// ignoring request. Returns true on success, false otherwise.
	bool Insert(const PATTERNINDEX index, const ROWINDEX rows);
	
	// Insert pattern to position with the lowest index, and return that index, PATTERNINDEX_INVALID on failure.
	// If respectQtyLimits is true, inserting patterns will fail if the resulting pattern index would exceed the current format's pattern quantity limits.
	PATTERNINDEX InsertAny(const ROWINDEX rows, bool respectQtyLimits = false);

	// Duplicate an existing pattern. Returns new pattern index on success, or PATTERNINDEX_INVALID on failure.
	// If respectQtyLimits is true, inserting patterns will fail if the resulting pattern index would exceed the current format's pattern quantity limits.
	PATTERNINDEX Duplicate(PATTERNINDEX from, bool respectQtyLimits = false);

	//Remove pattern from given position. Currently it actually makes the pattern
	//'invisible' - the pattern data is cleared but the actual pattern object won't get removed.
	void Remove(const PATTERNINDEX index);

	// Applies function object for modcommands in all patterns.
	template <class Func>
	void ForEachModCommand(Func func);
	template <class Func>
	void ForEachModCommand(Func func) const;

	std::vector<CPattern>::iterator begin() noexcept { return m_Patterns.begin(); }
	std::vector<CPattern>::const_iterator begin() const noexcept { return m_Patterns.begin(); }
	std::vector<CPattern>::const_iterator cbegin() const noexcept { return m_Patterns.cbegin(); }
	std::vector<CPattern>::iterator end() noexcept { return m_Patterns.end(); }
	std::vector<CPattern>::const_iterator end() const noexcept { return m_Patterns.end(); }
	std::vector<CPattern>::const_iterator cend() const noexcept { return m_Patterns.cend(); }

	PATTERNINDEX Size() const noexcept { return static_cast<PATTERNINDEX>(m_Patterns.size()); }

	CSoundFile& GetSoundFile() noexcept { return m_rSndFile; }
	const CSoundFile& GetSoundFile() const noexcept { return m_rSndFile; }

	// Return true if pattern can be accessed with operator[](iPat), false otherwise.
	bool IsValidIndex(const PATTERNINDEX iPat) const noexcept { return (iPat < Size()); }

	// Return true if IsValidIndex() is true and the corresponding pattern has allocated modcommand array, false otherwise.
	bool IsValidPat(const PATTERNINDEX iPat) const noexcept { return IsValidIndex(iPat) && m_Patterns[iPat].IsValid(); }

	// Returns true if the pattern is empty, i.e. there are no notes/effects in this pattern
	bool IsPatternEmpty(const PATTERNINDEX nPat) const noexcept;
	
	void ResizeArray(const PATTERNINDEX newSize);

	void OnModTypeChanged(const MODTYPE oldtype);

	// Returns index of last valid pattern + 1, zero if no such pattern exists.
	PATTERNINDEX GetNumPatterns() const noexcept;

	// Returns index of highest pattern with pattern named + 1.
	PATTERNINDEX GetNumNamedPatterns() const noexcept;

	// Number of patterns that can still be added, respecting the current format's limitations
	PATTERNINDEX GetRemainingCapacity() const noexcept;

private:
	std::vector<CPattern> m_Patterns;
	CSoundFile &m_rSndFile;
};


template <class Func>
void CPatternContainer::ForEachModCommand(Func func)
{
	for(auto &pattern : m_Patterns)
		std::for_each(pattern.begin(), pattern.end(), func);
}


template <class Func>
void CPatternContainer::ForEachModCommand(Func func) const
{
	for(const auto &pattern : m_Patterns)
		std::for_each(pattern.cbegin(), pattern.cend(), func);
}

const char FileIdPatterns[] = "mptPc";

void ReadModPatterns(std::istream& iStrm, CPatternContainer& patc, const size_t nSize = 0);
void WriteModPatterns(std::ostream& oStrm, const CPatternContainer& patc);


OPENMPT_NAMESPACE_END
