
#include "Precomp.h"
#include "QuitCommandlet.h"
#include "DebuggerApp.h"

QuitCommandlet::QuitCommandlet()
{
	SetShortFormName("q");
	SetLongFormName("quit");
	SetShortDescription("Exit the debugger");
}

void QuitCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	console->Exit();
}

void QuitCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
