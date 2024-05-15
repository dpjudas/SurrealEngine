#pragma once

#include <mutex>
#include <string>

class Exception
{
public:
	[[noreturn]] static void Throw(const std::string& text);

private:
	static int CaptureStackFrames(std::ostringstream& sstream, int maxframes);

	static std::mutex mutex;
};