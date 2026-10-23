#pragma once

#include "Commandlet/Commandlet.h"

class ListTracesCommandlet : public Commandlet
{
public:
	ListTracesCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};

class TraceCommandlet : public Commandlet
{
public:
	TraceCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;

private:
	void PrintColumn(DebuggerApp* console, const std::string& name, const std::string& value, bool lastColumn);
};

class DeleteTraceCommandlet : public Commandlet
{
public:
	DeleteTraceCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};

class EnableTraceCommandlet : public Commandlet
{
public:
	EnableTraceCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};

class DisableTraceCommandlet : public Commandlet
{
public:
	DisableTraceCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
