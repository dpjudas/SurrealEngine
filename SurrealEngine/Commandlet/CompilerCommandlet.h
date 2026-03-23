#pragma once

#include "Commandlet/Commandlet.h"

class CompilerCommandlet : public Commandlet
{
public:
	CompilerCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
