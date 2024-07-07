
#include "Precomp.h"
#include "Utils/Logger.h"
#include "VM/Frame.h"

void LogMessage(const std::string& message)
{
	Logger::Get()->LogMessage(message);
}

void LogUnimplemented(const std::string& message)
{
	Logger::Get()->LogUnimplemented(message);
}

void Logger::LogMessage(const std::string& message)
{
	if (!Frame::Callstack.empty() && Frame::Callstack.back()->Func)
	{
		UStruct* func = Frame::Callstack.back()->Func;
		std::string name;
		for (UStruct* s = func; s != nullptr; s = s->StructParent)
		{
			if (name.empty())
				name = s->Name.ToString();
			else
				name = s->Name.ToString() + "." + name;
		}

		LogMessageLine line;
		line.Time = time;
		line.Source = name;
		line.Text = message;
		Log.push_back(std::move(line));
	}
	else
	{
		LogMessageLine line;
		line.Time = time;
		line.Text = message;
		Log.push_back(std::move(line));
	}

	if (printLogDebugger)
		printLogDebugger(Log.back());
}

void Logger::LogUnimplemented(const std::string& message)
{
	LogMessage("Unimplemented: " + message);
}

Logger* Logger::Get()
{
	static Logger logger;
	return &logger;
}
