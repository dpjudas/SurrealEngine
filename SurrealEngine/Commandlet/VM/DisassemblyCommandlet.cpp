
#include "Precomp.h"
#include "DisassemblyCommandlet.h"
#include "DebuggerApp.h"

DisassemblyCommandlet::DisassemblyCommandlet()
{
	SetShortFormName("disasm");
	SetLongFormName("disassembly");
	SetShortDescription("Print disassembly for function");
}

void DisassemblyCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
}

void DisassemblyCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
