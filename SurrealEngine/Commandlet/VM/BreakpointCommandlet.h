#pragma once

#include "Commandlet/Commandlet.h"

class ListBreakpointsCommandlet : public Commandlet
{
public:
	ListBreakpointsCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};

class BreakpointCommandlet : public Commandlet
{
public:
	BreakpointCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};

class WatchpointCommandlet : public Commandlet
{
public:
	WatchpointCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};

class DeleteBreakpointCommandlet : public Commandlet
{
public:
	DeleteBreakpointCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};

class ClearBreakpointsCommandlet : public Commandlet
{
public:
	ClearBreakpointsCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};

class EnableBreakpointCommandlet : public Commandlet
{
public:
	EnableBreakpointCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};

class DisableBreakpointCommandlet : public Commandlet
{
public:
	DisableBreakpointCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
