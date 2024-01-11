#pragma once

#include "Commandlet/Commandlet.h"

class NativesCommandlet : public Commandlet
{
public:
	NativesCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
