#pragma once

#include <string>
#include <list>
#include <functional>

void LogMessage(const std::string& message);
void LogUnimplemented(const std::string& message);

struct LogMessageLine
{
	float Time;
	std::string Source;
	std::string Text;
};

class Logger
{
public:
	static Logger* Get();

	void LogMessage(const std::string& message);
	void LogUnimplemented(const std::string& message);
	void SetTimeSeconds(float value) { time = value; }
	void SetCallback(std::function<void(const LogMessageLine& line)> cb) { printLogDebugger = std::move(cb); }
	const std::list<LogMessageLine>& GetLog() const { return Log; }

private:
	float time = 0.0f;
	std::list<LogMessageLine> Log;
	std::function<void(const LogMessageLine& line)> printLogDebugger;

	Logger() = default;
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
};
