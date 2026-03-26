
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
		folder = "C:\\Games\\DeusEx-1112fm-Scripts";
#endif

	if (folder.empty())
	{
		console->WriteOutput("Compile command expects a path to a folder with unrealscript files" + NewLine());
		return;
	}

	for (const auto& entry : fs::recursive_directory_iterator("C:\\Games\\DeusEx-1112fm-Scripts"))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".uc")
		{
			// console->WriteOutput(entry.path().filename().string() + NewLine());
			Compiler cc;
			cc.add_code(File::read_all_text(entry.path().string()), entry.path().filename().string());
			cc.compile();

			for (const CompilerMessage& msg : cc.get_messages())
			{
				console->WriteOutput(msg.to_string() + NewLine());
			}
		}
	}
}

void CompilerCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
