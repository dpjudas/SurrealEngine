#pragma once

#include "UObject.h"

class UField : public UObject
{
public:
	UField(ObjectStream* stream);

	int SuperField = 0;
	int Next = 0;
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

	uint16_t ArrayDimension = 0;
	uint16_t ElementSize = 0;
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

	int ObjectType = 0;
};

class UFixedArrayProperty : public UProperty
{
public:
	UFixedArrayProperty(ObjectStream* stream);

	int ElementType = 0;
	int Count = 0;
};

class UArrayProperty : public UProperty
{
public:
	UArrayProperty(ObjectStream* stream);

	int ElementType = 0;
};

class UMapProperty : public UProperty
{
public:
	UMapProperty(ObjectStream* stream);

	int Key = 0;
	int Value = 0;
};

class UClassProperty : public UProperty
{
public:
	UClassProperty(ObjectStream* stream);

	int Type = 0;
};

class UStructProperty : public UProperty
{
public:
	UStructProperty(ObjectStream* stream);

	int Type = 0;
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
	UStruct(ObjectStream* stream);

	int ScriptText = 0;
	int Children = 0;
	std::string FriendlyName;
	uint32_t Line = 0;
	uint32_t TextPos = 0;
	uint32_t BytecodeSize = 0;
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
	UState(ObjectStream* stream);

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
