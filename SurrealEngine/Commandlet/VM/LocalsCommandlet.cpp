
#include "Precomp.h"
#include "LocalsCommandlet.h"
#include "DebuggerApp.h"
#include "VM/Frame.h"

LocalsCommandlet::LocalsCommandlet()
{
	SetLongFormName("locals");
	SetShortDescription("Print the local variables in the currently selected stack frame");
}

void LocalsCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	Frame* frame = console->GetCurrentFrame();
	if (frame)
	{
		for (UProperty* prop : frame->Func->Properties)
		{
			void* ptr = ((uint8_t*)frame->Variables.get()) + prop->DataOffset;

			std::string name = prop->Name.ToString();
			std::string value = prop->PrintValue(ptr);

			if (name.size() < 40)
				name.resize(40, ' ');

			console->WriteOutput(ColorEscape(96) + name + ResetEscape() + " " + ColorEscape(96) + value + ResetEscape() + NewLine());
		}
	}
	console->WriteOutput(NewLine());
}

void LocalsCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
