
#include "Precomp.h"
#include "UStruct.h"
#include "Engine.h"
#include "VM/NativeFunc.h"
#include "VM/Bytecode.h"
#include "Packages/Core/UFunction.h"
#include "Packages/Core/UTextBuffer.h"
#include "Packages/Core/Properties/UBoolProperty.h"
#include "Compiler/Frontend/Compiler.h"

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
	Children = stream->ReadObject<UField>();
	FriendlyName = stream->ReadName();
	if (FriendlyName == "None")
		Exception::Throw("Struct FriendlyName must not be None");

	Line = stream->ReadUInt32();
	TextPos = stream->ReadUInt32();

	int ScriptSize = stream->ReadUInt32();
	uint32_t rawStart = stream->Tell();
	while (Bytecode.size() < ScriptSize)
	{
		ReadToken(stream, 0);
	}

	if (Bytecode.size() != ScriptSize)
	{
		// Not sure what is up with this. Change it to a warning for now for 227
		if (engine->LaunchInfo.IsUnreal1_227())
			LogMessage("Unexpected bytecode for function " + Name.ToString() + " - expected = " + std::to_string(ScriptSize) + ", got = " + std::to_string(Bytecode.size()));
		else
			Exception::Throw("Bytecode load failed");
	}
	uint32_t rawEnd = stream->Tell();

	// Keep a copy of the original stream as we can't write it yet for the save function
	BytecodeRaw.resize(rawEnd - rawStart);
	stream->Seek(rawStart);
	stream->ReadBytes(BytecodeRaw.data(), (uint32_t)BytecodeRaw.size());

	Code = std::make_shared<::Bytecode>(Bytecode, stream->GetPackage());

	Array<UProperty*> properties;

	if (!BaseStruct && BaseField)
		BaseStruct = UObject::Cast<UStruct>(BaseField);

	size_t offset = 0;
	size_t structAlignment = 1;
	if (BaseStruct && !UObject::TryCast<UFunction>(this)) // functions should not inherit properties from the super/base function
	{
		BaseStruct->LoadNow();
		properties = BaseStruct->Properties;
		offset = BaseStruct->StructSize;
		structAlignment = std::max(structAlignment, BaseStruct->StructAlignment);
	}

	uint64_t bitfieldMask = 1;

	UField* child = Children;
	while (child)
	{
		child->LoadNow();

		if (UProperty* prop = UObject::TryCast<UBoolProperty>(child))
		{
			if (prop->ArrayDimension > 1)
				throw std::runtime_error("Bool properties with ArrayDimension larger than 1 not supported");

			// Pack bool properties into 32 bit bitfields
			properties.push_back(prop);
			if (bitfieldMask == 1 || bitfieldMask == (1ULL << 32))
			{
				size_t alignment = prop->ElementAlignment();
				size_t size = prop->ElementSize();
				prop->DataOffset.DataOffset = (offset + alignment - 1) / alignment * alignment;
				prop->DataOffset.BitfieldMask = 1;
				bitfieldMask = 2;
				offset = prop->DataOffset.DataOffset + size;
				structAlignment = std::max(structAlignment, alignment);
			}
			else
			{
				prop->DataOffset.DataOffset = offset - prop->ElementSize();
				prop->DataOffset.BitfieldMask = (uint32_t)bitfieldMask;
				bitfieldMask <<= 1;
			}
		}
		else if (UProperty* prop = UObject::TryCast<UProperty>(child))
		{
			properties.push_back(prop);
			bitfieldMask = 1;

			size_t alignment = prop->ArrayAlignment();
			size_t size = prop->ArraySize();
			prop->DataOffset.DataOffset = (offset + alignment - 1) / alignment * alignment;
			offset = prop->DataOffset.DataOffset + size;
			structAlignment = std::max(structAlignment, alignment);
		}
		else if (UStruct* childstruct = UObject::TryCast<UStruct>(child))
		{
			bitfieldMask = 1;
			childstruct->StructParent = this;
		}

		child = child->Next;
	}

	if (Name == "Object")
	{
		// We already initialized Object with properties in the UClass constructor.
		// Verify that the Core.u description of Object matches what we used:
		if (Properties.size() != properties.size() || StructAlignment != structAlignment || StructSize != offset)
		{
			throw std::runtime_error("UObject unexpected size!");
		}
	}

	Properties = std::move(properties);
	StructAlignment = structAlignment;
	StructSize = offset;

	child = Children;
	while (child)
	{
		UFunction* func = UObject::TryCast<UFunction>(child);
		if (func && AllFlags(func->FuncFlags, FunctionFlags::Native))
		{
			func->NativeStruct = this;
			NativeFunctions::RegisterNativeFunc(func);
		}
		child = child->Next;
	}

	if (ScriptText)
		ScriptText->LoadNow();
}

