
#include "Precomp.h"
#include "UClass.h"
#include "UTextBuffer.h"
#include "UProperty.h"
#include "VM/Bytecode.h"
#include "VM/NativeFunc.h"
#include "Package/PackageManager.h"

void UField::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	BaseField = stream->ReadObject<UField>();
	Next = stream->ReadObject<UField>();
}

/////////////////////////////////////////////////////////////////////////////

void UConst::Load(ObjectStream* stream)
{
	UField::Load(stream);
	Constant = stream->ReadString();
}

/////////////////////////////////////////////////////////////////////////////

void UEnum::Load(ObjectStream* stream)
{
	UField::Load(stream);
	int size = stream->ReadIndex();
	for (int i = 0; i < size; i++)
		ElementNames.push_back(stream->ReadName());
}

/////////////////////////////////////////////////////////////////////////////

void UStruct::Load(ObjectStream* stream)
{
	UField::Load(stream);
	ScriptText = stream->ReadObject<UTextBuffer>();
	Children = stream->ReadObject<UField>();
	FriendlyName = stream->ReadName();
	if (FriendlyName == "None")
		throw std::runtime_error("Struct FriendlyName must not be None");

	Line = stream->ReadUInt32();
	TextPos = stream->ReadUInt32();

	/*
	// for debug breakpoints
	if (FriendlyName == "UBrowserServerListWindow")
		FriendlyName = "UBrowserServerListWindow";
	if (FriendlyName == "BeforePaint")
		FriendlyName = "BeforePaint";
	*/

	int ScriptSize = stream->ReadUInt32();
	while (Bytecode.size() < ScriptSize)
	{
		ReadToken(stream, 0);
	}
	if (Bytecode.size() != ScriptSize)
		throw std::runtime_error("Bytecode load failed");

	Code = std::make_shared<::Bytecode>(Bytecode, stream->GetPackage());

	size_t offset = 0;
	if (Base)
	{
		Base->LoadNow();
		Properties = Base->Properties;
		offset = Base->StructSize;
	}
	UField* child = Children;
	while (child)
	{
		child->LoadNow();

		UProperty* prop = dynamic_cast<UProperty*>(child);
		if (prop)
		{
			Properties.push_back(prop);

			size_t alignment = prop->Alignment();
			size_t size = prop->Size();
			prop->DataOffset = (offset + alignment - 1) / alignment * alignment;
			offset = prop->DataOffset + size;
		}
		else if (dynamic_cast<UStruct*>(child))
		{
			static_cast<UStruct*>(child)->StructParent = this;
		}

		child = child->Next;
	}

	child = Children;
	while (child)
	{
		UFunction* func = dynamic_cast<UFunction*>(child);
		if (func && AllFlags(func->FuncFlags, FunctionFlags::Native))
		{
			func->NativeStruct = this;
			NativeFunctions::RegisterNativeFunc(func);
		}
		child = child->Next;
	}

	size_t alignment = sizeof(void*);
	StructSize = (offset + alignment - 1) / alignment * alignment;
}

