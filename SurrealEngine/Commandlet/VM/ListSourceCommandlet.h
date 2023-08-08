#pragma once

#include "Commandlet/Commandlet.h"

class ListSourceCommandlet : public Commandlet
{
public:
	ListSourceCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
