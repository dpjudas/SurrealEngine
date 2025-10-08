/*
 * PlaybackTest.h
 * --------------
 * Purpose: Tools for verifying correct playback of modules
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#if defined(MPT_ENABLE_PLAYBACK_TRACE)

#include "../common/FileReaderFwd.h"

#include <iosfwd>

#endif // MPT_ENABLE_PLAYBACK_TRACE

OPENMPT_NAMESPACE_BEGIN

#if defined(MPT_ENABLE_PLAYBACK_TRACE)

struct PlaybackTestData;
struct PlaybackTestSettings;
class CSoundFile;

class PlaybackTest
{
public:
	explicit PlaybackTest(FileReader file) noexcept(false);
	explicit PlaybackTest(PlaybackTestData &&testData);
	PlaybackTest(PlaybackTest &&other) noexcept;
	PlaybackTest(const PlaybackTest &) = delete;
	~PlaybackTest();

	PlaybackTest& operator=(PlaybackTest &&other) noexcept;
	PlaybackTest& operator=(const PlaybackTest &) = delete;

	void Deserialize(FileReader file) noexcept(false);
	void Serialize(std::ostream &output) const noexcept(false);
	void ToTSV(std::ostream &output) const noexcept(false);

	PlaybackTestSettings GetSettings() const noexcept;

	static std::vector<mpt::ustring> Compare(const PlaybackTest &lhs, const PlaybackTest &rhs);

private:
	std::vector<mpt::ustring> Compare(const PlaybackTest &otherTest) const;

private:
	std::unique_ptr<PlaybackTestData> m_testData;
};

#endif // MPT_ENABLE_PLAYBACK_TRACE

OPENMPT_NAMESPACE_END
