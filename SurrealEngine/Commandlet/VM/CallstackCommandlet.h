#pragma once

#include "Commandlet/Commandlet.h"

class CallstackCommandlet : public Commandlet
{
public:
	CallstackCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};

class SelectFrameCommandlet : public Commandlet
{
public:
	SelectFrameCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
