#pragma once

#include "ExpressionValue.h"

class UObject;

ExpressionValue CallEvent(UObject* Context, const NameString& name, std::vector<ExpressionValue> args = {});
UFunction* FindEventFunction(UObject* Context, const NameString& name);
