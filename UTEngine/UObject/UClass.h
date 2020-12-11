#pragma once

#include "UObject.h"

class UTextBuffer;
enum EExprToken;

class UField : public UObject
{
public:
	UField(ObjectStream* stream, bool isUClass = false);

	UField* BaseField = nullptr;
	UField* Next = nullptr;
};

class UConst : public UField
{
public:
	UConst(ObjectStream* stream);

	std::string Constant;
};

class UEnum : public UField
{
public:
	UEnum(ObjectStream* stream);

	std::vector<std::string> ElementNames;
};

enum class PropertyFlags : uint32_t
{
	Edit = 0x00000001, // Property is user - settable in the editor.
	Const = 0x00000002, // Actor's property always matches class's default actor property.
	Input = 0x00000004, // Variable is writable by the input system.
	ExportObject = 0x00000008, // Object can be exported with actor.
	OptionalParm = 0x00000010, // Optional parameter(if Param is set).
	Net = 0x00000020, // Property is relevant to network replication
	ConstRef = 0x00000040, // Reference to a constant object.
	Parm = 0x00000080, // Function / When call parameter
	OutParm = 0x00000100, // Value is copied out after function call.
	SkipParm = 0x00000200, // Property is a short - circuitable evaluation function parm.
	ReturnParm = 0x00000400, // Return value.
	CoerceParm = 0x00000800, // Coerce args into this function parameter
	Native = 0x00001000, // Property is native : C++ code is responsible for serializing it.
	Transient = 0x00002000, // Property is transient : shouldn't be saved, zerofilled at load time.
	Config = 0x00004000, // Property should be loaded / saved as permanent profile.
	Localized = 0x00008000, // Property should be loaded as localizable text
	Travel = 0x00010000, // Property travels across levels / servers.
	EditConst = 0x00020000, // Property is uneditable in the editor
	GlobalConfig = 0x00040000, // Load config from base class, not subclass.
	OnDemand = 0x00100000, // Object or dynamic array loaded on demand only.
	New = 0x00200000, // Automatically create inner object
	NeedCtorLink = 0x00400000 // Fields need construction / destruction
};

inline PropertyFlags operator|(PropertyFlags a, PropertyFlags b) { return (PropertyFlags)((uint32_t)a | (uint32_t)b); }
inline PropertyFlags operator&(PropertyFlags a, PropertyFlags b) { return (PropertyFlags)((uint32_t)a & (uint32_t)b); }
inline bool AllFlags(PropertyFlags value, PropertyFlags flags) { return (value & flags) == flags; }
inline bool AnyFlags(PropertyFlags value, PropertyFlags flags) { return (uint32_t)(value & flags) != 0; }

class UProperty : public UField
{
public:
	UProperty(ObjectStream* stream);

	uint32_t ArrayDimension = 0;
	PropertyFlags PropFlags = {};
	std::string Category;
	uint16_t ReplicationOffset = 0;
};

class UByteProperty : public UProperty
{
public:
	UByteProperty(ObjectStream* stream);

	int EnumType = 0; // 0 if it is a normal byte, other for a reference to the Enum type object
};

class UObjectProperty : public UProperty
{
public:
	UObjectProperty(ObjectStream* stream);

	int ObjectClass = 0;
};

class UFixedArrayProperty : public UProperty
{
public:
	UFixedArrayProperty(ObjectStream* stream);

	int Inner = 0;
};

class UArrayProperty : public UProperty
{
public:
	UArrayProperty(ObjectStream* stream);

	int Inner = 0;
};

class UMapProperty : public UProperty
{
public:
	UMapProperty(ObjectStream* stream);

	int Key = 0;
	int Value = 0;
};

class UClassProperty : public UObjectProperty
{
public:
	UClassProperty(ObjectStream* stream);

	int MetaClass = 0;
};

class UStructProperty : public UProperty
{
public:
	UStructProperty(ObjectStream* stream);

	int Struct = 0;
};

class UIntProperty : public UProperty
{
public:
	UIntProperty(ObjectStream* stream) : UProperty(stream) { }
};

class UBoolProperty : public UProperty
{
public:
	UBoolProperty(ObjectStream* stream) : UProperty(stream) { }
};

class UFloatProperty : public UProperty
{
public:
	UFloatProperty(ObjectStream* stream) : UProperty(stream) { }
};

class UNameProperty : public UProperty
{
public:
	UNameProperty(ObjectStream* stream) : UProperty(stream) { }
};

class UStrProperty : public UProperty
{
public:
	UStrProperty(ObjectStream* stream) : UProperty(stream) { }
};

class UStringProperty : public UProperty
{
public:
	UStringProperty(ObjectStream* stream) : UProperty(stream) { }
};