#ifdef _DEBUG
static const char* tokennames[256] =
{
	"LocalVariable", "InstanceVariable", "DefaultVariable", "0x03", "Return", "Switch", "Jump", "JumpIfNot",
	"Stop", "Assert", "Case", "Nothing", "LabelTable", "GotoLabel", "EatString", "Let",
	"DynArrayElement", "New", "ClassContext", "MetaCast", "LetBool", "Unknown0x15", "EndFunctionParms", "Self",
	"Skip", "Context", "ArrayElement", "VirtualFunction", "FinalFunction", "IntConst", "FloatConst", "StringConst",
	"ObjectConst", "NameConst", "RotationConst", "VectorConst", "ByteConst", "IntZero", "IntOne", "True",
	"False", "NativeParm", "NoObject", "Unknown0x2b", "IntConstByte", "BoolVariable", "DynamicCast", "Iterator",
	"IteratorPop", "IteratorNext", "StructCmpEq", "StructCmpNe", "UnicodeStringConst", "0x35", "StructMember", "0x37",
	"GlobalFunction", "RotatorToVector", "ByteToInt", "ByteToBool", "ByteToFloat", "IntToByte", "IntToBool", "IntToFloat",
	"BoolToByte", "BoolToInt", "BoolToFloat", "FloatToByte", "FloatToInt", "FloatToBool", "Unknown0x46", "ObjectToBool",
	"NameToBool", "StringToByte", "StringToInt", "StringToBool", "StringToFloat", "StringToVector", "StringToRotator", "VectorToBool",
	"VectorToRotator", "RotatorToBool", "ByteToString", "IntToString", "BoolToString", "FloatToString", "ObjectToString", "NameToString",
	"VectorToString", "RotatorToString", "0x5a", "0x5b", "0x5c", "0x5d", "0x5e", "0x5f",
	"ExtendedNative60", "ExtendedNative61", "ExtendedNative62", "ExtendedNative63", "ExtendedNative64", "ExtendedNative65", "ExtendedNative66", "ExtendedNative67",
	"ExtendedNative68", "ExtendedNative69", "ExtendedNative6A", "ExtendedNative6B", "ExtendedNative6C", "ExtendedNative6D", "ExtendedNative6E", "ExtendedNative6F",
	"Native70", "Native71", "Native72", "Native73", "Native74", "Native75", "Native76", "Native77",
	"Native78", "Native79", "Native7A", "Native7B", "Native7C", "Native7D", "Native7E", "Native7F",
	"Native80", "Native81", "Native82", "Native83", "Native84", "Native85", "Native86", "Native87",
	"Native88", "Native89", "Native8A", "Native8B", "Native8C", "Native8D", "Native8E", "Native8F",
	"Native90", "Native91", "Native92", "Native93", "Native94", "Native95", "Native96", "Native97",
	"Native98", "Native99", "Native9A", "Native9B", "Native9C", "Native9D", "Native9E", "Native9F",
	"NativeA0", "NativeA1", "NativeA2", "NativeA3", "NativeA4", "NativeA5", "NativeA6", "NativeA7",
	"NativeA8", "NativeA9", "NativeAA", "NativeAB", "NativeAC", "NativeAD", "NativeAE", "NativeAF",
	"NativeB0", "NativeB1", "NativeB2", "NativeB3", "NativeB4", "NativeB5", "NativeB6", "NativeB7",
	"NativeB8", "NativeB9", "NativeBA", "NativeBB", "NativeBC", "NativeBD", "NativeBE", "NativeBF",
	"NativeC0", "NativeC1", "NativeC2", "NativeC3", "NativeC4", "NativeC5", "NativeC6", "NativeC7",
	"NativeC8", "NativeC9", "NativeCA", "NativeCB", "NativeCC", "NativeCD", "NativeCE", "NativeCF",
	"NativeD0", "NativeD1", "NativeD2", "NativeD3", "NativeD4", "NativeD5", "NativeD6", "NativeD7",
	"NativeD8", "NativeD9", "NativeDA", "NativeDB", "NativeDC", "NativeDD", "NativeDE", "NativeDF",
	"NativeE0", "NativeE1", "NativeE2", "NativeE3", "NativeE4", "NativeE5", "NativeE6", "NativeE7",
	"NativeE8", "NativeE9", "NativeEA", "NativeEB", "NativeEC", "NativeED", "NativeEE", "NativeEF",
	"NativeF0", "NativeF1", "NativeF2", "NativeF3", "NativeF4", "NativeF5", "NativeF6", "NativeF7",
	"NativeF8", "NativeF9", "NativeFA", "NativeFB", "NativeFC", "NativeFD", "NativeFE", "NativeFF"
};
#endif

