#pragma once

#include "ExpressionValue.h"

class Iterator
{
public:
	virtual ~Iterator() = default;
	virtual bool Next() = 0;

	size_t StartStatementIndex = 0;
	size_t EndStatementIndex = 0;
};

class AllObjectsIterator : public Iterator
{
public:
	AllObjectsIterator(UObject* BaseClass, UObject** ReturnValue, std::string MatchTag);
	bool Next() override;

private:
	UObject* BaseClass = nullptr;
	UObject** ReturnValue = nullptr;
	std::string MatchTag;
	size_t index = 0;
};
