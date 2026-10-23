
#include "Precomp.h"
#include "TraceCommandlet.h"
#include "DebuggerApp.h"
#include "VM/Frame.h"
#include "Engine.h"
#include "Packages/Core/UClass.h"
#include "Packages/Core/Properties/UClassProperty.h"

ListTracesCommandlet::ListTracesCommandlet()
{
	SetLongFormName("traces");
	SetShortDescription("Print information about the tracepoints");
}

void ListTracesCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	int index = 0;
	for (const Trace& t : Frame::Traces)
	{
		if (t.Enabled)
			console->WriteOutput("#" + std::to_string(index) + ": " + t.Function.ToString() + NewLine());
		else
			console->WriteOutput("#" + std::to_string(index) + ": " + t.Function.ToString() + " [disabled]" + NewLine());
		index++;
	}
}

void ListTracesCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: traces" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

TraceCommandlet::TraceCommandlet()
{
	SetLongFormName("trace");
	SetShortDescription("Set a new trace");
}

void TraceCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (!engine)
	{
		console->WriteOutput("Game must be running before traces can be added" + NewLine());
		return;
	}

	Array<std::string> params = SplitString(args);
	if (params.size() == 1)
	{
		int index = (int)Frame::Breakpoints.size();
		Array<std::string> nameChunks = SplitString(params[0], '.');
		Frame::AddTrace(nameChunks.back(), [=,this](UFunction* func, UObject* instance, const ArrayView<ExpressionValue>& args) {
			if (nameChunks.size() > 1)
				if (instance->Name != nameChunks[0] || instance->Class->Name != nameChunks[0])
					return;
			LogMessage("Trace hit for " + nameChunks.back());
			LogMessage(Frame::GetCallstack());
			});
		console->WriteOutput("Added trace #" + std::to_string(index) + NewLine());
	}
	else if (params.size() == 2)
	{
		int index = (int)Frame::Breakpoints.size();
		Array<std::string> nameChunks = SplitString(params[0], '.');
		Array<std::string> chunks = SplitString(params[1], '.');

		if (chunks.size() == 1 && chunks.front() == "break")
		{
			Frame::AddTrace(nameChunks.back(), [=](UFunction* func, UObject* instance, const ArrayView<ExpressionValue>& args) {
				if (nameChunks.size() > 1)
					if (instance->Name != nameChunks[0] || instance->Class->Name != nameChunks[0])
						return;
				Frame::Break();
				});
		}
		else
		{
			Frame::AddTrace(nameChunks.back(), [=](UFunction* func, UObject* instance, const ArrayView<ExpressionValue>& args) {
				if (nameChunks.size() > 1)
					if (instance->Name != nameChunks[0] || instance->Class->Name != nameChunks[0])
						return;

				UObject* obj = instance;
				for (auto chunk = chunks.begin(); chunk != chunks.end(); chunk++)
				{
					bool bFoundObj = false;
					for (UProperty* prop : obj->PropertyData.Class->Properties)
					{
						if (prop->Name == (*chunk))
						{
							void* val = obj->PropertyData.Ptr(prop);

							if (chunk + 1 == chunks.end())
							{
								std::string name = prop->Name.ToString();
								std::string value = prop->PrintValue(val);
								PrintColumn(console, instance->Name.ToString(), instance->Class->Name.ToString(), false);
								PrintColumn(console, name, value, true);
								bFoundObj = true;
								break;
							}
							else if (UObject::TryCast<UObjectProperty>(prop) || UObject::TryCast<UClassProperty>(prop))
							{
								obj = *(UObject**)val;
								bFoundObj = true;
								break;
							}
						}
					}

					if (!bFoundObj)
						break;
				}
				});
		}
		console->WriteOutput("Added trace #" + std::to_string(index) + NewLine());
	}
	else
	{
		OnPrintHelp(console);
	}
}

void TraceCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: trace <function>              - Print callstack when called" + NewLine());
	console->WriteOutput("        trace <function> <property>   - Print variable when called " + NewLine());
	console->WriteOutput("        trace <function> break        - Break debugger when called " + NewLine());
}

void TraceCommandlet::PrintColumn(DebuggerApp* console, const std::string& name, const std::string& value, bool lastColumn)
{
	std::string space1, space2;
	space1.resize(std::max(32 - (int)name.size(), 1), ' ');
	if (!lastColumn)
		space2.resize(std::max(56 - (int)value.size(), 1), ' ');

	console->WriteOutput(name + ResetEscape() + space1 + ColorEscape(96) + value + space2 + ResetEscape());
	if (lastColumn)
		console->WriteOutput(NewLine());
}

/////////////////////////////////////////////////////////////////////////////

DeleteTraceCommandlet::DeleteTraceCommandlet()
{
	SetLongFormName("delete");
	SetShortDescription("Remove a trace");
}

void DeleteTraceCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (!args.empty())
	{
		int index = std::atoi(args.c_str());
		if (index >= 0 && (size_t)index < Frame::Traces.size())
		{
			Frame::Traces.erase(Frame::Traces.begin() + index);
		}
		else
		{
			console->WriteOutput("Trace not found" + NewLine());
		}
	}
}

void DeleteTraceCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: delete <trace index>" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

EnableTraceCommandlet::EnableTraceCommandlet()
{
	SetLongFormName("enable");
	SetShortDescription("Enable a disabled trace");
}

void EnableTraceCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (!args.empty())
	{
		int index = std::atoi(args.c_str());
		if (index >= 0 && (size_t)index < Frame::Traces.size())
		{
			Frame::Traces[index].Enabled = true;
			console->WriteOutput("Trace #" + std::to_string(index) + " enabled" + NewLine());
		}
		else
		{
			console->WriteOutput("Trace not found" + NewLine());
		}
	}
}

void EnableTraceCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: enable <trace index>" + NewLine());
}

/////////////////////////////////////////////////////////////////////////////

DisableTraceCommandlet::DisableTraceCommandlet()
{
	SetLongFormName("disable");
	SetShortDescription("Disable a trace");
}

void DisableTraceCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (!args.empty())
	{
		int index = std::atoi(args.c_str());
		if (index >= 0 && (size_t)index < Frame::Traces.size())
		{
			Frame::Traces[index].Enabled = false;
			console->WriteOutput("Trace #" + std::to_string(index) + " disabled" + NewLine());
		}
		else
		{
			console->WriteOutput("Trace not found" + NewLine());
		}
	}
}

void DisableTraceCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: disable <trace index>" + NewLine());
}