void UStruct::Save(PackageStreamWriter* stream)
{
	UField::Save(stream);

	stream->WriteObject(ScriptText);
	stream->WriteObject(Children);
	stream->WriteName(FriendlyName);
	stream->WriteUInt32(Line);
	stream->WriteUInt32(TextPos);

	stream->WriteUInt32((uint32_t)Bytecode.size());
	stream->WriteBytes(BytecodeRaw.data(), (uint32_t)BytecodeRaw.size());
	//size_t pos = 0;
	//while (pos < Bytecode.size())
	//	pos = WriteToken(stream, pos, 0);
}

bool UStruct::IsEqual(const void* v1, const void* v2)
{
	for (UProperty* prop : Properties)
	{
		if (!prop->CompareArray(
			static_cast<const uint8_t*>(v1) + prop->DataOffset.DataOffset,
			static_cast<const uint8_t*>(v2) + prop->DataOffset.DataOffset))
		{
			return false;
		}
	}
	return true;
}

int UStruct::GetStatementLine(Expression* statement)
{
	if (!Code)
		return -1;

	size_t index = 0;
	for (Expression* expr : Code->Statements)
	{
		if (expr == statement)
			break;
		index++;
	}

	FunctionDebugInfo* debugInfo = GetDebugInfo();
	if (debugInfo && index < debugInfo->Statements.size())
		return debugInfo->Statements[index].Line;
	return -1;
}

FunctionDebugInfo* UStruct::GetDebugInfo()
{
	UTextBuffer* scriptText = nullptr;
	UStruct* func = this;
	do
	{
		scriptText = func->ScriptText;
		func = func->StructParent;
	} while (func && !scriptText);
	if (!scriptText)
		return nullptr;

	if (!scriptText->DebugInfo)
	{
		Compiler compiler;
		compiler.add_code(scriptText->Text, scriptText->Name.ToString());
		if (compiler.compile())
		{
			scriptText->DebugInfo = compiler.move_debug_info(0);
			if (!scriptText->DebugInfo)
				LogMessage("Compiler did not generate any debug info!");
		}
		else
		{
			LogMessage("Could not generate debug info due to compile errors:");
			for (const auto& msg : compiler.get_messages())
			{
				LogMessage(msg.to_string());
			}
		}
		if (!scriptText->DebugInfo)
			scriptText->DebugInfo = std::make_unique<ClassDebugInfo>();

		// To do: validate compiled statements matches code statements
	}

	auto it = scriptText->DebugInfo->Functions.find(Name);
	if (it != scriptText->DebugInfo->Functions.end())
		return &it->second;
	return nullptr;
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
	"IteratorPop", "IteratorNext", "StructCmpEq", "StructCmpNe", "UnicodeStringConst", "0x35", "StructMember", "Construct",
	"GlobalFunction", "RotatorToVector", "ByteToInt", "ByteToBool", "ByteToFloat", "IntToByte", "IntToBool", "IntToFloat",
	"BoolToByte", "BoolToInt", "BoolToFloat", "FloatToByte", "FloatToInt", "FloatToBool", "Unknown0x46", "ObjectToBool",
	"NameToBool", "StringToByte", "StringToInt", "StringToBool", "StringToFloat", "StringToVector", "StringToRotator", "VectorToBool",
	"VectorToRotator", "RotatorToBool", "ByteToString", "IntToString", "BoolToString", "FloatToString", "ObjectToString", "NameToString",
	"VectorToString", "RotatorToString", "StringToName", "0x5b", "0x5c", "0x5d", "0x5e", "0x5f",
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
	else if (token == ExprToken::Construct_227 && engine->LaunchInfo.IsUnreal1_227())
	{
		int type = stream->ReadIndex();
		PushIndex(type);
		int argcount = stream->ReadIndex();
		PushIndex(argcount);
		for (int i = 0; i < argcount; i++)
		{
			int argName = stream->ReadIndex();
			PushIndex(argName);
			ReadToken(stream, depth);
		}
	}
	else if (token == ExprToken::DynArrayToInt_HP1 && engine->LaunchInfo.IsHarryPotter1())
	{
		ReadToken(stream, depth);
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
