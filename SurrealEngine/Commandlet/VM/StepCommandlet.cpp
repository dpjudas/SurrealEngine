
#include "Precomp.h"
#include "StepCommandlet.h"
#include "DebuggerApp.h"

StepInCommandlet::StepInCommandlet()
{
	SetLongFormName("step");
	SetShortDescription("Go to next source line, diving into function");
}

void StepInCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
}

void StepInCommandlet::OnPrintHelp(DebuggerApp* console)
{
}

/////////////////////////////////////////////////////////////////////////////

StepOverCommandlet::StepOverCommandlet()
{
	SetLongFormName("next");
	SetShortDescription("Go to next source line, but don't dive into functions");
}

void StepOverCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
}

void StepOverCommandlet::OnPrintHelp(DebuggerApp* console)
{
}

/////////////////////////////////////////////////////////////////////////////

StepOutCommandlet::StepOutCommandlet()
{
	SetLongFormName("finish");
	SetShortDescription("Continue until the current function returns");
}

void StepOutCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
}

void StepOutCommandlet::OnPrintHelp(DebuggerApp* console)
{
}

/////////////////////////////////////////////////////////////////////////////

ContinueCommandlet::ContinueCommandlet()
{
	SetLongFormName("continue");
	SetShortDescription("Continue normal execution");
}

void ContinueCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
}

void ContinueCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
