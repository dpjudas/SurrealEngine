/*
 * MIDIMacroParser.h
 * -----------------
 * Purpose: Class for parsing IT MIDI macro strings and splitting them into individual raw MIDI messages.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "Snd_defs.h"

OPENMPT_NAMESPACE_BEGIN

class CSoundFile;
struct PlayState;

class MIDIMacroParser
{
public:
	// Parse the given MIDI macro into the out span. out needs to be at least one byte longer than the input string to support the longest possible macro translation.
	MIDIMacroParser(const CSoundFile &sndFile, PlayState *playState, CHANNELINDEX nChn, bool isSmooth, const mpt::span<const char> macro, mpt::span<uint8> out, uint8 param = 0, PLUGINDEX plugin = 0);
	// Split a raw MIDI dump into multiple messages. Note that in order to support running status, NextMessage() may temporarily alter the provided data.
	// When the MIDIMacroParser destructor has run, the data will be back in its original state.
	MIDIMacroParser(mpt::span<uint8> data) : m_data{data} {}
	~MIDIMacroParser();

	bool NextMessage(mpt::span<uint8> &message, bool outputRunningStatus = true);

private:
	mpt::span<uint8> m_data;
	uint32 m_sendPos = 0;
	uint32 m_runningStatusPos = uint32_max;
	uint8 m_runningStatus = 0;
	uint8 m_runningstatusOldData = 0;
};

OPENMPT_NAMESPACE_END
