#pragma once

#include "UStruct.h"

enum class FunctionFlags : uint32_t
{
	Final = 0x00000001,
	Defined = 0x00000002,
	Iterator = 0x00000004,
	Latent = 0x00000008,
	PreOperator = 0x00000010,
	Singular = 0x00000020,
	Net = 0x00000040,
	NetReliable = 0x00000080,
	Simulated = 0x00000100,
	Exec = 0x00000200,
	Native = 0x00000400,
	Event = 0x00000800,
	Operator = 0x00001000,
	Static = 0x00002000,
	NoExport = 0x00004000,
	Const = 0x00008000,
	Invariant = 0x00010000
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
	void Save(PackageStreamWriter* stream) override;

	int ParmsSize = 0;
	int NativeFuncIndex = 0;
	int NumParms = 0;
	uint8_t OperatorPrecedence = 0;
	int ReturnValueOffset = 0;
	FunctionFlags FuncFlags = {};
	uint16_t ReplicationOffset = 0;

	UStruct* NativeStruct = nullptr;
};
