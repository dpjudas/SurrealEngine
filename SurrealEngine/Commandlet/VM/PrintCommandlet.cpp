
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
	if (NameString("self") == args)
	{
		obj = frame->Object;
	}
	else
	{
		for (UProperty* prop : frame->Func->Properties)
		{
			if (prop->Name == args && (UObject::TryCast<UObjectProperty>(prop) || UObject::TryCast<UClassProperty>(prop)))
			{
				void* ptr = ((uint8_t*)frame->Variables.get()) + prop->DataOffset;
				obj = *(UObject**)ptr;
				break;
			}
		}
	}

	if (!obj)
	{
		console->WriteOutput(ColorEscape(96) + "None" + ResetEscape() + NewLine());
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

		console->WriteOutput(ColorEscape(96) + name + ResetEscape() + " " + ColorEscape(96) + value + ResetEscape() + NewLine());
	}
}

void PrintCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
