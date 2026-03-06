#pragma once

#include "Commandlet/Commandlet.h"

class PrintCommandlet : public Commandlet
{
public:
	PrintCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;

private:
	std::vector<std::pair<std::string, std::string>> FindProperties(DebuggerApp* console, const std::string& args);
	void PrintColumn(DebuggerApp* console, const std::string& name, const std::string& value, bool lastColumn);
};
