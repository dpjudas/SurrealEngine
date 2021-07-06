#pragma once

#include "ExpressionValue.h"

class UObject;

ExpressionValue CallEvent(UObject* Context, const std::string& name, std::vector<ExpressionValue> args = {});
