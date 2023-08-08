
#include "Precomp.h"
#include "BreakpointCommandlet.h"
#include "DebuggerApp.h"
#include "VM/Frame.h"

ListBreakpointsCommandlet::ListBreakpointsCommandlet()
{
	SetLongFormName("breakpoints");
	SetShortDescription("Print information about the break- and watchpoints");
}

void ListBreakpointsCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
}

void ListBreakpointsCommandlet::OnPrintHelp(DebuggerApp* console)
{
}

/////////////////////////////////////////////////////////////////////////////

BreakpointCommandlet::BreakpointCommandlet()
{
	SetLongFormName("break");
	SetShortDescription("Set a new breakpoint");
}

void BreakpointCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	// Frame::AddBreakpoint("Botpack", "DeathMatchPlus", "Timer");
}

void BreakpointCommandlet::OnPrintHelp(DebuggerApp* console)
{
}

/////////////////////////////////////////////////////////////////////////////

WatchpointCommandlet::WatchpointCommandlet()
{
	SetLongFormName("watch");
	SetShortDescription("Set a new watchpoint");
}

void WatchpointCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	// Frame::AddWatchpoint(object, property);
}

void WatchpointCommandlet::OnPrintHelp(DebuggerApp* console)
{
}

/////////////////////////////////////////////////////////////////////////////

DeleteBreakpointCommandlet::DeleteBreakpointCommandlet()
{
	SetLongFormName("delete");
	SetShortDescription("Remove a breakpoint");
}

void DeleteBreakpointCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
}

void DeleteBreakpointCommandlet::OnPrintHelp(DebuggerApp* console)
{
}

/////////////////////////////////////////////////////////////////////////////

ClearBreakpointsCommandlet::ClearBreakpointsCommandlet()
{
	SetLongFormName("clear");
	SetShortDescription("Remove all breakpoints");
}

void ClearBreakpointsCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
}

void ClearBreakpointsCommandlet::OnPrintHelp(DebuggerApp* console)
{
}

/////////////////////////////////////////////////////////////////////////////

EnableBreakpointCommandlet::EnableBreakpointCommandlet()
{
	SetLongFormName("enable");
	SetShortDescription("Enable a disabled breakpoint");
}

void EnableBreakpointCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
}

void EnableBreakpointCommandlet::OnPrintHelp(DebuggerApp* console)
{
}

/////////////////////////////////////////////////////////////////////////////

DisableBreakpointCommandlet::DisableBreakpointCommandlet()
{
	SetLongFormName("disable");
	SetShortDescription("Disable a breakpoint");
}

void DisableBreakpointCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
}

void DisableBreakpointCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