ExprToken UStruct::ReadToken(ObjectStream* stream, int depth)
{
	if (depth == 64)
		throw std::runtime_error("Bytecode parsing error");
	depth++;

	ExprToken token = (ExprToken)stream->ReadUInt8();
	PushUInt8((uint8_t)token);

#ifdef _DEBUG
	std::string tokendebug;
	tokendebug.resize(depth - 1, '\t');
	tokendebug += tokennames[(uint8_t)token];
	tokendebug += "\r\n";
	BytecodePlainText += tokendebug;
#endif

	if (token >= ExprToken::MinConversion && token <= ExprToken::MaxConversion)
	{
		ReadToken(stream, depth);
	}
	else if (token >= ExprToken::FirstNative)
	{
		//int nativeindex = (int)token;
		while (ReadToken(stream, depth) != ExprToken::EndFunctionParms);
	}
	else if (token >= ExprToken::ExtendedNative)
	{
		int part2 = stream->ReadUInt8();
		PushUInt8(part2);
		//int nativeindex = (((int)token - (int)ExprToken::ExtendedNative) << 8) + part2;
		while (ReadToken(stream, depth) != ExprToken::EndFunctionParms);
	}
	else if (token == ExprToken::VirtualFunction)
	{
		int name = stream->ReadIndex();
		PushIndex(name);
		while (ReadToken(stream, depth) != ExprToken::EndFunctionParms);
	}
	else if (token == ExprToken::FinalFunction)
	{
		int object = stream->ReadIndex();
		PushIndex(object);
		while (ReadToken(stream, depth) != ExprToken::EndFunctionParms);
	}
	else if (token == ExprToken::GlobalFunction)
	{
		int name = stream->ReadIndex();
		PushIndex(name);
		while (ReadToken(stream, depth) != ExprToken::EndFunctionParms);
	}
	else
	{
		switch (token)
		{
		case ExprToken::LocalVariable: PushIndex(stream->ReadIndex()); break;
		case ExprToken::InstanceVariable: PushIndex(stream->ReadIndex()); break;
		case ExprToken::DefaultVariable: PushIndex(stream->ReadIndex()); break;
		case ExprToken::Return: ReadToken(stream, depth); break;
		case ExprToken::Switch: PushUInt8(stream->ReadUInt8()); ReadToken(stream, depth); break;
		case ExprToken::Jump: PushUInt16(stream->ReadUInt16()); break;
		case ExprToken::JumpIfNot: PushUInt16(stream->ReadUInt16()); ReadToken(stream, depth); break;
		case ExprToken::Stop: break;
		case ExprToken::Assert: PushUInt16(stream->ReadUInt16()); ReadToken(stream, depth); break;
		case ExprToken::Case: { uint16_t nextoffset = stream->ReadUInt16(); PushUInt16(nextoffset); if (nextoffset != 0xffff) ReadToken(stream, depth); break; }
		case ExprToken::Nothing: break;
		case ExprToken::LabelTable: while (true) { int name = stream->ReadIndex(); PushIndex(name); PushUInt32(stream->ReadUInt32()); if (stream->GetPackage()->GetName(name) == "None") break; } break;
		case ExprToken::GotoLabel: ReadToken(stream, depth); break;
		case ExprToken::EatString: ReadToken(stream, depth); break;
		case ExprToken::Let: ReadToken(stream, depth); ReadToken(stream, depth); break;
		case ExprToken::DynArrayElement: ReadToken(stream, depth); ReadToken(stream, depth); break;
		case ExprToken::New: ReadToken(stream, depth); ReadToken(stream, depth); ReadToken(stream, depth); ReadToken(stream, depth); break;
		case ExprToken::ClassContext: ReadToken(stream, depth); PushUInt16(stream->ReadUInt16()); PushUInt8(stream->ReadUInt8()); ReadToken(stream, depth); break;
		case ExprToken::MetaCast: PushIndex(stream->ReadIndex()); ReadToken(stream, depth); break;
		case ExprToken::LetBool: ReadToken(stream, depth); ReadToken(stream, depth); break;
		case ExprToken::Unknown0x15: ReadToken(stream, depth); break;
		case ExprToken::EndFunctionParms: break;
		case ExprToken::Self: break;
		case ExprToken::Skip: PushUInt16(stream->ReadUInt16()); ReadToken(stream, depth); break;
		case ExprToken::Context: ReadToken(stream, depth); PushUInt16(stream->ReadUInt16()); PushUInt8(stream->ReadUInt8()); ReadToken(stream, depth); break;
		case ExprToken::ArrayElement: ReadToken(stream, depth); ReadToken(stream, depth); break;
		case ExprToken::IntConst: PushUInt32(stream->ReadUInt32()); break;
		case ExprToken::FloatConst: PushFloat(stream->ReadFloat()); break;
		case ExprToken::StringConst: PushAsciiZ(stream->ReadAsciiZ()); break;
		case ExprToken::ObjectConst: PushIndex(stream->ReadIndex()); break;
		case ExprToken::NameConst: PushIndex(stream->ReadIndex()); break;
		case ExprToken::RotationConst: PushUInt32(stream->ReadUInt32()); PushUInt32(stream->ReadUInt32()); PushUInt32(stream->ReadUInt32()); break;
		case ExprToken::VectorConst: PushFloat(stream->ReadFloat()); PushFloat(stream->ReadFloat()); PushFloat(stream->ReadFloat()); break;
		case ExprToken::ByteConst: PushUInt8(stream->ReadUInt8()); break;
		case ExprToken::IntZero: break;
		case ExprToken::IntOne: break;
		case ExprToken::True: break;
		case ExprToken::False: break;
		case ExprToken::NativeParm: PushIndex(stream->ReadIndex()); break;
		case ExprToken::NoObject: break;
		case ExprToken::Unknown0x2b: PushUInt8(stream->ReadUInt8()); ReadToken(stream, depth); break;
		case ExprToken::IntConstByte: PushUInt8(stream->ReadUInt8()); break;
		case ExprToken::BoolVariable: break;
		case ExprToken::DynamicCast: PushIndex(stream->ReadIndex()); ReadToken(stream, depth); break;
		case ExprToken::Iterator: ReadToken(stream, depth); PushUInt16(stream->ReadUInt16()); break;
		case ExprToken::IteratorPop: break;
		case ExprToken::IteratorNext: break;
		case ExprToken::StructCmpEq: PushIndex(stream->ReadIndex()); ReadToken(stream, depth); ReadToken(stream, depth); break;
		case ExprToken::StructCmpNe: PushIndex(stream->ReadIndex()); ReadToken(stream, depth); ReadToken(stream, depth); break;
		case ExprToken::UnicodeStringConst: PushUnicodeZ(stream->ReadUnicodeZ()); break;
		case ExprToken::StructMember: PushIndex(stream->ReadIndex()); ReadToken(stream, depth); break;
		default: throw std::runtime_error("Unknown script bytecode token encountered");
		}
	}
	return token;
}

