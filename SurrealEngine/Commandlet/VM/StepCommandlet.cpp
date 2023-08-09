
#include "Precomp.h"
#include "StepCommandlet.h"
#include "DebuggerApp.h"
#include "VM/Frame.h"

StepInCommandlet::StepInCommandlet()
{
	SetLongFormName("step");
	SetShortDescription("Go to next source line, diving into function");
}

void StepInCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	Frame::StepInto();
	console->ResumeProgram = true;
}

void StepInCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: step" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

StepOverCommandlet::StepOverCommandlet()
{
	SetLongFormName("next");
	SetShortDescription("Go to next source line, but don't dive into functions");
}

void StepOverCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	Frame::StepOver();
	console->ResumeProgram = true;
}

void StepOverCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: next" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

StepOutCommandlet::StepOutCommandlet()
{
	SetLongFormName("finish");
	SetShortDescription("Continue until the current function returns");
}

void StepOutCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	// Frame::StepOut();
	// console->ResumeProgram = true;

	console->WriteOutput("Not implemented" + NewLine());
}

void StepOutCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: finish" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

ContinueCommandlet::ContinueCommandlet()
{
	SetLongFormName("continue");
	SetShortDescription("Continue normal execution");
}

void ContinueCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	Frame::Resume();
	console->ResumeProgram = true;
}

void ContinueCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: continue" + NewLine());
}
