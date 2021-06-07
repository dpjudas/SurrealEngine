#pragma once

#include "UObject.h"

class UTextBuffer;
class UStruct;
class UProperty;
enum class ExprToken : uint8_t;
class Bytecode;

class UField : public UObject
{
public:
	using UObject::UObject;
	void Load(ObjectStream* stream) override;

	UField* BaseField = nullptr;
	UField* Next = nullptr;
};

class UConst : public UField
{
public:
	using UField::UField;
	void Load(ObjectStream* stream) override;

	std::string Constant;
};

class UEnum : public UField
{
public:
	using UField::UField;
	void Load(ObjectStream* stream) override;

	std::vector<std::string> ElementNames;
};

class UStruct : public UField
{
public:
	using UField::UField;
	void Load(ObjectStream* stream) override;

	UTextBuffer* ScriptText = nullptr;
	UField* Children = nullptr;
	std::string FriendlyName;
	uint32_t Line = 0;
	uint32_t TextPos = 0;
#ifdef _DEBUG
	std::string BytecodePlainText;
#endif
	std::vector<uint8_t> Bytecode;
	std::shared_ptr<::Bytecode> Code;

	size_t StructSize = 0;
	std::map<std::string, UProperty*> Properties;

private:
	ExprToken ReadToken(ObjectStream* stream, int depth);
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
	using UStruct::UStruct;
	void Load(ObjectStream* stream) override;

	int ParmsSize = 0;
	int NativeFuncIndex = 0;
	int NumParms = 0;
	uint8_t OperatorPrecedence = 0;
	int ReturnValueOffset = 0;
	FunctionFlags FuncFlags = {};
	uint16_t ReplicationOffset = 0;

	UStruct* NativeStruct = nullptr;
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
	using UStruct::UStruct;
	void Load(ObjectStream* stream) override;

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
	UClass* Class = nullptr;
	uint32_t Deep = 0;
	uint32_t ScriptTextCRC = 0;
};

class UClass : public UState
{
public:
	using UState::UState;
	void Load(ObjectStream* stream) override;

	UProperty* GetProperty(const std::string& name);
	UObject* GetDefaultObject() { return this; }

	uint32_t OldClassRecordSize = 0;
	uint32_t ClassFlags = 0;
	Guid ClassGuid;
	std::vector<ClassDependency> Dependencies;
	std::vector<int> PackageImports;
	int ClassWithin = 0;
	std::string ClassConfigName;
};

enum class ExprToken : uint8_t
{
	// Variable references
	LocalVariable = 0x00,	// A local variable
	InstanceVariable = 0x01,	// An object variable
	DefaultVariable = 0x02,	// Default variable for a concrete object

	// Tokens
	Return = 0x04,	// Return from function
	Switch = 0x05,	// Switch
	Jump = 0x06,	// Goto a local address in code
	JumpIfNot = 0x07,	// Goto if not expression
	Stop = 0x08,	// Stop executing state code
	Assert = 0x09,	// Assertion
	Case = 0x0A,	// Case
	Nothing = 0x0B,	// No operation
	LabelTable = 0x0C,	// Table of labels
	GotoLabel = 0x0D,	// Goto a label
	EatString = 0x0E, // Ignore a dynamic string
	Let = 0x0F,	// Assign an arbitrary size value to a variable
	DynArrayElement = 0x10, // Dynamic array element
	New = 0x11, // New object allocation
	ClassContext = 0x12, // Class default metaobject context
	MetaCast = 0x13, // Metaclass cast
	LetBool = 0x14, // Let boolean variable
	Unknown0x15 = 0x15,
	EndFunctionParms = 0x16,	// End of function call parameters
	Self = 0x17,	// Self object
	Skip = 0x18,	// Skippable expression
	Context = 0x19,	// Call a function through an object context
	ArrayElement = 0x1A,	// Array element
	VirtualFunction = 0x1B,	// A function call with parameters
	FinalFunction = 0x1C,	// A prebound function call with parameters
	IntConst = 0x1D,	// Int constant
	FloatConst = 0x1E,	// Floating point constant
	StringConst = 0x1F,	// String constant
	ObjectConst = 0x20,	// An object constant
	NameConst = 0x21,	// A name constant
	RotationConst = 0x22,	// A rotation constant
	VectorConst = 0x23,	// A vector constant
	ByteConst = 0x24,	// A byte constant
	IntZero = 0x25,	// Zero
	IntOne = 0x26,	// One
	True = 0x27,	// Bool True
	False = 0x28,	// Bool False
	NativeParm = 0x29, // Native function parameter offset
	NoObject = 0x2A,	// NoObject
	Unknown0x2b = 0x2B,
	IntConstByte = 0x2C,	// Int constant that requires 1 byte
	BoolVariable = 0x2D,	// A bool variable which requires a bitmask
	DynamicCast = 0x2E,	// Safe dynamic class casting
	Iterator = 0x2F, // Begin an iterator operation
	IteratorPop = 0x30, // Pop an iterator level
	IteratorNext = 0x31, // Go to next iteration
	StructCmpEq = 0x32,	// Struct binary compare-for-equal
	StructCmpNe = 0x33,	// Struct binary compare-for-unequal
	UnicodeStringConst = 0x34, // Unicode string constant
	//
	StructMember = 0x36, // Struct member
	//
	GlobalFunction = 0x38, // Call non-state version of a function

	// Native conversions.
	MinConversion = 0x39,	// Minimum conversion token
	RotatorToVector = 0x39,
	ByteToInt = 0x3A,
	ByteToBool = 0x3B,
	ByteToFloat = 0x3C,
	IntToByte = 0x3D,
	IntToBool = 0x3E,
	IntToFloat = 0x3F,
	BoolToByte = 0x40,
	BoolToInt = 0x41,
	BoolToFloat = 0x42,
	FloatToByte = 0x43,
	FloatToInt = 0x44,
	FloatToBool = 0x45,
	Unknown0x46 = 0x46,
	ObjectToBool = 0x47,
	NameToBool = 0x48,
	StringToByte = 0x49,
	StringToInt = 0x4A,
	StringToBool = 0x4B,
	StringToFloat = 0x4C,
	StringToVector = 0x4D,
	StringToRotator = 0x4E,
	VectorToBool = 0x4F,
	VectorToRotator = 0x50,
	RotatorToBool = 0x51,
	ByteToString = 0x52,
	IntToString = 0x53,
	BoolToString = 0x54,
	FloatToString = 0x55,
	ObjectToString = 0x56,
	NameToString = 0x57,
	VectorToString = 0x58,
	RotatorToString = 0x59,
	MaxConversion = 0x60,	// Maximum conversion token
	ExtendedNative = 0x60,
	FirstNative = 0x70
};

inline bool operator<(ExprToken a, ExprToken b) { return (uint8_t)a < (uint8_t)b; }
inline bool operator<=(ExprToken a, ExprToken b) { return (uint8_t)a <= (uint8_t)b; }
inline bool operator>(ExprToken a, ExprToken b) { return (uint8_t)a > (uint8_t)b; }
inline bool operator>=(ExprToken a, ExprToken b) { return (uint8_t)a >= (uint8_t)b; }