void UStruct::PushBytes(const void* data, size_t size)
{
	Bytecode.insert(Bytecode.end(), (const uint8_t*)data, (const uint8_t*)data + size);
}

void UStruct::PushUInt8(uint8_t value)
{
	Bytecode.push_back(value);
}

void UStruct::PushUInt16(uint16_t value)
{
	PushBytes(&value, sizeof(uint16_t));
}

void UStruct::PushUInt32(uint32_t value)
{
	PushBytes(&value, sizeof(uint32_t));
}

void UStruct::PushIndex(int32_t value)
{
	PushBytes(&value, sizeof(int32_t));
}

void UStruct::PushFloat(float value)
{
	PushBytes(&value, sizeof(float));
}

void UStruct::PushAsciiZ(const std::string& value)
{
	PushBytes(value.c_str(), value.length() + 1);
}

void UStruct::PushUnicodeZ(const std::wstring& value)
{
	PushBytes(value.c_str(), (value.length() + 1) * 2);
}

/////////////////////////////////////////////////////////////////////////////

void UFunction::Load(ObjectStream* stream)
{
	UStruct::Load(stream);
	if (stream->GetVersion() <= 63)
		ParmsSize = stream->ReadIndex();
	NativeFuncIndex = stream->ReadUInt16();
	if (stream->GetVersion() <= 63)
		NumParms = stream->ReadIndex();
	OperatorPrecedence = stream->ReadUInt8();
	if (stream->GetVersion() <= 63)
		ReturnValueOffset = stream->ReadIndex();
	FuncFlags = (FunctionFlags)stream->ReadUInt32();
	if (AllFlags(FuncFlags, FunctionFlags::Net))
		ReplicationOffset = stream->ReadUInt16();
}

