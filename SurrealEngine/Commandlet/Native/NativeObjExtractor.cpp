
#include "Precomp.h"
#include "NativeObjExtractor.h"
#include "Package/PackageManager.h"
#include "UObject/UClass.h"
#include "UObject/UProperty.h"
#include "VM/Bytecode.h"
#include "Utils/File.h"
#include <set>

std::string NativeObjExtractor::Run(PackageManager* packages)
{
	JsonValue jsonRoot = JsonValue::object();

	for (NameString pkgname : packages->GetPackageNames())
	{
		if (pkgname == "Editor")
			continue;

		Package* package = packages->GetPackage(pkgname.ToString());

		JsonValue jsonPackage = CreatePackageJson(package);
		if (jsonPackage.properties().size() > 0)
		{
			jsonRoot.add(pkgname.ToString(), jsonPackage);
		}
	}

	return jsonRoot.to_json(true);
}

JsonValue NativeObjExtractor::CreatePackageJson(Package* package)
{
	JsonValue jsonPackage = JsonValue::object();

	Array<UClass*> classes = package->GetAllObjects<UClass>();
	for (UClass* cls : classes)
	{
		if (AllFlags(cls->Flags, ObjectFlags::Native))
		{
			JsonValue jsonClass = CreateClassJson(cls);
			if (jsonClass.properties().size() > 0)
			{
				jsonPackage.add(cls->Name.ToString(), jsonClass);
			}
		}
	}

	return jsonPackage;
}

JsonValue NativeObjExtractor::CreateClassJson(UClass* cls)
{
	JsonValue jsonClass = JsonValue::object();
	for (UField* child = cls->Children; child != nullptr; child = child->Next)
	{
		UProperty* prop = UClass::TryCast<UProperty>(child);
		if (prop)
		{
			std::string type;
			std::string comment;

			if (UClass::TryCast<UIntProperty>(prop)) type = "int";
			else if (UClass::TryCast<UByteProperty>(prop)) type = "uint8_t";
			else if (UClass::TryCast<UBoolProperty>(prop)) type = "BitfieldBool";
			else if (UClass::TryCast<UObjectProperty>(prop)) 
			{ 
				type = "UObject*"; 
				auto objprop = UClass::Cast<UObjectProperty>(prop); 
				if (objprop->ObjectClass && objprop->ObjectClass->Name != "Object") 
					type = "U" + objprop->ObjectClass->Name.ToString() + "*"; 
			}
			else if (UClass::TryCast<UFloatProperty>(prop)) type = "float";
			else if (UClass::TryCast<UFixedArrayProperty>(prop)) type = "Array<void*>";
			else if (UClass::TryCast<UArrayProperty>(prop)) type = "Array<void*>";
			else if (UClass::TryCast<UMapProperty>(prop)) type = "std::map<void*, void*>*";
			else if (UClass::TryCast<UClassProperty>(prop)) type = "UClass*";
			else if (UClass::TryCast<UStructProperty>(prop)) type = UClass::Cast<UStructProperty>(prop)->Struct->Name.ToString();
			else if (UClass::TryCast<UNameProperty>(prop)) type = "NameString";
			else if (UClass::TryCast<UStrProperty>(prop)) type = "std::string";
			else if (UClass::TryCast<UStringProperty>(prop)) type = "std::string";

			if (NameString(type) == "Vector")
				type = "vec3";

			jsonClass.add(prop->Name.ToString(), JsonValue::string(type));
		}
	}
	return jsonClass;
}
