#pragma once

#include "Commandlet/Commandlet.h"

class StepInCommandlet : public Commandlet
{
public:
	StepInCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};

class StepOverCommandlet : public Commandlet
{
public:
	StepOverCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};

class StepOutCommandlet : public Commandlet
{
public:
	StepOutCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};

class ContinueCommandlet : public Commandlet
{
public:
	ContinueCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
