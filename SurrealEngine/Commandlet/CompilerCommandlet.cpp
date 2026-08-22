
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
	std::string folder = args;

#if 0
	if (folder.empty())
		folder = "C:\\Games\\UTScript"; // "C:\\Games\\DeusEx-1112fm-Scripts"
#endif

	if (folder.empty())
	{
		console->WriteOutput("Compile command expects a path to a folder with unrealscript files" + NewLine());
		return;
	}

	Compiler cc;
	for (const auto& entry : fs::recursive_directory_iterator(folder))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".uc")
		{
			cc.add_code(File::read_all_text(entry.path().string()), entry.path().filename().string());
		}
	}
	cc.compile();
	for (const CompilerMessage& msg : cc.get_messages())
	{
		console->WriteOutput(msg.to_string() + NewLine());
	}
}

void CompilerCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("cc <path to scripts directory>" + NewLine());
}
