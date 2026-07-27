#pragma once

#include "UStatLog.h"

class UStatLogFile : public UStatLog
{
public:
	using UStatLog::UStatLog;

	int& LogAr() { return Value<int>(PropOffsets_StatLogFile.LogAr); }
	std::string& StatLogFile() { return Value<std::string>(PropOffsets_StatLogFile.StatLogFile); }
	std::string& StatLogFinal() { return Value<std::string>(PropOffsets_StatLogFile.StatLogFinal); }
	BitfieldBool bWatermark() { return BoolValue(PropOffsets_StatLogFile.bWatermark); }
};
