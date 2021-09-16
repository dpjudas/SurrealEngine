
#include "Precomp.h"
#include "NativeObjExtractor.h"
#include "Package/PackageManager.h"
#include "UObject/UClass.h"
#include "UObject/UProperty.h"
#include "VM/Bytecode.h"
#include "File.h"
#include <set>

std::string NativeObjExtractor::Run(PackageManager* packages)
{
	std::string structs;
	std::string api;
	std::string inits;
	std::string pkginits;

	pkginits += "void InitPropertyOffsets(PackageManager* packages)\r\n{\r\n";

	for (std::string pkgname : { "Core", "Engine", "Fire", "IpDrv", "IpServer" })
	{
		Package* package = packages->GetPackage(pkgname);
		std::vector<UClass*> classes = package->GetAllClasses();

		for (UClass* cls : classes)
		{
			api += WriteClassHeader(cls);
			structs += WriteStructHeader(cls);
			inits += WriteStructBody(package, cls);
		}

		for (UClass* cls : classes)
		{
			bool propFound = false;
			for (UField* child = cls->Children; child != nullptr; child = child->Next)
			{
				UProperty* prop = UClass::TryCast<UProperty>(child);
				if (prop)
				{
					propFound = true;
					break;
				}
			}

			if (propFound)
				pkginits += "\tInitPropertyOffsets_" + cls->Name + "(packages);\r\n";
		}

	}

	pkginits += "}\r\n\r\n";

	return api + "\r\n\r\n////////////////\r\n\r\n" + structs + "\r\n\r\n////////////////\r\n\r\n" + inits + pkginits;
}

std::string NativeObjExtractor::WriteStructHeader(UClass* cls)
{
	std::map<std::string, std::string> locations;

	for (UField* child = cls->Children; child != nullptr; child = child->Next)
	{
		UProperty* prop = UClass::TryCast<UProperty>(child);
		if (prop)
		{
			locations[prop->Name] = "\tsize_t " + prop->Name + ";\r\n";
		}
	}
	if (locations.empty())
		return {};

	std::string header;
	header += "struct PropertyOffsets_" + cls->Name + "\r\n";
	header += "{\r\n";
	for (auto& it : locations)
		header += it.second;
	header += "};\r\n\r\n";
	header += "extern PropertyOffsets_" + cls->Name + " PropOffsets_" + cls->Name + ";\r\n\r\n";
	return header;
}

std::string NativeObjExtractor::WriteClassHeader(UClass* cls)
{
	std::map<std::string, std::string> props;
	std::map<std::string, std::string> locations;

	for (UField* child = cls->Children; child != nullptr; child = child->Next)
	{
		UProperty* prop = UClass::TryCast<UProperty>(child);
		if (prop)
		{
			props[prop->Name] = "\t" + WritePropertyGetter(cls->Name, prop) + "\r\n";
		}
	}
	if (props.empty())
		return {};

	std::string header;
	header += "\r\nclass U" + cls->Name + "\r\n";
	header += "{\r\npublic:\r\n";
	for (auto& it : props)
		header += it.second;
	header += "};\r\n";
	return header;
}

std::string NativeObjExtractor::WriteStructBody(Package* package, UClass* cls)
{
	std::map<std::string, std::string> props;

	for (UField* child = cls->Children; child != nullptr; child = child->Next)
	{
		UProperty* prop = UClass::TryCast<UProperty>(child);
		if (prop)
		{
			props[prop->Name] = "\tPropOffsets_" + cls->Name + "." + prop->Name + " = cls->GetProperty(\"" + prop->Name + "\")->DataOffset;\r\n";
		}
	}
	if (props.empty())
		return {};

	std::string body;

	body += "PropertyOffsets_" + cls->Name + " PropOffsets_" + cls->Name + ";\r\n\r\n";
	body += "static void InitPropertyOffsets_" + cls->Name + "(PackageManager* packages)\r\n{\r\n";
	body += "\tUClass* cls = dynamic_cast<UClass*>(packages->GetPackage(\"" + package->GetPackageName() + "\")->GetUObject(\"Class\", \"" + cls->Name + "\"));\r\n";
	body += "\tif (!cls)\r\n";
	body += "\tif (!cls)\r\n";
	body += "\t{\r\n";
	body += "\t\tmemset(&PropOffsets_" + cls->Name + ", 0xff, sizeof(PropOffsets_" + cls->Name + ")); \r\n";
	body += "\t\treturn;\r\n";
	body += "\t}\r\n";

	body += "\t\tthrow std::runtime_error(\"Could not find class object for " + cls->Name + "\");\r\n";
	for (auto& it : props)
		body += it.second;
	body += "}\r\n\r\n";

	return body;
}

std::string NativeObjExtractor::WritePropertyGetter(const std::string& clsname, UProperty* prop)
{
	std::string type;
	std::string comment;

	if (UClass::TryCast<UIntProperty>(prop)) type = "int";
	else if (UClass::TryCast<UByteProperty>(prop)) type = "uint8_t";
	else if (UClass::TryCast<UBoolProperty>(prop)) type = "bool";
	else if (UClass::TryCast<UObjectProperty>(prop)) { type = "UObject*"; auto objprop = UClass::Cast<UObjectProperty>(prop); if (objprop->ObjectClass && objprop->ObjectClass->Name != "Object") type = "U" + objprop->ObjectClass->Name + "*"; }
	else if (UClass::TryCast<UFloatProperty>(prop)) type = "float";
	else if (UClass::TryCast<UFixedArrayProperty>(prop)) type = "std::vector<void*>";
	else if (UClass::TryCast<UArrayProperty>(prop)) type = "std::vector<void*>";
	else if (UClass::TryCast<UMapProperty>(prop)) type = "std::map<void*, void*>*";
	else if (UClass::TryCast<UClassProperty>(prop)) type = "UClass*";
	else if (UClass::TryCast<UStructProperty>(prop)) type = UClass::Cast<UStructProperty>(prop)->Struct->Name;
	else if (UClass::TryCast<UNameProperty>(prop)) type = "NameString";
	else if (UClass::TryCast<UStrProperty>(prop)) type = "std::string";
	else if (UClass::TryCast<UStringProperty>(prop)) type = "std::string";

	if (type == "Vector") type = "vec3";

	if (AllFlags(prop->PropFlags, PropertyFlags::Native))
	{
		if (!comment.empty())
			comment += ", ";
		comment += "native";
	}

	if (!comment.empty())
		comment = " // " + comment;

	return type + "& " + prop->Name + "() { return Value<" + type + ">(PropOffsets_" + clsname + "." + prop->Name + "); }" + comment;
}