class UStruct : public UField
{
public:
	UStruct(ObjectStream* stream, bool isUClass = false);

	UTextBuffer* ScriptText = nullptr;
	UField* Children = nullptr;
	std::string FriendlyName;
	uint32_t Line = 0;
	uint32_t TextPos = 0;
	std::vector<uint8_t> Bytecode;

private:
	EExprToken ReadToken(ObjectStream* stream, int depth);
	void PushBytes(const void* data, size_t size);
	void PushUInt8(uint8_t value);
	void PushUInt16(uint16_t value);
	void PushUInt32(uint32_t value);
	void PushIndex(int32_t value);
	void PushFloat(float value);
	void PushAsciiZ(const std::string& value);
	void PushUnicodeZ(const std::wstring& value);
};

enum class FunctionFlags : uint32_t
{
	Final = 0x00000001, // Function is final(prebindable, non - overridable function)
	Defined = 0x00000002, // Function has been defined(not just declared)
	Iterator = 0x00000004, // Function is an iterator
	Latent = 0x00000008, // Function is a latent state function
	PreOperator = 0x00000010, // Unary operator is a prefix operator
	Singular = 0x00000020, // Function cannot be reentered
	Net = 0x00000040, // Function is network - replicated
	NetReliable = 0x00000080, // Function should be sent reliably on the network
	Simulated = 0x00000100, // Function executed on the client side
	Exec = 0x00000200, // Executable from command line
	Native = 0x00000400, // Native function
	Event = 0x00000800, // Event function
	Operator = 0x00001000, // Operator function
	Static = 0x00002000, // Static function
	NoExport = 0x00004000, // Don't export intrinsic function to C++
	Const = 0x00008000, // Function doesn't modify this object
	Invariant = 0x00010000 // Return value is purely dependent on parameters; no state dependencies or internal state changes
};

inline FunctionFlags operator|(FunctionFlags a, FunctionFlags b) { return (FunctionFlags)((uint32_t)a | (uint32_t)b); }
inline FunctionFlags operator&(FunctionFlags a, FunctionFlags b) { return (FunctionFlags)((uint32_t)a & (uint32_t)b); }
inline bool AllFlags(FunctionFlags value, FunctionFlags flags) { return (value & flags) == flags; }
inline bool AnyFlags(FunctionFlags value, FunctionFlags flags) { return (uint32_t)(value & flags) != 0; }

class UFunction : public UStruct
{
public:
	UFunction(ObjectStream* stream);

	int ParmsSize = 0;
	int NativeFuncIndex = 0;
	int NumParms = 0;
	uint8_t OperatorPrecedence = 0;
	int ReturnValueOffset = 0;
	FunctionFlags FuncFlags = {};
	uint16_t ReplicationOffset = 0;
};

enum class ScriptStateFlags : uint32_t
{
	Editable = 0x00000001, // State should be user - selectable in UnrealEd
	Auto = 0x00000002, // State is automatic(the default state)
	Simulated = 0x00000004 // State executes on client side
};

inline ScriptStateFlags operator|(ScriptStateFlags a, ScriptStateFlags b) { return (ScriptStateFlags)((uint32_t)a | (uint32_t)b); }
inline ScriptStateFlags operator&(ScriptStateFlags a, ScriptStateFlags b) { return (ScriptStateFlags)((uint32_t)a & (uint32_t)b); }
inline bool AllFlags(ScriptStateFlags value, ScriptStateFlags flags) { return (value & flags) == flags; }
inline bool AnyFlags(ScriptStateFlags value, ScriptStateFlags flags) { return (uint32_t)(value & flags) != 0; }

class UState : public UStruct
{
public:
	UState(ObjectStream* stream, bool isUClass = false);

	uint64_t ProbeMask = 0;
	uint64_t IgnoreMask = 0;
	uint16_t LabelTableOffset = 0;
	ScriptStateFlags StateFlags = {};
};

struct Guid
{
	uint8_t Data[16] = {};
};

struct ClassDependency
{
	int Class = 0;
	uint32_t Deep = 0;
	uint32_t ScriptTextCRC = 0;
};

class UClass : public UState
{
public:
	UClass(ObjectStream* stream);

	uint32_t OldClassRecordSize = 0;
	uint32_t ClassFlags = 0;
	Guid ClassGuid;
	std::vector<ClassDependency> Dependencies;
	std::vector<int> PackageImports;
	int ClassWithin = 0;
	std::string ClassConfigName;
};

enum EExprToken
{
	// Variable references
	EX_LocalVariable = 0x00,	// A local variable
	EX_InstanceVariable = 0x01,	// An object variable
	EX_DefaultVariable = 0x02,	// Default variable for a concrete object

