
#include "Precomp.h"
#include "UClassProperty.h"
#include "Packages/Core/UClass.h"

void UClassProperty::Load(ObjectStream* stream)
{
	UObjectProperty::Load(stream);
	MetaClass = stream->ReadObject<UClass>();
}

void UClassProperty::Save(PackageStreamWriter* stream)
{
	UObjectProperty::Save(stream);
	stream->WriteObject(MetaClass);
}

std::string UClassProperty::PrintValue(const void* data)
{
	UObject* obj = *(UObject**)data;
	if (obj)
		return "Class'" + obj->package->GetPackageName().ToString() + '.' + obj->Name.ToString() + '\'';
	else
		return "None";
}
