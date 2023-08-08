#pragma once

#include "Commandlet/Commandlet.h"

class ExtractCommandlet : public Commandlet
{
public:
	ExtractCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
