
#include "Precomp.h"
#include "NativeFuncExtractor.h"
#include "Package/PackageManager.h"
#include "UObject/UClass.h"
#include "UObject/UProperty.h"
#include "VM/Bytecode.h"
#include "File.h"
#include <set>

std::string NativeFuncExtractor::Run(PackageManager* packages)
{
	std::string headers;
	std::string bodies;

	//for (std::string pkgname : { "Core", "Engine", "IpDrv", "DeusEx", "Extension" } /* packages->GetPackageNames() */)
	for (NameString pkgname : packages->GetPackageNames())
	{
		if (pkgname == "Editor")
			continue;
		Package* package = packages->GetPackage(pkgname.ToString());
		std::vector<UClass*> classes = package->GetAllClasses();

		for (UClass* cls : classes)
		{
			headers += WriteClassHeader(cls);
			bodies += WriteClassBody(cls);
		}
	}

	return headers + "\r\n\r\n" + bodies;
}

std::string NativeFuncExtractor::WriteClassHeader(UClass* cls)
{
	std::map<NameString, std::string> funcs;

	for (UField* child = cls->Children; child != nullptr; child = child->Next)
	{
		UFunction* func = UClass::TryCast<UFunction>(child);
		if (func && AllFlags(func->FuncFlags, FunctionFlags::Native))
		{
			funcs[func->Name] = "\tstatic void " + WriteFunctionSignature(func) + ";\r\n";
		}
	}
	if (funcs.empty())
		return {};

	std::string header;
	header += "\r\nclass N" + cls->Name.ToString() + "\r\n";
	header += "{\r\npublic:\r\n\tstatic void RegisterFunctions();\r\n\r\n";
	for (auto& it : funcs) header += it.second;
	header += "};\r\n";
	return header;
}

std::string NativeFuncExtractor::WriteClassBody(UClass* cls)
{
	std::map<NameString, std::string> funcs;

	for (UField* child = cls->Children; child != nullptr; child = child->Next)
	{
		UFunction* func = UClass::TryCast<UFunction>(child);
		if (func && AllFlags(func->FuncFlags, FunctionFlags::Native))
		{
			funcs[func->Name] = "void N" + cls->Name.ToString() + "::" + WriteFunctionSignature(func) + "\r\n{\r\n\tthrow std::runtime_error(\"" + cls->Name.ToString() + "." + func->Name.ToString() + " not implemented\");\r\n}\r\n\r\n";
		}
	}

	if (funcs.empty())
		return {};

	std::map<std::string, std::string> registrations;
	for (UField* child = cls->Children; child != nullptr; child = child->Next)
	{
		UFunction* func = UClass::TryCast<UFunction>(child);
		if (func && AllFlags(func->FuncFlags, FunctionFlags::Native))
		{
			int numargs = 0;
			for (UField* arg = func->Children; arg != nullptr; arg = arg->Next)
			{
				UProperty* prop = UClass::TryCast<UProperty>(arg);
				if (prop && AllFlags(prop->PropFlags, PropertyFlags::Parm))
					numargs++;
			}

			registrations[func->Name.ToString()] = "\tRegisterVMNativeFunc_" + std::to_string(numargs) + "(\"" + cls->Name.ToString() + "\", \"" + func->Name.ToString() + "\", &N" + cls->Name.ToString() + "::" + func->Name.ToString() + ", " + std::to_string(func->NativeFuncIndex) + ");\r\n";
		}
	}

	std::string body;

	body += "void N" + cls->Name.ToString() + "::RegisterFunctions()\r\n{\r\n";
	for (auto& it : registrations) body += it.second;
	body += "}\r\n\r\n";
	for (auto& it : funcs) body += it.second;
	return body;
}

std::string NativeFuncExtractor::WriteFunctionSignature(UFunction* func)
{
	std::string args;

	if (!AllFlags(func->FuncFlags, FunctionFlags::Static))
	{
		args += "UObject* Self";
	}

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

			if (!args.empty()) args += ", ";

			if (AnyFlags(prop->PropFlags, PropertyFlags::OptionalParm | PropertyFlags::SkipParm)) type += "*";
			else if (AnyFlags(prop->PropFlags, PropertyFlags::OutParm | PropertyFlags::ReturnParm)) type += "&";
			else if (!pointer && !primitive) type = "const " + type + "&";

			args += type + " " + prop->Name.ToString();
		}
	}

	return func->Name.ToString() + "(" + args + ")";
}

