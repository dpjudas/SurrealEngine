
#include "Precomp.h"
#include "BreakpointCommandlet.h"
#include "DebuggerApp.h"
#include "VM/Frame.h"
#include "Engine.h"

ListBreakpointsCommandlet::ListBreakpointsCommandlet()
{
	SetLongFormName("breakpoints");
	SetShortDescription("Print information about the break- and watchpoints");
}

void ListBreakpointsCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	int index = 0;
	for (const Breakpoint& bp : Frame::Breakpoints)
	{
		if (bp.Enabled)
			console->WriteOutput("#" + std::to_string(index) + ": " + bp.Package.ToString() + " " + bp.Class.ToString() + " " + bp.Function.ToString() + " " + bp.State.ToString());
		else
			console->WriteOutput("#" + std::to_string(index) + ": " + bp.Package.ToString() + " " + bp.Class.ToString() + " " + bp.Function.ToString() + " " + bp.State.ToString() + " [disabled]");
		index++;
	}
}

void ListBreakpointsCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: breakpoints" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

BreakpointCommandlet::BreakpointCommandlet()
{
	SetLongFormName("break");
	SetShortDescription("Set a new breakpoint");
}

void BreakpointCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (!engine)
	{
		console->WriteOutput("Game must be running before breakpoints can be added" + NewLine());
		return;
	}

	std::vector<std::string> params = SplitString(args);
	if (params.size() == 3)
	{
		if (Frame::AddBreakpoint(params[0], params[1], params[2]))
		{
			int index = (int)Frame::Breakpoints.size();
			console->WriteOutput("Added breakpoint #" + std::to_string(index) + NewLine());
		}
		else
		{
			console->WriteOutput("Could not add breakpoint: package/class/function not found" + NewLine());
		}
	}
	else if (params.size() == 4)
	{
		if (Frame::AddBreakpoint(params[0], params[1], params[2], params[3]))
		{
			int index = (int)Frame::Breakpoints.size();
			console->WriteOutput("Added breakpoint #" + std::to_string(index) + NewLine());
		}
		else
		{
			console->WriteOutput("Could not add breakpoint: package/class/function/state not found" + NewLine());
		}
	}
}

void BreakpointCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: break <package> <class> <function> [state]" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

WatchpointCommandlet::WatchpointCommandlet()
{
	SetLongFormName("watch");
	SetShortDescription("Set a new watchpoint");
}

void WatchpointCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	console->WriteOutput("Not implemented" + NewLine());

	/*
	std::vector<std::string> params = SplitString(args);
	if (params.size() == 3)
	{
		if (Frame::AddWatchpoint(params[0], params[1], params[2]))
		{
			int index = (int)Frame::Breakpoints.size();
			console->WriteOutput("Added watchpoint #" + std::to_string(index) + NewLine());
		}
		else
		{
			console->WriteOutput("Could not add watchpoint: package/class/function not found" + NewLine());
		}
	}
	*/
}

void WatchpointCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: watch <package> <class> <property>" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

DeleteBreakpointCommandlet::DeleteBreakpointCommandlet()
{
	SetLongFormName("delete");
	SetShortDescription("Remove a breakpoint");
}

void DeleteBreakpointCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (!args.empty())
	{
		int index = std::atoi(args.c_str());
		if (index >= 0 && (size_t)index < Frame::Breakpoints.size())
		{
			Frame::Breakpoints.erase(Frame::Breakpoints.begin() + index);
		}
		else
		{
			console->WriteOutput("Breakpoint not found" + NewLine());
		}
	}
}

void DeleteBreakpointCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: delete <breakpoint index>" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

ClearBreakpointsCommandlet::ClearBreakpointsCommandlet()
{
	SetLongFormName("clear");
	SetShortDescription("Remove all breakpoints");
}

void ClearBreakpointsCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	Frame::Breakpoints.clear();
	console->WriteOutput("Removed all breakpoints" + NewLine());
}

void ClearBreakpointsCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: clear" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

EnableBreakpointCommandlet::EnableBreakpointCommandlet()
{
	SetLongFormName("enable");
	SetShortDescription("Enable a disabled breakpoint");
}

void EnableBreakpointCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (!args.empty())
	{
		int index = std::atoi(args.c_str());
		if (index >= 0 && (size_t)index < Frame::Breakpoints.size())
		{
			Frame::Breakpoints[index].Enabled = true;
			console->WriteOutput("Breakpoint #" + std::to_string(index) + " enabled" + NewLine());
		}
		else
		{
			console->WriteOutput("Breakpoint not found" + NewLine());
		}
	}
}

void EnableBreakpointCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: enable <breakpoint index>" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

DisableBreakpointCommandlet::DisableBreakpointCommandlet()
{
	SetLongFormName("disable");
	SetShortDescription("Disable a breakpoint");
}

void DisableBreakpointCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (!args.empty())
	{
		int index = std::atoi(args.c_str());
		if (index >= 0 && (size_t)index < Frame::Breakpoints.size())
		{
			Frame::Breakpoints[index].Enabled = false;
			console->WriteOutput("Breakpoint #" + std::to_string(index) + " disabled" + NewLine());
		}
		else
		{
			console->WriteOutput("Breakpoint not found" + NewLine());
		}
	}
}

void DisableBreakpointCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: disable <breakpoint index>" + NewLine());
}