	// Tokens
	EX_Return = 0x04,	// Return from function
	EX_Switch = 0x05,	// Switch
	EX_Jump = 0x06,	// Goto a local address in code
	EX_JumpIfNot = 0x07,	// Goto if not expression
	EX_Stop = 0x08,	// Stop executing state code
	EX_Assert = 0x09,	// Assertion
	EX_Case = 0x0A,	// Case
	EX_Nothing = 0x0B,	// No operation
	EX_LabelTable = 0x0C,	// Table of labels
	EX_GotoLabel = 0x0D,	// Goto a label
	EX_EatString = 0x0E, // Ignore a dynamic string
	EX_Let = 0x0F,	// Assign an arbitrary size value to a variable
	EX_DynArrayElement = 0x10, // Dynamic array element
	EX_New = 0x11, // New object allocation
	EX_ClassContext = 0x12, // Class default metaobject context
	EX_MetaCast = 0x13, // Metaclass cast
	EX_LetBool = 0x14, // Let boolean variable
	EX_Unknown0x15 = 0x15,
	EX_EndFunctionParms = 0x16,	// End of function call parameters
	EX_Self = 0x17,	// Self object
	EX_Skip = 0x18,	// Skippable expression
	EX_Context = 0x19,	// Call a function through an object context
	EX_ArrayElement = 0x1A,	// Array element
	EX_VirtualFunction = 0x1B,	// A function call with parameters
	EX_FinalFunction = 0x1C,	// A prebound function call with parameters
	EX_IntConst = 0x1D,	// Int constant
	EX_FloatConst = 0x1E,	// Floating point constant
	EX_StringConst = 0x1F,	// String constant
	EX_ObjectConst = 0x20,	// An object constant
	EX_NameConst = 0x21,	// A name constant
	EX_RotationConst = 0x22,	// A rotation constant
	EX_VectorConst = 0x23,	// A vector constant
	EX_ByteConst = 0x24,	// A byte constant
	EX_IntZero = 0x25,	// Zero
	EX_IntOne = 0x26,	// One
	EX_True = 0x27,	// Bool True
	EX_False = 0x28,	// Bool False
	EX_NativeParm = 0x29, // Native function parameter offset
	EX_NoObject = 0x2A,	// NoObject
	Ex_Unknown0x2b = 0x2B,
	EX_IntConstByte = 0x2C,	// Int constant that requires 1 byte
	EX_BoolVariable = 0x2D,	// A bool variable which requires a bitmask
	EX_DynamicCast = 0x2E,	// Safe dynamic class casting
	EX_Iterator = 0x2F, // Begin an iterator operation
	EX_IteratorPop = 0x30, // Pop an iterator level
	EX_IteratorNext = 0x31, // Go to next iteration
	EX_StructCmpEq = 0x32,	// Struct binary compare-for-equal
	EX_StructCmpNe = 0x33,	// Struct binary compare-for-unequal
	EX_UnicodeStringConst = 0x34, // Unicode string constant
	//
	EX_StructMember = 0x36, // Struct member
	//
	EX_GlobalFunction = 0x38, // Call non-state version of a function

	// Native conversions.
	EX_MinConversion = 0x39,	// Minimum conversion token
	EX_RotatorToVector = 0x39,
	EX_ByteToInt = 0x3A,
	EX_ByteToBool = 0x3B,
	EX_ByteToFloat = 0x3C,
	EX_IntToByte = 0x3D,
	EX_IntToBool = 0x3E,
	EX_IntToFloat = 0x3F,
	EX_BoolToByte = 0x40,
	EX_BoolToInt = 0x41,
	EX_BoolToFloat = 0x42,
	EX_FloatToByte = 0x43,
	EX_FloatToInt = 0x44,
	EX_FloatToBool = 0x45,
	//
	EX_ObjectToBool = 0x47,
	EX_NameToBool = 0x48,
	EX_StringToByte = 0x49,
	EX_StringToInt = 0x4A,
	EX_StringToBool = 0x4B,
	EX_StringToFloat = 0x4C,
	EX_StringToVector = 0x4D,
	EX_StringToRotator = 0x4E,
	EX_VectorToBool = 0x4F,
	EX_VectorToRotator = 0x50,
	EX_RotatorToBool = 0x51,
	EX_ByteToString = 0x52,
	EX_IntToString = 0x53,
	EX_BoolToString = 0x54,
	EX_FloatToString = 0x55,
	EX_ObjectToString = 0x56,
	EX_NameToString = 0x57,
	EX_VectorToString = 0x58,
	EX_RotatorToString = 0x59,
	EX_MaxConversion = 0x60,	// Maximum conversion token
	EX_ExtendedNative = 0x60,
	EX_FirstNative = 0x70,
	EX_Max = 0x1000,
};
