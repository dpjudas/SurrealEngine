
#include "Precomp.h"
#include "ListSourceCommandlet.h"
#include "DebuggerApp.h"

ListSourceCommandlet::ListSourceCommandlet()
{
	SetShortFormName("l");
	SetLongFormName("list");
	SetShortDescription("Show source code");
}

void ListSourceCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
}

void ListSourceCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
