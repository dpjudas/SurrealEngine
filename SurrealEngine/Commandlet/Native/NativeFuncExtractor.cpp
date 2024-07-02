
#include "Precomp.h"
#include "NativeFuncExtractor.h"
#include "Package/PackageManager.h"
#include "UObject/UClass.h"
#include "UObject/UProperty.h"
#include "VM/Bytecode.h"
#include "Utils/JsonValue.h"
#include "Utils/File.h"
#include <set>

std::string NativeFuncExtractor::Run(PackageManager* packages)
{
	JsonValue jsonRoot = JsonValue::object();

	//for (std::string pkgname : { "Core", "Engine", "IpDrv", "DeusEx", "Extension" } /* packages->GetPackageNames() */)
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

JsonValue NativeFuncExtractor::CreatePackageJson(Package* package)
{
	JsonValue jsonPackage = JsonValue::object();

	std::vector<UClass*> classes = package->GetAllObjects<UClass>();
	for (UClass* cls : classes)
	{
		JsonValue jsonClass = CreateClassJson(cls);
		if (jsonClass.properties().size() > 0)
		{
			jsonPackage.add(cls->Name.ToString(), jsonClass);
		}
	}

	return jsonPackage;
}

JsonValue NativeFuncExtractor::CreateClassJson(UClass* cls)
{
	JsonValue jsonClass = JsonValue::object();

	for (UField* child = cls->Children; child != nullptr; child = child->Next)
	{
		UFunction* func = UClass::TryCast<UFunction>(child);
		if (func && AllFlags(func->FuncFlags, FunctionFlags::Native))
		{
			jsonClass.add(func->Name.ToString(), CreateFunctionJson(func));
		}
	}

	return jsonClass;
}

JsonValue NativeFuncExtractor::CreateFunctionJson(UFunction* func)
{
	JsonValue jsonFunc = JsonValue::object();
	jsonFunc.add("NativeFuncIndex", JsonValue::number(func->NativeFuncIndex));
	jsonFunc.add("Static", JsonValue::boolean(AllFlags(func->FuncFlags, FunctionFlags::Static)));

	std::vector<std::string> args;
	for (UField* arg = func->Children; arg != nullptr; arg = arg->Next)
	{
		UProperty* prop = UClass::TryCast<UProperty>(arg);
		if (prop && AllFlags(prop->PropFlags, PropertyFlags::Parm))
		{
			std::string type;
			bool pointer = false;
			bool primitive = false;
			if (UClass::TryCast<UIntProperty>(prop)) { type = "int"; primitive = true; }
			else if (UClass::TryCast<UByteProperty>(prop)) { type = "uint8_t"; primitive = true; }
			else if (UClass::TryCast<UBoolProperty>(prop)) { type = "bool"; primitive = true; }
			else if (UClass::TryCast<UObjectProperty>(prop)) { type = "UObject*"; pointer = true; }
			else if (UClass::TryCast<UFloatProperty>(prop)) { type = "float"; primitive = true; }
			else if (UClass::TryCast<UFixedArrayProperty>(prop)) { type = "UFixedArray*"; pointer = true; }
			else if (UClass::TryCast<UArrayProperty>(prop)) { type = "UArray*"; pointer = true; }
			else if (UClass::TryCast<UMapProperty>(prop)) { type = "UMap*"; pointer = true; }
			else if (UClass::TryCast<UClassProperty>(prop)) { type = "UClass*"; pointer = true; }
			else if (UClass::TryCast<UStructProperty>(prop)) type = UClass::Cast<UStructProperty>(prop)->Struct->Name.ToString();
			else if (UClass::TryCast<UNameProperty>(prop)) type = "NameString";
			else if (UClass::TryCast<UStrProperty>(prop)) type = "std::string";
			else if (UClass::TryCast<UStringProperty>(prop)) type = "std::string";

			if (NameString(type) == "Vector") type = "vec3";

			if (AnyFlags(prop->PropFlags, PropertyFlags::OptionalParm | PropertyFlags::SkipParm))
			{
				if (type == "bool")
					type = "BitfieldBool*";
				else
					type += "*";
			}
			else if (AnyFlags(prop->PropFlags, PropertyFlags::OutParm | PropertyFlags::ReturnParm))
			{
				if (type == "bool")
					type = "BitfieldBool&";
				else
					type += "&";
			}
			else if (!pointer && !primitive)
			{
				type = "const " + type + "&";
			}

			args.push_back(type + " " + arg->Name.ToString());
		}
	}

	JsonValue jsonArgs = JsonValue::array(args);
	jsonFunc.add("Arguments", jsonArgs);
	return jsonFunc;
}