/////////////////////////////////////////////////////////////////////////////

void UState::Load(ObjectStream* stream)
{
	UStruct::Load(stream);
	ProbeMask = stream->ReadUInt64();
	IgnoreMask = stream->ReadUInt64();
	LabelTableOffset = stream->ReadUInt16();
	StateFlags = (ScriptStateFlags)stream->ReadUInt32();
}

/////////////////////////////////////////////////////////////////////////////

void UClass::Load(ObjectStream* stream)
{
	UState::Load(stream);

	if (stream->GetVersion() <= 61)
	{
		OldClassRecordSize = stream->ReadUInt32();
		Flags = Flags | ObjectFlags::Public | ObjectFlags::Standalone;
	}

	ClassFlags = stream->ReadUInt32();
	stream->ReadBytes(ClassGuid.Data, 16);

	int NumDependencies = stream->ReadIndex();
	for (int i = 0; i < NumDependencies; i++)
	{
		ClassDependency dep;
		dep.Class = stream->ReadObject<UClass>();
		dep.Deep = stream->ReadUInt32();
		dep.ScriptTextCRC = stream->ReadUInt32();
		Dependencies.push_back(dep);
	}

	int NumPackageImports = stream->ReadIndex();
	for (int i = 0; i < NumPackageImports; i++)
		PackageImports.push_back(stream->ReadIndex());

	if (stream->GetVersion() >= 62)
	{
		ClassWithin = stream->ReadIndex();
		ClassConfigName = stream->ReadName();
	}

	PropertyData.Init(this);
	PropertyData.ReadProperties(stream);

	auto packages = stream->GetPackage()->GetPackageManager();
	std::string sectionName = stream->GetPackage()->GetPackageName() + "." + Name;
	std::string configName = ClassConfigName;
	if (configName.empty()) configName = "system";
	for (UProperty* prop : Properties)
	{
		if (AllFlags(prop->PropFlags, PropertyFlags::Config) || (AllFlags(prop->PropFlags, PropertyFlags::GlobalConfig) && prop->Outer() == this))
		{
			std::string value = packages->GetIniValue(configName, sectionName, prop->Name);
			if (!value.empty())
			{
				void* ptr = PropertyData.Ptr(prop);
				if (dynamic_cast<UByteProperty*>(prop)) *static_cast<uint8_t*>(ptr) = (uint8_t)std::atoi(value.c_str());
				else if (dynamic_cast<UIntProperty*>(prop)) *static_cast<int32_t*>(ptr) = (int32_t)std::atoi(value.c_str());
				else if (dynamic_cast<UFloatProperty*>(prop)) *static_cast<float*>(ptr) = (float)std::atof(value.c_str());
				else if (dynamic_cast<UNameProperty*>(prop)) *static_cast<std::string*>(ptr) = value;
				else if (dynamic_cast<UStrProperty*>(prop)) *static_cast<std::string*>(ptr) = value;
				else if (dynamic_cast<UStringProperty*>(prop)) *static_cast<std::string*>(ptr) = value;
				else if (dynamic_cast<UBoolProperty*>(prop))
				{
					for (char& c : value)
						if (c >= 'A' && c <= 'Z')
							c += 'a' - 'A';
					*static_cast<bool*>(ptr) = (value == "1" || value == "true" || value == "yes");
				}
			}
		}
		else if (AllFlags(prop->PropFlags, PropertyFlags::Localized))
		{
			// To do: read this from <packagename>.int
		}
	}
}

UProperty* UClass::GetProperty(const std::string& name)
{
	for (UProperty* prop : PropertyData.Class->Properties)
	{
		if (prop->Name == name)
			return prop;
	}
	throw std::runtime_error("Property '" + name + "' not found");
}
