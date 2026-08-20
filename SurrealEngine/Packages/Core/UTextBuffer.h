#pragma once

#include "UObject.h"
#include "UStruct.h"

class StatementDebugInfo
{
public:
	StatementDebugInfo(ExprToken token, int line) : Token(token), Line(line) {}

	ExprToken Token = {};
	int Line = -1;
};

class FunctionDebugInfo
{
public:
	std::vector<StatementDebugInfo> Statements;
};

class ClassDebugInfo
{
public:
	NameString Name;
	std::map<NameString, FunctionDebugInfo> Functions;
};

class UTextBuffer : public UObject
{
public:
	using UObject::UObject;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	uint32_t Pos = 0;
	uint32_t Top = 0;
	std::string Text;
	std::unique_ptr<ClassDebugInfo> DebugInfo;
};
