
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
		console->ListSourceLineOffset = 0;
	}
}

void SelectFrameCommandlet::OnPrintHelp(DebuggerApp* console)
{
}

/////////////////////////////////////////////////////////////////////////////

UpFrameCommandlet::UpFrameCommandlet()
{
	//SetShortFormName("up");
	SetLongFormName("up");
	SetShortDescription("Selects the previous (outer) stack frame or one of the frames preceding it");
}

void UpFrameCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	int count = 1;
	if (!args.empty())
		count = std::atoi(args.c_str());
	console->CallstackIndex = std::max(console->CallstackIndex + count, 0);
	console->ListSourceLineOffset = 0;
}

void UpFrameCommandlet::OnPrintHelp(DebuggerApp* console)
{
}

/////////////////////////////////////////////////////////////////////////////

DownFrameCommandlet::DownFrameCommandlet()
{
	//SetShortFormName("down");
	SetLongFormName("down");
	SetShortDescription("Selects the next (inner) stack frame or one of the frames following it");
}

void DownFrameCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	int count = 1;
	if (!args.empty())
		count = std::atoi(args.c_str());
	console->CallstackIndex = std::max(console->CallstackIndex - count, 0);
	console->ListSourceLineOffset = 0;
}

void DownFrameCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
