#pragma once

#include "UObject/UClass.h"
#include "UObject/UProperty.h"
#include "Package/Package.h"
#include "Expression.h"

class BytecodeStream;

class Bytecode
{
public:
	Bytecode(const Array<uint8_t>& bytecode, Package* package);

	int FindStatementIndex(uint16_t offset) const
	{
		return OffsetToExpression.find(offset)->second->StatementIndex;
	}

	int FindLabelIndex(const NameString& label)
	{
		LabelTableExpression* labels = dynamic_cast<LabelTableExpression*>(Statements.back());
		if (labels)
		{
			for (LabelEntry& entry : labels->Labels)
			{
				if (entry.Name == label)
					return FindStatementIndex(entry.Offset);
			}
		}
		return -1;
	}

	Array<Expression*> Statements;

private:
	Expression* ReadToken(BytecodeStream* stream, int depth);

	template<typename T>
	T* Create(uint16_t offset)
	{
		Allocations.push_back(std::make_unique<T>());
		T* obj = static_cast<T*>(Allocations.back().get());
		OffsetToExpression[offset] = obj;
		return obj;
	}

	std::map<uint16_t, Expression*> OffsetToExpression;
	Array<std::unique_ptr<Expression>> Allocations;
};

class BytecodeStream
{
public:
	BytecodeStream(const void* d, size_t s, Package* package) : data((uint8_t*)d), size((uint32_t)s), package(package) { }

	void ReadBytes(void* d, uint32_t s)
	{
		if (pos + s > size)
			Exception::Throw("BytecodeStream::ReadBytes: Unexpected end of file in " + package->GetPackageName().ToString());
		memcpy(d, data + pos, s);
		pos += s;
	}

	int8_t ReadInt8() { int8_t t; ReadBytes(&t, 1); return t; }
	int16_t ReadInt16() { int16_t t; ReadBytes(&t, 2); return t; }
	int32_t ReadInt32() { int32_t t; ReadBytes(&t, 4); return t; }
	int64_t ReadInt64() { int64_t t; ReadBytes(&t, 8); return t; }
	float ReadFloat() { float t; ReadBytes(&t, 4); return t; }

	int32_t ReadIndex() { return ReadInt32(); }

	uint8_t ReadUInt8() { return ReadInt8(); }
	uint16_t ReadUInt16() { return ReadInt16(); }
	uint32_t ReadUInt32() { return ReadInt32(); }
	uint64_t ReadUInt64() { return ReadInt64(); }

	std::string ReadAsciiZ()
	{
		std::string s;
		while (true)
		{
			char c = ReadInt8();
			if (c == 0) break;
			s.push_back(c);
		}
		return s;
	}

	std::wstring ReadUnicodeZ()
	{
		std::wstring s;
		while (true)
		{
			wchar_t c = ReadUInt16();
			if (c == 0) break;
			s.push_back(c);
		}
		return s;
	}

	ExprToken ReadToken()
	{
		if (pos >= size)
			Exception::Throw("BytecodeStream::ReadToken: Unexpected end of file in " + package->GetPackageName().ToString());
		return (ExprToken)data[pos++];
	}

	ExprToken PeekToken()
	{
		if (pos >= size)
			Exception::Throw("BytecodeStream::PeekToken: Unexpected end of file in " + package->GetPackageName().ToString());
		return (ExprToken)data[pos];
	}

	NameString ReadName()
	{
		return package->GetName(ReadIndex());
	}

	template<typename T>
	T* ReadObject()
	{
		return UObject::Cast<T>(package->GetUObject(ReadIndex()));
	}

	uint16_t GetOffset() const { return pos; }
	bool IsEnd() const { return pos == size; }

	int GetVersion() const
	{
		return package->GetVersion();
	}

private:
	uint8_t* data = nullptr;
	uint32_t size = 0;
	uint32_t pos = 0;
	Package* package = nullptr;
};
