#pragma once

#include "UInfo.h"

class UStatLog : public UInfo
{
public:
	using UInfo::UInfo;

	int& Context() { return Value<int>(PropOffsets_StatLog.Context); }
	std::string& DecoderRingURL() { return Value<std::string>(PropOffsets_StatLog.DecoderRingURL); }
	std::string& GameCreator() { return Value<std::string>(PropOffsets_StatLog.GameCreator); }
	std::string& GameCreatorURL() { return Value<std::string>(PropOffsets_StatLog.GameCreatorURL); }
	std::string& GameName() { return Value<std::string>(PropOffsets_StatLog.GameName); }
	std::string& LocalBatcherParams() { return Value<std::string>(PropOffsets_StatLog.LocalBatcherParams); }
	std::string& LocalBatcherURL() { return Value<std::string>(PropOffsets_StatLog.LocalBatcherURL); }
	std::string& LocalLogDir() { return Value<std::string>(PropOffsets_StatLog.LocalLogDir); }
	std::string& LocalStandard() { return Value<std::string>(PropOffsets_StatLog.LocalStandard); }
	std::string& LocalStatsURL() { return Value<std::string>(PropOffsets_StatLog.LocalStatsURL); }
	std::string& LogInfoURL() { return Value<std::string>(PropOffsets_StatLog.LogInfoURL); }
	std::string& LogVersion() { return Value<std::string>(PropOffsets_StatLog.LogVersion); }
	float& TimeStamp() { return Value<float>(PropOffsets_StatLog.TimeStamp); }
	std::string& WorldBatcherParams() { return Value<std::string>(PropOffsets_StatLog.WorldBatcherParams); }
	std::string& WorldBatcherURL() { return Value<std::string>(PropOffsets_StatLog.WorldBatcherURL); }
	std::string& WorldLogDir() { return Value<std::string>(PropOffsets_StatLog.WorldLogDir); }
	std::string& WorldStandard() { return Value<std::string>(PropOffsets_StatLog.WorldStandard); }
	std::string& WorldStatsURL() { return Value<std::string>(PropOffsets_StatLog.WorldStatsURL); }
	BitfieldBool bWorld() { return BoolValue(PropOffsets_StatLog.bWorld); }
	BitfieldBool bWorldBatcherError() { return BoolValue(PropOffsets_StatLog.bWorldBatcherError); }
};
