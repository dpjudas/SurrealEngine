#pragma once

#include "Commandlet/Commandlet.h"

class PrintCommandlet : public Commandlet
{
public:
	PrintCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
