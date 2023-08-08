
#include "Precomp.h"
#include "RunCommandlet.h"
#include "DebuggerApp.h"

RunCommandlet::RunCommandlet()
{
	SetShortFormName("r");
	SetLongFormName("run");
	SetShortDescription("Run the game to be debugged");
}

void RunCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	console->RunGame();
}

void RunCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
