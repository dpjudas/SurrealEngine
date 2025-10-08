/*
 * PlaybackTest.cpp
 * ----------------
 * Purpose: Tools for verifying correct playback of modules
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#include "PlaybackTest.h"

#include "../common/mptBaseMacros.h"

#if defined(MPT_ENABLE_PLAYBACK_TRACE)

#include "../common/FileReader.h"
#include "OPL.h"
#include "SampleIO.h"
#include "Sndfile.h"

#include "mpt/base/bit.hpp"
#include "mpt/binary/hex.hpp"
#include "mpt/crc/crc.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "mpt/random/seed.hpp"

#include "openmpt/base/Endian.hpp"

#include <sstream>
#include <iomanip>


#endif // MPT_ENABLE_PLAYBACK_TRACE


OPENMPT_NAMESPACE_BEGIN


#if defined(MPT_ENABLE_PLAYBACK_TRACE)


struct TestDataHeader
{
	static constexpr char TestDataHeaderMagic[] = "OpenMPT Test Data\r\n\x1A";

	char     magic[std::size(TestDataHeaderMagic) - 1];
	uint8    fileVersion;
	uint8    isAmiga;
	uint8    positionPrecisionBits;
	uint8    filterPrecisionBits;
	uint8    srcMode;
	uint8    outputChannels;
	uint16le mixerChannels;
	uint16le numSamples;
	uint16le synthVolume;
	int32le  globalVolumeUnity;
	int32le  channelVolumeUnity;
	uint32le mixingFreq;
	uint32le mptVersion;
};

MPT_BINARY_STRUCT(TestDataHeader, 48)


struct TestDataRow
{
	uint32le order;
	uint32le row;
	int32le  globalVolume;
	uint32le tickLength;
	uint8    isFirstTick;
	uint16le activeChannels;
};

MPT_BINARY_STRUCT(TestDataRow, 19)


struct TestDataChannel
{
	enum Flags : uint8
	{
		kFilterNone     = 0x00,
		kSurround       = 0x01,
		kOPL            = 0x02,
		kAmigaFilter    = 0x04,
		kFilterLowPass  = 0x08,
		kFilterHighPass = 0x10,
		kFilterMask     = 0x18,
	};

	int16le  channel;  // Negative = NNA channel (-1 = NNA of first channel, -2 = NNA of second channel, etc.)
	uint16le nnaAge;   // Generation of NNA channels
	uint8    flags;
	uint8    srcMode;
	uint16le sample;
	int32le  leftVol;
	int32le  rightVol;
	int64le  increment;
	int64le  position;
	int32le  filterA0;
	int32le  filterB0;
	int32le  filterB1;

	bool operator<(const TestDataChannel &other) const noexcept
	{
		if(channel == -other.channel)  // One is an NNA channel of the other
			return channel > 0;
		else if(channel != other.channel)  // Completely unrelated channels
			return std::abs(channel) < std::abs(other.channel);
		else  // Both are NNA channels of the same source channel
			return nnaAge < other.nnaAge;
	}
};

MPT_BINARY_STRUCT(TestDataChannel, 44)


using SampleDataHashAlgorithm = mpt::crc64_jones;
using SampleDataHash = mpt::packed<decltype(SampleDataHashAlgorithm{}.result()), mpt::endian::little>;


struct PlaybackTestData
{
	struct Row
	{
		TestDataRow header;
		std::vector<TestDataChannel> channels;
		std::vector<uint8> oplRegisters;
	};

	TestDataHeader header;
	std::vector<SampleDataHash> sampleDataHashes;
	std::vector<Row> rows;

	PlaybackTestSettings GetSettings() const
	{
		PlaybackTestSettings result;
		result.mixingFreq = header.mixingFreq;
		result.outputChannels = header.outputChannels;
		result.mixerChannels = header.mixerChannels;
		result.srcMode = Resampling::ToKnownMode(header.srcMode);
		return result;
	}
};


class OPLPlaybackLog final : public OPL::IRegisterLogger
{
public:
	using OPLData = std::vector<uint8>;

	OPLPlaybackLog(const PlayState &playState) : m_playState{playState} {}

	void Reset()
	{
		m_registers.clear();
		m_prevRegisters.clear();
		m_keyOnToggle.clear();
		m_globalRegisters.clear();
		m_chnRegisters.clear();
		m_prevChnRegisters.clear();
	}

	OPLData DumpRegisters()
	{
		OPLData dump;

		// Dump registers for all channels
		for(const auto &[chn, registerDump] : m_chnRegisters)
		{
			bool first = true;
			const auto prevRegisters = m_prevChnRegisters.find(chn);
			for(const auto &[reg, value] : registerDump)
			{
				if(prevRegisters != m_prevChnRegisters.end())
				{
					const auto prevRegister = prevRegisters->second.find(reg);
					if(prevRegister != prevRegisters->second.end() && prevRegister->second == value)
						continue;
				}
				m_prevChnRegisters[chn][reg] = value;
				if(first)
				{
					const auto [sourceChn, nnaGeneration] = chn;
					MPT_ASSERT(sourceChn < uint8_max);  // nnaGeneration may be truncated but it doesn't matter, as it is purely for informational purposes
					dump.insert(dump.end(), {static_cast<uint8>(sourceChn), static_cast<uint8>(nnaGeneration)});

					// Was key-on toggled on this channel?
					dump.push_back(static_cast<uint8>(m_keyOnToggle.count(chn.first)));
					first = false;
				}
				dump.insert(dump.end(), {reg, value});
			}
			if(!first)
				dump.push_back(uint8_max);
		}
		// Dump global register updates
		bool first = true;
		for(const auto reg : m_globalRegisters)
		{
			if(!m_prevRegisters.count(reg) || m_registers[reg] != m_prevRegisters[reg])
			{
				if(first)
				{
					dump.insert(dump.end(), {uint8_max, uint8_max});
					first = false;
				}
				dump.insert(dump.end(), {static_cast<uint8>(reg & 0xFF), static_cast<uint8>(reg >> 8), m_registers[reg]});
			}
		}

		for(const auto &[reg, value] : m_registers)
		{
			m_prevRegisters[reg] = value;
		}

		m_chnRegisters.clear();
		m_globalRegisters.clear();
		m_registers.clear();
		m_keyOnToggle.clear();

		return dump;
	}

#if MPT_GCC_AT_LEAST(12, 0, 0) && MPT_GCC_BEFORE(13, 1, 0)
// Work-around <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105329> /
// <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105651>.
#pragma GCC push_options
#if defined(__OPTIMIZE__)
#pragma GCC optimize("O1")
#endif
#endif
	static std::string Format(const OPLData &data)
	{
		FileReader file(mpt::as_span(data));
		std::string result;
		while(file.CanRead(3))
		{
			const auto [chn, generation] = file.ReadArray<uint8, 2>();
			if(chn == uint8_max && generation == uint8_max)
				break;

			result += "[Ch " + mpt::afmt::val(chn);
			if(generation)
				result += ":" + mpt::afmt::val(generation);
			result += "] ";

			if(file.ReadUint8())
				result += "[Toggle] ";

			while(file.CanRead(2))
			{
				const uint8 reg = file.ReadUint8();
				if(reg == uint8_max)
					break;
				const uint8 value = file.ReadUint8();
				result += mpt::afmt::HEX0<2>(reg) + "=" + mpt::afmt::HEX0<2>(value) + " ";
			}
		}
		if(!file.CanRead(3))
			return result;
		result += "[Global] ";
		while(file.CanRead(3))
		{
			const uint16 reg = file.ReadUint16LE();
			const uint8 value = file.ReadUint8();
			result += mpt::afmt::HEX0<2>(reg) + "=" + mpt::afmt::HEX0<2>(value) + " ";
		}
		return result;
	}
#if MPT_GCC_AT_LEAST(12, 0, 0) && MPT_GCC_BEFORE(13, 1, 0)
#pragma GCC diagnostic pop
#pragma GCC pop_options
#endif

protected:
	void Port(CHANNELINDEX c, OPL::Register reg, OPL::Value value) override
	{
		MPT_ASSERT((OPL::RegisterToVoice(reg) == 0xFF) == (c == CHANNELINDEX_INVALID));
		if(c != CHANNELINDEX_INVALID)
		{
			if((reg & 0xF0) == OPL::KEYON_BLOCK)
			{
				const auto prev = m_prevRegisters.find(reg);
				const auto current = m_registers.find(reg);
				if(prev != m_prevRegisters.end() && current != m_registers.end()
				   && (prev->second & OPL::KEYON_BIT) == (value & OPL::KEYON_BIT)
				   && (prev->second & OPL::KEYON_BIT) != (current->second & OPL::KEYON_BIT))
				{
					// Key-On was toggled off and on again within a single frame, which retriggers the note. This needs to be recorded.
					m_keyOnToggle.insert(c);
					m_prevRegisters.erase(reg);
				}
			}

			const auto voiceReg = OPL::StripVoiceFromRegister(reg);
			MPT_ASSERT(voiceReg <= uint8_max);

			std::pair<CHANNELINDEX, uint16> key{c, uint16(0)};
			if(m_playState.Chn[c].nMasterChn)
				key = {static_cast<CHANNELINDEX>(m_playState.Chn[c].nMasterChn - 1), m_playState.Chn[c].nnaGeneration};
			m_chnRegisters[key][static_cast<uint8>(voiceReg)] = value;
		} else
		{
			m_globalRegisters.insert(reg);
		}
		m_registers[reg] = value;
	}

	void MoveChannel(CHANNELINDEX from, CHANNELINDEX to) override
	{
		m_prevChnRegisters[{from, m_playState.Chn[to].nnaGeneration}] = std::move(m_prevChnRegisters[{from, uint16(0)}]);
	}

	std::map<OPL::Register, OPL::Value> m_registers;      // All registers that have been updated in the current tick
	std::map<OPL::Register, OPL::Value> m_prevRegisters;  // Previous state of all registers that have been set so far
	std::set<CHANNELINDEX> m_keyOnToggle;                 // Set of channels on which a key-on -> key-off -> key-on transition was made on this tick
	std::set<OPL::Register> m_globalRegisters;            // Set of all global registers that have been modified in the current tick

	std::map<std::pair<CHANNELINDEX, uint16>, std::map<uint8, uint8>> m_chnRegisters;      // Maps [source channel, age] to [voice register, voice value]
	std::map<std::pair<CHANNELINDEX, uint16>, std::map<uint8, uint8>> m_prevChnRegisters;  // Previous state of registers for this channel

	const PlayState &m_playState;
};


PlaybackTest::PlaybackTest(FileReader file) noexcept(false)
{
	Deserialize(file);
}

PlaybackTest::PlaybackTest(PlaybackTestData &&testData)
	: m_testData{std::make_unique<PlaybackTestData>(std::move(testData))}
{
}

PlaybackTest::PlaybackTest(PlaybackTest &&other) noexcept
	: m_testData{std::move(other.m_testData)}
{
}

PlaybackTest::~PlaybackTest()
{
	// This destructor is put here so that we can forward-declare the PlaybackTestData class.
}

PlaybackTest& PlaybackTest::operator=(PlaybackTest &&other) noexcept
{
	m_testData = std::move(other.m_testData);
	return *this;
}


void PlaybackTest::Deserialize(FileReader file) noexcept(false)
{

	file.Rewind();

	m_testData = std::make_unique<PlaybackTestData>();

	auto &header = m_testData->header;
	file.Read(header);
	if(memcmp(header.magic, TestDataHeader::TestDataHeaderMagic, sizeof(header.magic)))
		throw std::runtime_error{"Invalid test data file"};
	if(header.fileVersion != 0)
		throw std::runtime_error{"Invalid test data file version"};
	if(!Resampling::IsKnownMode(header.srcMode))
		throw std::runtime_error{"Invalid test data: SRC mode"};
	if(header.outputChannels <= 0)
		throw std::runtime_error{"Invalid test data: number of output channels"};
	if(header.mixerChannels <= 0)
		throw std::runtime_error{"Invalid test data: number of mixer channels"};
	if(header.mixingFreq <= 0)
		throw std::runtime_error{"Invalid test data: mixing frequency"};
	if(header.globalVolumeUnity <= 0)
		throw std::runtime_error{"Invalid test data: global volume unity"};
	if(header.channelVolumeUnity <= 0)
		throw std::runtime_error{"Invalid test data: channel volume unity"};

	file.ReadVector(m_testData->sampleDataHashes, header.numSamples);

	while(!file.EndOfFile())
	{
		auto &row = m_testData->rows.emplace_back();

		file.Read(row.header);
		file.ReadVector(row.channels, row.header.activeChannels);
		uint32 oplDataSize = 0;
		file.ReadVarInt(oplDataSize);
		file.ReadVector(row.oplRegisters, oplDataSize);
	}
}


void PlaybackTest::Serialize(std::ostream &output) const noexcept(false)
{
	mpt::IO::Write(output, m_testData->header);
	mpt::IO::Write(output, m_testData->sampleDataHashes);
	for(const auto &row : m_testData->rows)
	{
		mpt::IO::Write(output, row.header);
		mpt::IO::Write(output, row.channels);
		mpt::IO::WriteVarInt(output, row.oplRegisters.size());
		mpt::IO::Write(output, row.oplRegisters);
	}
}


void PlaybackTest::ToTSV(std::ostream &output) const noexcept(false)
{
	const auto &header = m_testData->header;
	const auto positionPrecision = 1.0 / (int64(1) << header.positionPrecisionBits);
	const auto filterPrecision = 1.0 / (int64(1) << header.filterPrecisionBits);

	static constexpr int floatPrecision = 10;
	const auto floatFormat = mpt::format_simple_spec<std::string>().SetPrecision(floatPrecision);
	output << std::setprecision(floatPrecision);

	output << "OpenMPT Test Data version " << mpt::afmt::val(header.fileVersion) << "\n"
		"Created by OpenMPT " << mpt::ToCharset(mpt::Charset::UTF8, Version::Current().ToUString()) << "\n"
		"isAmiga\t" << mpt::afmt::val(header.isAmiga) << "\n"
		"srcMode\t" << mpt::afmt::val(header.srcMode) << "\n"
		"outputChannels\t" << mpt::afmt::val(header.outputChannels) << "\n"
		"mixerChannels\t" << header.mixerChannels << "\n"
		"synthVolume\t" << header.synthVolume << "\n"
		"mixingFreq\t" << header.mixingFreq << "\n"
		"\nSample data hashes:\n";

	for(SAMPLEINDEX smp = 1; smp <= header.numSamples; smp++)
		output << mpt::ToCharset(mpt::Charset::UTF8, mpt::encode_hex(mpt::as_raw_memory(m_testData->sampleDataHashes[smp - 1]))) << "\t" << smp << "\n";

	output << "\nChannel data:\n"
		"index\torder\trow\ttick\tglobalVolume\ttickLength\tchannel\tsample\tleftVol\trightVol\tsurround\tspeed\tposition\tfilterType\tfilterA0\tfilterB0\tfilterB1\tsrcMode\toplRegisters\n";

	uint32 tick = 0, rowIndex = 0;
	for(const auto &row : m_testData->rows)
	{
		if(row.header.isFirstTick)
			tick = 0;
		
		const auto headerFormat = MPT_AFORMAT("{}\t{}\t{}\t{}\t{}\t{}\t");
		const auto channelHeaderFirst = headerFormat(rowIndex, row.header.order, row.header.row, tick, row.header.globalVolume, Util::muldivr_unsigned(row.header.tickLength, 100000, header.mixingFreq));
		const auto channelHeaderFollow = headerFormat("", "", "", "", "", "");
		tick++;
		rowIndex++;

		bool first = true;
		if(!row.oplRegisters.empty())
		{
			output << channelHeaderFirst << "\t\t\t\t\t\t\t\t\t\t\t\t" << OPLPlaybackLog::Format(row.oplRegisters);
			output << "\n";
			first = false;
		} else if(row.channels.empty())
		{
			output << channelHeaderFirst << "--\n";
		}
		for(const auto &channel : row.channels)
		{
			output << (first ? channelHeaderFirst : channelHeaderFollow);
			first = false;

			const char *filterType;
			switch(channel.flags & TestDataChannel::kFilterMask)
			{
			case TestDataChannel::kFilterNone: filterType = "--"; break;
			case TestDataChannel::kFilterLowPass: filterType = "LP"; break;
			case TestDataChannel::kFilterHighPass: filterType = "HP"; break;
			default: throw std::runtime_error{"Unknown filter type in test data"};
			}

			output << std::abs(channel.channel) << (channel.channel < 0 ? "[NNA]" : "") << "\t"
				   << channel.sample << "\t"
				   << channel.leftVol << "\t"
				   << channel.rightVol << "\t"
				   << ((channel.flags & TestDataChannel::kSurround) ? "yes" : "no") << "\t"
				   << (static_cast<double>(channel.increment) * positionPrecision * header.mixingFreq) << "\t"
				   << ((channel.flags & TestDataChannel::kOPL) ? "OPL" : mpt::afmt::fmt(static_cast<double>(channel.position) * positionPrecision, floatFormat)) << "\t"
				   << filterType << "\t"
				   << channel.filterA0 * filterPrecision << "\t"
				   << channel.filterB0 * filterPrecision << "\t"
				   << channel.filterB1 * filterPrecision << "\t"
				   << mpt::afmt::val(channel.srcMode) << "\n";
		}
	}
}


PlaybackTestSettings PlaybackTest::GetSettings() const noexcept
{
	return m_testData->GetSettings();
}



static bool FuzzyEquals(const double left, const double right, const double epsilon) noexcept
{
	return std::abs(left - right) <= std::min(std::abs(left), std::abs(right)) * epsilon;
}

#define MPT_LOG_TEST(propName, left, right) \
	errors.push_back(mpt::ToUnicode(mpt::Charset::UTF8, MPT_AFORMAT("{} differs: {} vs {}") \
		(propName, left, right)));
#define MPT_LOG_TEST_WITH_ROW(propName, left, right) \
	errors.push_back(mpt::ToUnicode(mpt::Charset::UTF8, MPT_AFORMAT("{} differs in test row {} (order {}, row {}, tick {}): {} vs {}") \
		(propName, row, lRow.header.order, lRow.header.row, lTick, left, right)));
#define MPT_LOG_TEST_WITH_ROW_CHN(propName, left, right) \
	errors.push_back(mpt::ToUnicode(mpt::Charset::UTF8, MPT_AFORMAT("{} differs in test row {} (order {}, row {}, tick {}), channel {}: {} vs {}") \
		(propName, row, lRow.header.order, lRow.header.row, lTick, chn, left, right)));

std::vector<mpt::ustring> PlaybackTest::Compare(const PlaybackTest &lhs, const PlaybackTest &rhs)
{
	return lhs.Compare(rhs);
}

std::vector<mpt::ustring> PlaybackTest::Compare(const PlaybackTest &otherTest) const
{
	const auto &other = *otherTest.m_testData;
	const auto &header = m_testData->header;

	std::vector<mpt::ustring> errors;

	if(header.mixingFreq != other.header.mixingFreq)
		return {MPT_UFORMAT("Mixing frequency differs ({} vs {}), not even going to try to compare them")(header.mixingFreq, other.header.mixingFreq)};
	if(header.outputChannels != other.header.outputChannels)
		return {MPT_UFORMAT("Output channel layout differs ({} vs {} channels), not even going to try to compare them")(header.outputChannels, other.header.outputChannels)};
	if(header.mixerChannels != other.header.mixerChannels)
		errors.push_back(MPT_UFORMAT("Mixer channel limit differs ({} vs {} channels), results may differ in case of channel starvation")(header.mixerChannels, other.header.mixerChannels));
	if(header.srcMode != other.header.srcMode)
		errors.push_back(MPT_UFORMAT("Default SRC mode differs ({} vs {} channels), results may differ")(header.srcMode, other.header.srcMode));

	const auto lPositionPrecision = 1.0 / (int64(1) << header.positionPrecisionBits);
	const auto rPositionPrecision = 1.0 / (int64(1) << other.header.positionPrecisionBits);
	const auto lFilterPrecision = 1.0 / (int64(1) << header.filterPrecisionBits);
	const auto rFilterPrecision = 1.0 / (int64(1) << other.header.filterPrecisionBits);
	const auto lGlobalVolumeScale = 1.0 / header.globalVolumeUnity;
	const auto rGlobalVolumeScale = 1.0 / other.header.globalVolumeUnity;
	const auto lChannelVolumeScale = 1.0 / header.channelVolumeUnity;
	const auto rChannelVolumeScale = 1.0 / other.header.channelVolumeUnity;
	const auto epsilon = 0.000001;

	if(header.isAmiga != other.header.isAmiga)
		MPT_LOG_TEST("Amiga mode", header.isAmiga, other.header.isAmiga);
	if(header.synthVolume != other.header.synthVolume)
		MPT_LOG_TEST("Synth volume", header.synthVolume, other.header.synthVolume);

	if(m_testData->sampleDataHashes.size() != other.sampleDataHashes.size())
		MPT_LOG_TEST("Number of sample slots", m_testData->sampleDataHashes.size(), other.sampleDataHashes.size());
	for(size_t smp = 0; smp < std::min(m_testData->sampleDataHashes.size(), other.sampleDataHashes.size()); smp++)
	{
		if(m_testData->sampleDataHashes[smp] != other.sampleDataHashes[smp])
			errors.push_back(MPT_UFORMAT("Sample hash in slot {} differs: {} vs {}")(smp + 1, mpt::encode_hex(mpt::as_raw_memory(m_testData->sampleDataHashes[smp])), mpt::encode_hex(mpt::as_raw_memory(other.sampleDataHashes[smp]))));
	}

	uint64 lDuration = 0, rDuration = 0;
	uint32 lTick = 0, rTick = 0;

	if(m_testData->rows.size() != other.rows.size())
		MPT_LOG_TEST("Number of test rows", m_testData->rows.size(), other.rows.size());
	for(size_t row = 0; row < std::min(m_testData->rows.size(), other.rows.size()); row++)
	{
		const auto &lRow = m_testData->rows[row], &rRow = other.rows[row];

		if(lRow.header.isFirstTick)
			lTick = 0;
		if(rRow.header.isFirstTick)
			rTick = 0;

		if(lRow.header.order != rRow.header.order
		   || lRow.header.row != rRow.header.row
		   || lRow.header.isFirstTick != rRow.header.isFirstTick)
			errors.push_back(MPT_UFORMAT("Play position differs in test row {} (order {}, row {}, tick {} vs order {}, row {}, tick {})")(row, lRow.header.order, lRow.header.row, lTick, rRow.header.order, rRow.header.row, rTick));

		if(const auto l = lRow.header.globalVolume * lGlobalVolumeScale, r = rRow.header.globalVolume * rGlobalVolumeScale; !FuzzyEquals(l, r, epsilon))
			MPT_LOG_TEST_WITH_ROW("Global volume", l, r);

		if(std::abs(static_cast<int32>(lRow.header.tickLength) - static_cast<int32>(rRow.header.tickLength)) > 1)
		{
			const auto lTickLength = Util::muldivr_unsigned(lRow.header.tickLength, 1'000'000, header.mixingFreq);
			const auto rTickLength = Util::muldivr_unsigned(rRow.header.tickLength, 1'000'000, other.header.mixingFreq);
			MPT_LOG_TEST_WITH_ROW("Tick length", mpt::afmt::val(lTickLength) + "us", mpt::afmt::val(rTickLength) + "us");
		}

		if(lRow.oplRegisters != rRow.oplRegisters)
		{
			MPT_LOG_TEST_WITH_ROW("OPL register log", OPLPlaybackLog::Format(lRow.oplRegisters), OPLPlaybackLog::Format(rRow.oplRegisters))
		}

		if(lRow.channels.size() != rRow.channels.size())
			MPT_LOG_TEST_WITH_ROW("Number of active voices", lRow.channels.size(), rRow.channels.size());
		for(size_t chn = 0; chn < std::min(lRow.channels.size(), rRow.channels.size()); chn++)
		{
			const auto &lChn = lRow.channels[chn], &rChn = rRow.channels[chn];

			if(lChn.channel != rChn.channel)
				MPT_LOG_TEST_WITH_ROW_CHN("Source channel", lChn.channel, rChn.channel);
			if(lChn.flags != rChn.flags)
				MPT_LOG_TEST_WITH_ROW_CHN("Flags", lChn.flags, rChn.flags);
			if(lChn.srcMode != rChn.srcMode)
				MPT_LOG_TEST_WITH_ROW_CHN("SRC mode", lChn.srcMode, rChn.srcMode);
			if(lChn.sample != rChn.sample)
				MPT_LOG_TEST_WITH_ROW_CHN("Sample", lChn.sample, rChn.sample);
			if(const auto l = lChn.leftVol * lChannelVolumeScale, r = rChn.leftVol * rChannelVolumeScale; !FuzzyEquals(l, r, epsilon))
				MPT_LOG_TEST_WITH_ROW_CHN("Left volume", l, r);
			if(const auto l = lChn.rightVol * lChannelVolumeScale, r = rChn.rightVol * rChannelVolumeScale; !FuzzyEquals(l, r, epsilon))
				MPT_LOG_TEST_WITH_ROW_CHN("Right volume", l, r);
			if(const auto l = static_cast<double>(lChn.increment) * lPositionPrecision * header.mixingFreq, r = static_cast<double>(rChn.increment) * rPositionPrecision * other.header.mixingFreq; !FuzzyEquals(l, r, epsilon))
				MPT_LOG_TEST_WITH_ROW_CHN("Speed", l, r);
			if(const auto l = static_cast<double>(lChn.position) * lPositionPrecision, r = static_cast<double>(rChn.position) * rPositionPrecision; !FuzzyEquals(l, r, epsilon))
				MPT_LOG_TEST_WITH_ROW_CHN("Position", l, r);
			if(const auto l = lChn.filterA0 * lFilterPrecision, r = rChn.filterA0 * rFilterPrecision; !FuzzyEquals(l, r, epsilon))
				MPT_LOG_TEST_WITH_ROW_CHN("Filter A0", l, r);
			if(const auto l = lChn.filterB0 * lFilterPrecision, r = rChn.filterB0 * rFilterPrecision; !FuzzyEquals(l, r, epsilon))
				MPT_LOG_TEST_WITH_ROW_CHN("Filter B0", l, r);
			if(const auto l = lChn.filterB1 * lFilterPrecision, r = rChn.filterB1 * rFilterPrecision; !FuzzyEquals(l, r, epsilon))
				MPT_LOG_TEST_WITH_ROW_CHN("Filter B1", l, r);
		}

		lDuration += lRow.header.tickLength;
		rDuration += rRow.header.tickLength;
		lTick++;
		rTick++;
	}

	if(const auto l = static_cast<double>(lDuration) / header.mixingFreq, r = static_cast<double>(rDuration) / other.header.mixingFreq; !FuzzyEquals(l, r, epsilon))
		MPT_LOG_TEST("Total duration", mpt::afmt::val(l) + "s", mpt::afmt::val(r) + "s");

	return errors;
}


PlaybackTest CSoundFile::CreatePlaybackTest(PlaybackTestSettings settings)
{
	settings.Sanitize();

	PlaybackTestData testData{};

	m_bIsRendering = true;
	const auto origResamplerSettings = m_Resampler.m_Settings;
	m_Resampler.m_Settings.SrcMode = settings.srcMode;
	m_Resampler.m_Settings.emulateAmiga = m_SongFlags[SONG_ISAMIGA] ? Resampling::AmigaFilter::A1200 : Resampling::AmigaFilter::Off;
	const auto origMixerSettings = m_MixerSettings;
	MixerSettings testSettings;
	testSettings.gdwMixingFreq = settings.mixingFreq;
	testSettings.gnChannels = settings.outputChannels;
	testSettings.m_nMaxMixChannels = settings.mixerChannels;
	testSettings.VolumeRampUpMicroseconds = 0;
	testSettings.VolumeRampDownMicroseconds = 0;
	SetMixerSettings(testSettings);

	const auto origRepeatCount = GetRepeatCount();
	SetRepeatCount(0);

	auto origPRNG = std::move(m_PRNG);
	mpt::deterministic_random_device rd;
	m_PRNG = mpt::make_prng<mpt::deterministic_fast_engine>(rd);

	auto origPlayState = std::make_unique<PlayState>(std::move(m_PlayState));
	mpt::reconstruct(m_PlayState);
	ResetPlayPos();

	auto origOPL = std::move(m_opl);
	OPLPlaybackLog oplLogger{m_PlayState};

	auto &header = testData.header;
	memcpy(header.magic, TestDataHeader::TestDataHeaderMagic, sizeof(header.magic));
	header.fileVersion = 0;
	header.isAmiga = m_SongFlags[SONG_ISAMIGA] ? 1 : 0;
	header.positionPrecisionBits = static_cast<uint8>(mpt::bit_width(static_cast<typename std::make_unsigned<SamplePosition::value_t>::type>(SamplePosition{1, 0}.GetRaw())) - 1);
	header.filterPrecisionBits = MIXING_FILTER_PRECISION;
	header.srcMode = m_Resampler.m_Settings.SrcMode;
	header.outputChannels = static_cast<uint8>(m_MixerSettings.gnChannels);
	header.mixerChannels = static_cast<uint16>(m_MixerSettings.m_nMaxMixChannels);
	header.synthVolume = static_cast<uint16>(m_nVSTiVolume);
	header.globalVolumeUnity = MAX_GLOBAL_VOLUME;
	header.channelVolumeUnity = 16384;
	header.mixingFreq = m_MixerSettings.gdwMixingFreq;
	header.mptVersion = Version::Current().GetRawVersion();
	header.numSamples = GetNumSamples();

	testData.sampleDataHashes.reserve(GetNumSamples());
	for(SAMPLEINDEX smp = 1; smp <= GetNumSamples(); smp++)
	{
		SampleDataHashAlgorithm hasher;
		const ModSample &sample = Samples[smp];
		if(sample.uFlags[CHN_ADLIB])
		{
			hasher.process(mpt::as_raw_memory(sample.adlib));
		} else
		{
			std::ostringstream ss{std::ios::out | std::ios::binary};
			SampleIO{sample.uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit, sample.uFlags[CHN_STEREO] ? SampleIO::stereoInterleaved : SampleIO::mono, SampleIO::littleEndian, SampleIO::signedPCM}
				.WriteSample(ss, sample);
			const auto s = std::move(ss).str();
			hasher.process(mpt::byte_cast<mpt::const_byte_span>(mpt::as_span(s)));
		}
		SampleDataHash result;
		result.set(hasher.result());
		testData.sampleDataHashes.push_back(result);
	}

	for(const auto &song : GetAllSubSongs())
	{
		oplLogger.Reset();
		m_opl = std::make_unique<OPL>(oplLogger);
		ResetPlayPos();
		GetLength(eAdjust, GetLengthTarget(song.startOrder, song.startRow).StartPos(song.sequence, 0, 0));
		m_PlayState.m_flags.reset();

		while(ReadOneTick())
		{
			auto &row = testData.rows.emplace_back();
			row.header.order = m_PlayState.m_nCurrentOrder;
			row.header.row = m_PlayState.m_nRow;
			row.header.globalVolume = m_PlayState.m_nGlobalVolume;
			row.header.tickLength = m_PlayState.m_nSamplesPerTick;
			row.header.isFirstTick = m_PlayState.m_nTickCount ? 0 : 1;
			row.header.activeChannels = static_cast<uint16>(std::count_if(std::begin(m_PlayState.Chn), std::end(m_PlayState.Chn), [](const auto &chn) { return chn.nLength != 0; }));

			row.channels.reserve(row.header.activeChannels);
			for(CHANNELINDEX chn = 0; chn < MAX_CHANNELS; chn++)
			{
				if(!m_PlayState.Chn[chn].nLength)
					continue;

				auto &channel = m_PlayState.Chn[chn];
				auto &channelData = row.channels.emplace_back();
				channelData.channel = static_cast<int16>(channel.nMasterChn ? -static_cast<int>(channel.nMasterChn) : (chn + 1));
				channelData.nnaAge = channel.nnaGeneration;
				if(channel.dwFlags[CHN_SURROUND])
					channelData.flags |= TestDataChannel::kSurround;
				if(channel.dwFlags[CHN_ADLIB])
					channelData.flags |= TestDataChannel::kOPL;
				if(channel.dwFlags[CHN_AMIGAFILTER])
					channelData.flags |= TestDataChannel::kAmigaFilter;
				channelData.srcMode = channel.resamplingMode;
				channelData.sample = static_cast<SAMPLEINDEX>(std::distance(&static_cast<const ModSample &>(Samples[0]), channel.pModSample));
				channelData.leftVol = channel.newLeftVol;
				channelData.rightVol =  channel.newRightVol;
				channelData.increment = channel.increment.GetRaw();
				channelData.position = channel.position.GetRaw();
				if(channel.dwFlags[CHN_FILTER])
					channelData.flags |= (channel.nFilter_HP ? TestDataChannel::kFilterHighPass : TestDataChannel::kFilterLowPass);
				channelData.filterA0 = channel.nFilter_A0;
				channelData.filterB0 = channel.nFilter_B0;
				channelData.filterB1 = channel.nFilter_B1;
			}
			std::sort(row.channels.begin(), row.channels.end());

			row.oplRegisters = oplLogger.DumpRegisters();
		}
	}

	m_opl = std::move(origOPL);
	m_PlayState = std::move(*origPlayState);
	m_PRNG = std::move(origPRNG);
	SetRepeatCount(origRepeatCount);
	SetMixerSettings(origMixerSettings);
	m_Resampler.m_Settings = origResamplerSettings;
	m_bIsRendering = false;

	return PlaybackTest{std::move(testData)};
}


#else // !MPT_ENABLE_PLAYBACK_TRACE


MPT_MSVC_WORKAROUND_LNK4221(PlaybackTest)


#endif // MPT_ENABLE_PLAYBACK_TRACE


OPENMPT_NAMESPACE_END
