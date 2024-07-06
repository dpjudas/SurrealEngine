
#include "Precomp.h"
#include "PrintCommandlet.h"
#include "DebuggerApp.h"
#include "VM/Frame.h"

PrintCommandlet::PrintCommandlet()
{
	SetShortFormName("p");
	SetLongFormName("print");
	SetShortDescription("Print content of variable");
}

void PrintCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	Frame* frame = console->GetCurrentFrame();

	UObject* obj = nullptr;
	bool bFoundObj = false;

	Array<std::string> chunks = SplitString(args, '.');

	if (NameString("self") == chunks[0])
	{
		obj = frame->Object;
	}
	else
	{
		for (UProperty* prop : frame->Func->Properties)
		{
			if (prop->Name == chunks[0] && (UObject::TryCast<UObjectProperty>(prop) || UObject::TryCast<UClassProperty>(prop)))
			{
				void* ptr = ((uint8_t*)frame->Variables.get()) + prop->DataOffset.DataOffset;
				obj = *(UObject**)ptr;
				bFoundObj = true;
				break;
			}
		}
	}

	if (!obj)
	{
		if (!bFoundObj)
			console->WriteOutput("Unknown variable " + chunks[0] + ResetEscape() + NewLine());
		else
			console->WriteOutput("None" + ResetEscape() + NewLine());
		return;
	}

	std::string name;
	std::string value;
	for (auto chunk = chunks.begin() + 1; chunk != chunks.end(); chunk++)
	{
		if (!obj)
		{
			console->WriteOutput(*chunk + ResetEscape() + " " + ColorEscape(96) + "None" + ResetEscape() + NewLine());
			return;
		}

		bFoundObj = false;
		for (UProperty* prop : obj->PropertyData.Class->Properties)
		{
			if (prop->Name == (*chunk))
			{
				void* ptr = ((uint8_t*)obj) + prop->DataOffset.DataOffset;
				void* val = obj->PropertyData.Ptr(prop);

				if (UObject::TryCast<UObjectProperty>(prop) || UObject::TryCast<UClassProperty>(prop))
				{
					obj = *(UObject**)val;
					bFoundObj = true;
					break;
				}
				else if (chunk+1 == chunks.end())
				{
					name = prop->Name.ToString();
					value = prop->PrintValue(val);

					if (name.size() < 40)
						name.resize(40, ' ');

					console->WriteOutput(name + ResetEscape() + " " + ColorEscape(96) + value + ResetEscape() + NewLine());
					return;
				}
			}
		}

		if (!bFoundObj)
		{
			console->WriteOutput("Unknown variable " + *chunk + ResetEscape() + NewLine());
			return;
		}
	}

	if (!obj)
	{
		std::string name = *(chunks.end() - 1);
		if (name.size() < 40)
			name.resize(40, ' ');

		console->WriteOutput(name + ResetEscape() + " " + ColorEscape(96) + "None" + ResetEscape() + NewLine());
		return;
	}

	auto props = obj->PropertyData.Class->Properties;
	std::stable_sort(props.begin(), props.end(), [](UProperty* a, UProperty* b) { return a->Name < b->Name; });
	for (UProperty* prop : props)
	{
		void* ptr = obj->PropertyData.Ptr(prop);

		std::string name = prop->Name.ToString();
		std::string value = prop->PrintValue(ptr);

		if (name.size() < 40)
			name.resize(40, ' ');

		console->WriteOutput(name + ResetEscape() + " " + ColorEscape(96) + value + ResetEscape() + NewLine());
	}
}

void PrintCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
