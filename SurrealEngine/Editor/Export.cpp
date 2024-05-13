#include "Editor/Export.h"
#include "UObject/UProperty.h"
#include "UObject/UTextBuffer.h"

std::string Exporter::ExportObject(UObject* obj, int tablevel, bool bInline)
{
	std::string txt = "";
	if (bInline)
	{
		txt.append(tablevel - 1, '\t');
		txt.append("Begin Object Class=");
		txt.append(obj->Class->Name.ToString());
		txt.append(" Name=");
		txt.append(obj->Name.ToString());
		txt.append("\r\n");
	}
	else
	{
		txt = "\r\ndefaultproperties\r\n{\r\n";
	}

	for (UProperty* prop : obj->Class->Properties)
	{
		for (int i = 0; i < prop->ArrayDimension; i++)
		{
			if (AnyFlags(prop->Flags, ObjectFlags::TagExp))
			{
				// Get default property from super class
				UObject* default = nullptr;
				if (obj->Class == obj->Class->Class)
					default = obj->Class->BaseStruct;
				else
					default = obj->Class;

				if (prop->Name == "Tag")
				{
					if (obj->Class->GetPropertyAsString(prop->Name) == obj->Name.ToString())
						continue;
				}

				// TODO: implement inline declared objects here
				// not necessary until we support <= 227j

				std::string tabs(tablevel, '\t');
				prop->GetExportText(txt, tabs, obj, default, i);
			}
		}
	}

	if (bInline)
	{
		txt.append(tablevel - 1, '\t');
		txt.append("End Object\n");
	}
	else
	{
		txt.append("}\r\n");
	}

	return txt;
}

std::string Exporter::ExportClass(UClass* cls)
{
	if (!cls->ScriptText)
		return "";

	return cls->ScriptText->Text + ExportObject(cls->GetDefaultObject(), 1, false);
}