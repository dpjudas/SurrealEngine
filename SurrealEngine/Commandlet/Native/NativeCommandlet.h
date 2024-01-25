#pragma once

#include "Commandlet/Commandlet.h"

class NativeCommandlet : public Commandlet
{
public:
	NativeCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
