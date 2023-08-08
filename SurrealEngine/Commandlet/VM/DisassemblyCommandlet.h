#pragma once

#include "Commandlet/Commandlet.h"

class DisassemblyCommandlet : public Commandlet
{
public:
	DisassemblyCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
