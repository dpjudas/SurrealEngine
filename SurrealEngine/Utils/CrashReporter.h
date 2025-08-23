#pragma once

#include <string>
#include <functional>

class CrashDumpInfo
{
public:
	std::string exception;
	std::string callstack;
};

class CrashReporter
{
public:
	static void Init(const std::string& reportsDirectory, std::function<void(const std::string& logFilename)> saveLog);
	static void HookThread();
	static void Invoke();
	static CrashDumpInfo GetCrashDumpInfo(const std::string& dumpFilename);
};
