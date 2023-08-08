#pragma once

#include "Commandlet/Commandlet.h"

class RunCommandlet : public Commandlet
{
public:
	RunCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;

private:
	bool GameRunning = false;
};
