#pragma once

#include "Commandlet/Commandlet.h"

class LocalsCommandlet : public Commandlet
{
public:
	LocalsCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
