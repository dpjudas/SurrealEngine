
#include "Precomp.h"
#include "Utils/Logger.h"
#include "Utils/JsonValue.h"
#include "Utils/File.h"
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
		std::string name;

		name = UObject::GetUClassFullName(Frame::Callstack.front()->Object).ToString();
		/*
		UStruct* func = Frame::Callstack.back()->Func;
		for (UStruct* s = func; s != nullptr; s = s->StructParent)
		{
			if (name.empty())
				name = s->Name.ToString();
			else
				name = s->Name.ToString() + "." + name;
		}
		*/

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

void Logger::SaveLog(const std::string& filename)
{
	// We want this formatted in a specific way so its reasonably readable both by humans and still by a json parser.
	std::string json = "[";
	bool firstLine = true;
	for (const LogMessageLine& line : Log)
	{
		if (!firstLine)
			json += ',';
		firstLine = false;
		json += '\n';
		JsonValue logMessage = JsonValue::array();
		logMessage.items().push_back(JsonValue::number(line.Time));
		logMessage.items().push_back(JsonValue::string(line.Source));
		logMessage.items().push_back(JsonValue::string(line.Text));
		json += logMessage.to_json(false);
	}
	json += "\n]\n";
	File::write_all_text(filename, json);
}

void Logger::SaveLogAsPlaintext(const std::string& filename) const
{
	std::string finalLog;

	for (const LogMessageLine& line : Log)
	{
		const auto source = line.Source.empty() ? "<Surreal Engine>" : line.Source;
		finalLog += "[" + std::to_string(line.Time) + "] " + source + ": " + line.Text + "\n";
	}

	File::write_all_text(filename, finalLog);
}


std::list<LogMessageLine> Logger::LoadLog(const std::string& filename)
{
	std::list<LogMessageLine> log;
	JsonValue json = JsonValue::parse(File::read_all_text(filename));
	for (const JsonValue& logMessage : json.items())
	{
		LogMessageLine line;
		line.Time = logMessage.at(0).to_float();
		line.Source = logMessage.at(1).to_string();
		line.Text = logMessage.at(2).to_string();
		log.push_back(std::move(line));
	}
	return log;
}
