
#include "Precomp.h"
#include "UClass.h"
#include "UTextBuffer.h"
#include "UProperty.h"
#include "VM/Bytecode.h"
#include "VM/NativeFunc.h"
#include "VM/ScriptCall.h"
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

UStruct::UStruct(NameString name, UClass* cls, ObjectFlags flags) : UField(std::move(name), cls, flags)
{
}

UStruct::UStruct(NameString name, UClass* cls, ObjectFlags flags, UStruct* base) : UField(std::move(name), cls, flags)
{
	BaseStruct = base;
}

void UStruct::Load(ObjectStream* stream)
{
	UField::Load(stream);
	ScriptText = stream->ReadObject<UTextBuffer>();
	if (ScriptText)
		ScriptText->LoadNow();
	Children = stream->ReadObject<UField>();
	FriendlyName = stream->ReadName();
	if (FriendlyName == "None")
		Exception::Throw("Struct FriendlyName must not be None");

	Line = stream->ReadUInt32();
	TextPos = stream->ReadUInt32();

	int ScriptSize = stream->ReadUInt32();

	while (Bytecode.size() < ScriptSize)
	{
		ReadToken(stream, 0);
	}
	if (Bytecode.size() != ScriptSize)
		Exception::Throw("Bytecode load failed");

	Code = std::make_shared<::Bytecode>(Bytecode, stream->GetPackage());

	size_t offset = 0;
	if (BaseStruct)
	{
		BaseStruct->LoadNow();
		Properties = BaseStruct->Properties;
		offset = BaseStruct->StructSize;
	}

	uint64_t bitfieldMask = 1;

	UField* child = Children;
	while (child)
	{
		child->LoadNow();

		if (UProperty* prop = dynamic_cast<UBoolProperty*>(child))
		{
			// Pack bool properties into 32 bit bitfields
			Properties.push_back(prop);
			if (bitfieldMask == 1 || bitfieldMask == (1ULL << 32))
			{
				size_t alignment = prop->Alignment();
				size_t size = prop->Size();
				prop->DataOffset.DataOffset = (offset + alignment - 1) / alignment * alignment;
				prop->DataOffset.BitfieldMask = 1;
				bitfieldMask = 2;
				offset = prop->DataOffset.DataOffset + size;
			}
			else
			{
				prop->DataOffset.DataOffset = offset - prop->Size();
				prop->DataOffset.BitfieldMask = (uint32_t)bitfieldMask;
				bitfieldMask <<= 1;
			}
		}
		else if (UProperty* prop = dynamic_cast<UProperty*>(child))
		{
			Properties.push_back(prop);
			bitfieldMask = 1;

			size_t alignment = prop->Alignment();
			size_t size = prop->Size();
			prop->DataOffset.DataOffset = (offset + alignment - 1) / alignment * alignment;
			offset = prop->DataOffset.DataOffset + size;
		}
		else if (UStruct* childstruct = dynamic_cast<UStruct*>(child))
		{
			bitfieldMask = 1;
			childstruct->StructParent = this;
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

	if (FriendlyName == "Vector" || FriendlyName == "Rotator")
	{
		size_t alignment = sizeof(uint32_t);
		StructSize = (offset + alignment - 1) / alignment * alignment;
	}
	else
	{
		size_t alignment = sizeof(void*);
		StructSize = (offset + alignment - 1) / alignment * alignment;
	}
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
		Exception::Throw("Bytecode parsing error");
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
	else if (token == ExprToken::LetBool && stream->GetVersion() <= 63)
	{
		while (true)
		{
			uint8_t size = stream->ReadUInt8();
			PushUInt8(size);
			if (size == 0)
				break;
			PushUInt8(stream->ReadUInt8());
		}
	}
	else
	{
		switch (token)
		{
		case ExprToken::LocalVariable: PushIndex(stream->ReadIndex()); break;
		case ExprToken::InstanceVariable: PushIndex(stream->ReadIndex()); break;
		case ExprToken::DefaultVariable: PushIndex(stream->ReadIndex()); break;
		case ExprToken::Return: if (stream->GetVersion() > 61) ReadToken(stream, depth); break;
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
		case ExprToken::Unknown0x15: /*ReadToken(stream, depth);*/ break;
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
		case ExprToken::BoolVariable: ReadToken(stream, depth); break;
		case ExprToken::DynamicCast: PushIndex(stream->ReadIndex()); ReadToken(stream, depth); break;
		case ExprToken::Iterator: ReadToken(stream, depth); PushUInt16(stream->ReadUInt16()); break;
		case ExprToken::IteratorPop: break;
		case ExprToken::IteratorNext: break;
		case ExprToken::StructCmpEq: PushIndex(stream->ReadIndex()); ReadToken(stream, depth); ReadToken(stream, depth); break;
		case ExprToken::StructCmpNe: PushIndex(stream->ReadIndex()); ReadToken(stream, depth); ReadToken(stream, depth); break;
		case ExprToken::UnicodeStringConst: PushUnicodeZ(stream->ReadUnicodeZ()); break;
		case ExprToken::StructMember: PushIndex(stream->ReadIndex()); ReadToken(stream, depth); break;
		default: Exception::Throw("Unknown script bytecode token encountered");
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
		ParmsSize = stream->ReadUInt16();
	NativeFuncIndex = stream->ReadUInt16();
	if (stream->GetVersion() <= 63)
		NumParms = stream->ReadUInt8();
	OperatorPrecedence = stream->ReadUInt8();
	if (stream->GetVersion() <= 63)
		ReturnValueOffset = stream->ReadUInt16();
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

	for (UField* child = Children; child; child = child->Next)
	{
		if (UObject::TryCast<UFunction>(child))
		{
			Functions[child->Name] = static_cast<UFunction*>(child);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

UClass::UClass(NameString name, UClass* base, ObjectFlags flags) : UState(std::move(name), this, flags, base)
{
	if (base)
		ClsFlags = base->ClsFlags;
}

void UClass::Load(ObjectStream* stream)
{
	UState::Load(stream);

	if (stream->GetVersion() <= 61)
	{
		OldClassRecordSize = stream->ReadUInt32();
		Flags = Flags | ObjectFlags::Public | ObjectFlags::Standalone;
	}

	ClsFlags = (ClassFlags)stream->ReadUInt32();
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

	// Copy native UObject properties into the VM
	SetObject("Class", this);
	SetName("Name", Name);
	SetInt("ObjectFlags", (int)Flags);

	auto packages = stream->GetPackage()->GetPackageManager();
	PackageName = stream->GetPackage()->GetPackageName();
	NameString sectionName = PackageName.ToString() + "." + Name.ToString();
	NameString configName = ClassConfigName;
	if (configName.IsNone()) configName = "system";
	for (UProperty* prop : Properties)
	{
		bool isConfig = AllFlags(prop->PropFlags, PropertyFlags::Config) || (AllFlags(prop->PropFlags, PropertyFlags::GlobalConfig) && prop->Outer() == this);
		bool isLocalized = AllFlags(prop->PropFlags, PropertyFlags::Localized);
		if (isConfig || isLocalized)
		{
			void* ptr = PropertyData.Ptr(prop);
			for (int arrayIndex = 0; arrayIndex < prop->ArrayDimension; arrayIndex++)
			{
				NameString name = prop->Name;
				if (prop->ArrayDimension > 1)
					name = NameString(name.ToString() + "[" + std::to_string(arrayIndex) + "]");

				std::string value;
				if (isConfig)
				{
					value = packages->GetIniValue(configName, sectionName, name);
				}
				else if (isLocalized)
				{
					value = packages->Localize(PackageName, Name, name);
				}

				if (!value.empty())
				{
					if (dynamic_cast<UByteProperty*>(prop)) *static_cast<uint8_t*>(ptr) = (uint8_t)std::atoi(value.c_str());
					else if (dynamic_cast<UIntProperty*>(prop)) *static_cast<int32_t*>(ptr) = (int32_t)std::atoi(value.c_str());
					else if (dynamic_cast<UFloatProperty*>(prop)) *static_cast<float*>(ptr) = (float)std::atof(value.c_str());
					else if (dynamic_cast<UNameProperty*>(prop)) *static_cast<NameString*>(ptr) = value;
					else if (dynamic_cast<UStrProperty*>(prop)) *static_cast<std::string*>(ptr) = value;
					else if (dynamic_cast<UStringProperty*>(prop)) *static_cast<std::string*>(ptr) = value;
					else if (auto boolprop = dynamic_cast<UBoolProperty*>(prop))
					{
						for (char& c : value)
							if (c >= 'A' && c <= 'Z')
								c += 'a' - 'A';
						boolprop->SetBool(ptr, value == "1" || value == "true" || value == "yes");
					}
					else if (dynamic_cast<UClassProperty*>(prop))
					{
						try
						{
							size_t pos = value.find_first_of('.');
							if (pos != std::string::npos)
							{
								NameString packageName = value.substr(0, pos);
								NameString className = value.substr(pos + 1);
								Package* pkg = packages->GetPackage(packageName);
								*static_cast<UObject**>(ptr) = pkg->GetUObject("Class", className);
							}
						}
						catch (...)
						{
							// To do: is this actually a fatal error?
						}
					}
					else if (dynamic_cast<UStructProperty*>(prop))
					{
						// Yes, this is total spaghetti code at this point. No, I don't care anymore. ;)
						auto values = ParseStructValue(value);
						for (UProperty* member : static_cast<UStructProperty*>(prop)->Struct->Properties)
						{
							std::string membervalue = values[member->Name];
							void* memberptr = static_cast<uint8_t*>(ptr) + member->DataOffset.DataOffset;
							if (dynamic_cast<UByteProperty*>(member)) *static_cast<uint8_t*>(memberptr) = (uint8_t)std::atoi(membervalue.c_str());
							else if (dynamic_cast<UIntProperty*>(member)) *static_cast<int32_t*>(memberptr) = (int32_t)std::atoi(membervalue.c_str());
							else if (dynamic_cast<UFloatProperty*>(member)) *static_cast<float*>(memberptr) = (float)std::atof(membervalue.c_str());
							else if (dynamic_cast<UNameProperty*>(member)) *static_cast<NameString*>(memberptr) = membervalue;
							else if (dynamic_cast<UStrProperty*>(member)) *static_cast<std::string*>(memberptr) = membervalue;
							else if (dynamic_cast<UStringProperty*>(member)) *static_cast<std::string*>(memberptr) = membervalue;
							else if (auto boolprop = dynamic_cast<UBoolProperty*>(member))
							{
								for (char& c : membervalue)
									if (c >= 'A' && c <= 'Z')
										c += 'a' - 'A';
								boolprop->SetBool(memberptr, membervalue == "1" || membervalue == "true" || membervalue == "yes");
							}
							else if (dynamic_cast<UClassProperty*>(member))
							{
								try
								{
									size_t pos = membervalue.find_first_of('.');
									if (pos != std::string::npos)
									{
										NameString packageName = membervalue.substr(0, pos);
										NameString className = membervalue.substr(pos + 1);
										Package* pkg = packages->GetPackage(packageName);
										*static_cast<UObject**>(memberptr) = pkg->GetUObject("Class", className);
									}
								}
								catch (...)
								{
									// To do: is this actually a fatal error?
								}
							}
							else if (dynamic_cast<UObjectProperty*>(member))
							{
								// This happens for Deus Ex
							}
							else
							{
								Exception::Throw("localize keyword used on unsupported struct member property type");
							}
						}
					}
					else
					{
						Exception::Throw("localize keyword used on unsupported property type");
					}
				}

				ptr = static_cast<uint8_t*>(ptr) + prop->ElementSize();
			}
		}
	}

	for (UField* child = Children; child; child = child->Next)
	{
		if (UObject::TryCast<UState>(child))
		{
			States[child->Name] = static_cast<UState*>(child);
		}
	}
}

std::map<NameString, std::string> UClass::ParseStructValue(const std::string& text)
{
	// Parse one of the following:
	//
	// Object=(Name=Package.ObjectName,Class=ObjectClass,MetaClass=Package.MetaClassName,Description="descriptive string")
	// Preferences=(Caption="display name",Parent="display name of parent",Class=Package.ClassName,Category=variable group name,Immediate=True)

	if (text.size() < 2 || text.front() != '(' || text.back() != ')')
		return {};

	std::map<NameString, std::string> desc;

	// This would have been so much easier with a regular expression, but we can't use that as we have no idea what character set those .int files might be using
	size_t pos = 1;
	while (pos < text.size() - 1)
	{
		size_t endpos = text.find('=', pos);
		if (endpos == std::string::npos)
			break;
		NameString keyname = text.substr(pos, endpos - pos);
		pos = endpos + 1;

		if (text[pos] == '"')
		{
			pos++;
			endpos = text.find('"', pos);
			if (endpos == std::string::npos)
				break;

			std::string value = text.substr(pos, endpos - pos);
			desc[keyname] = value;
			pos++;

			pos = text.find(',', pos);
			if (pos == std::string::npos)
				break;
			pos++;
		}
		else
		{
			endpos = text.find_first_of(",)", pos);
			if (endpos == std::string::npos)
				break;
			std::string value = text.substr(pos, endpos - pos);
			desc[keyname] = value;
			pos = endpos + 1;
		}
	}

	return desc;
}

UProperty* UClass::GetProperty(const NameString& propName)
{
	for (UProperty* prop : PropertyData.Class->Properties)
	{
		if (prop->Name == propName)
			return prop;
	}
	Exception::Throw("Class Property '" + Name.ToString() + "." + propName.ToString() + "' not found");
}

void UClass::SaveToConfig(PackageManager& packageManager)
{
	if (!(ClsFlags & ClassFlags::Config))
		return;

	auto ini_file = packageManager.GetIniFile(ClassConfigName);

	// Iterate and save Properties that are marked with Config
	for (UProperty* prop : PropertyData.Class->Properties)
	{
		if ((uint32_t)(prop->PropFlags & PropertyFlags::Config))
		{
			ini_file->SetValue(this->Name, prop->Name, prop->PrintValue(PropertyData.Ptr(prop)));
		}
	}
}
