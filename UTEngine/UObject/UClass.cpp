
#include "Precomp.h"
#include "UClass.h"
#include "UTextBuffer.h"

UField::UField(ObjectStream* stream, bool isUClass) : UObject(stream, isUClass)
{
	if (stream->IsEmptyStream()) return;

	Name = stream->GetObjectName();
	Base = stream->GetObjectBase();

	BaseField = stream->ReadObject<UField>();
	Next = stream->ReadObject<UField>();
}

/////////////////////////////////////////////////////////////////////////////

UConst::UConst(ObjectStream* stream) : UField(stream)
{
	if (stream->IsEmptyStream()) return;

	Constant = stream->ReadString();

	stream->ThrowIfNotEnd();
}

/////////////////////////////////////////////////////////////////////////////

UEnum::UEnum(ObjectStream* stream) : UField(stream)
{
	if (stream->IsEmptyStream()) return;

	int size = stream->ReadIndex();
	for (int i = 0; i < size; i++)
		ElementNames.push_back(stream->ReadName());

	stream->ThrowIfNotEnd();
}

/////////////////////////////////////////////////////////////////////////////

UStruct::UStruct(ObjectStream* stream, bool isUClass) : UField(stream, isUClass)
{
	if (stream->IsEmptyStream()) return;

	ScriptText = stream->ReadObject<UTextBuffer>();
	Children = stream->ReadObject<UField>();
	FriendlyName = stream->ReadName();
	if (FriendlyName == "None")
		throw std::runtime_error("Struct FriendlyName must not be None");

	Line = stream->ReadUInt32();
	TextPos = stream->ReadUInt32();

	int ScriptSize = stream->ReadUInt32();
	while (Bytecode.size() < ScriptSize)
	{
		ReadToken(stream, 0);
	}
	if (Bytecode.size() != ScriptSize)
		throw std::runtime_error("Bytecode load failed");
}

ExprToken UStruct::ReadToken(ObjectStream* stream, int depth)
{
	if (depth == 16)
		throw std::runtime_error("Bytecode parsing error");
	depth++;

	ExprToken token = (ExprToken)stream->ReadUInt8();
	PushUInt8((uint8_t)token);

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

UFunction::UFunction(ObjectStream* stream) : UStruct(stream)
{
	if (stream->IsEmptyStream()) return;

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
	stream->ThrowIfNotEnd();
}

/////////////////////////////////////////////////////////////////////////////

UState::UState(ObjectStream* stream, bool isUClass) : UStruct(stream, isUClass)
{
	if (stream->IsEmptyStream()) return;

	ProbeMask = stream->ReadUInt64();
	IgnoreMask = stream->ReadUInt64();
	LabelTableOffset = stream->ReadUInt16();
	StateFlags = (ScriptStateFlags)stream->ReadUInt32();
}

/////////////////////////////////////////////////////////////////////////////

UClass::UClass(ObjectStream* stream) : UState(stream, true)
{
	if (stream->IsEmptyStream()) return;

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

	ReadProperties(stream);
	stream->ThrowIfNotEnd();
}
