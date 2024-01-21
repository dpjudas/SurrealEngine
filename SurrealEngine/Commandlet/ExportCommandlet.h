#pragma once

#include "Commandlet/Commandlet.h"

class ExportCommandlet : public Commandlet
{
public:
	ExportCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
