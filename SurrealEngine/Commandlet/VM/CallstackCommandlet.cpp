
#include "Precomp.h"
#include "CallstackCommandlet.h"
#include "DebuggerApp.h"
#include "VM/Frame.h"

CallstackCommandlet::CallstackCommandlet()
{
	SetShortFormName("bt");
	SetLongFormName("backtrace");
	SetShortDescription("Show call stack");
}

void CallstackCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	int index = 0;
	for (auto it = Frame::Callstack.rbegin(); it != Frame::Callstack.rend(); ++it)
	{
		Frame* frame = *it;
		UStruct* func = frame->Func;
		if (func)
		{
			std::string name;
			for (UStruct* s = func; s != nullptr; s = s->StructParent)
			{
				if (name.empty())
					name = s->Name.ToString();
				else
					name = s->Name.ToString() + "." + name;
			}

			if (name.size() < 40)
				name.resize(40, ' ');

			console->WriteOutput("#" + std::to_string(index) + ": " + ColorEscape(96) + name + ResetEscape() + " line " + ColorEscape(96) + std::to_string(func->Line) + ResetEscape() + NewLine());
		}
		index++;
	}
	console->WriteOutput(NewLine());
}

void CallstackCommandlet::OnPrintHelp(DebuggerApp* console)
{
}

/////////////////////////////////////////////////////////////////////////////

SelectFrameCommandlet::SelectFrameCommandlet()
{
	SetShortFormName("f");
	SetLongFormName("frame");
	SetShortDescription("Select the stack frame to operate on");
}

void SelectFrameCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (!args.empty())
	{
		console->CallstackIndex = std::atoi(args.c_str());
	}
}

void SelectFrameCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
