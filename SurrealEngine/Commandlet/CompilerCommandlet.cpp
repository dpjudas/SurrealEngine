
#include "Precomp.h"
#include "CompilerCommandlet.h"
#include "DebuggerApp.h"
#include "Compiler/Frontend/Compiler.h"
#include "Utils/File.h"

CompilerCommandlet::CompilerCommandlet()
{
	SetShortFormName("cc");
	SetLongFormName("compile");
	SetShortDescription("Compile unrealscript file");
}

void CompilerCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	std::string filename = args;

#if 1
	if (filename.empty())
		filename = "C:\\Games\\DeusEx-1112fm-Scripts\\ConSys\\Classes\\ConCamera.uc";
#endif

	if (filename.empty())
	{
		console->WriteOutput("Compile command expects a filename argument" + NewLine());
		return;
	}

	Compiler cc;
	cc.add_code(File::read_all_text(filename), fs::path(filename).filename().string());
	cc.compile();

	for (const CompilerMessage& msg : cc.get_messages())
	{
		console->WriteOutput(msg.to_string() + NewLine());
	}
}

void CompilerCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
