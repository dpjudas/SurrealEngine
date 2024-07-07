
#include "Precomp.h"
#include "ExpressionEvaluator.h"
#include "Expression.h"
#include "Bytecode.h"
#include "Frame.h"
#include "NativeFunc.h"
#include "Engine.h"
#include "Package/PackageManager.h"

ExpressionEvalResult ExpressionEvaluator::Eval(Expression* expr, UObject* self, UObject* context, void* localVariables)
{
	auto oldExpr = Frame::StepExpression;
	Frame::StepExpression = expr;

	for (const Breakpoint& bp : Frame::Breakpoints)
	{
		if (bp.Expr == expr && bp.Enabled)
		{
			Frame::Break();
		}
	}

	ExpressionEvaluator evaluator;
	evaluator.Self = self;
	evaluator.Context = context;
	evaluator.LocalVariables = localVariables;
	expr->Visit(&evaluator);
	Frame::StepExpression = oldExpr;
	return std::move(evaluator.Result);
}

void ExpressionEvaluator::Expr(LocalVariableExpression* expr)
{
	Result.Value = ExpressionValue::Variable(LocalVariables, expr->Variable);
}

void ExpressionEvaluator::Expr(InstanceVariableExpression* expr)
{
	Result.Value = ExpressionValue::Variable(Context->PropertyData.Data, expr->Variable);
}

void ExpressionEvaluator::Expr(DefaultVariableExpression* expr)
{
	if (UObject::TryCast<UClass>(Context))
		Result.Value = ExpressionValue::Variable(Context->PropertyData.Data, expr->Variable);
	else
		Result.Value = ExpressionValue::Variable(Context->Class->GetDefaultObject<UObject>()->PropertyData.Data, expr->Variable);
}

void ExpressionEvaluator::Expr(ReturnExpression* expr)
{
	if (expr->Value)
		Result.Value = Eval(expr->Value).Value;
	else
		Result.Value = ExpressionValue::NothingValue();
	Result.Result = StatementResult::Return;
}

void ExpressionEvaluator::Expr(SwitchExpression* expr)
{
	Result.Value = Eval(expr->Condition).Value;
	Result.Result = StatementResult::Switch;
}

void ExpressionEvaluator::Expr(JumpExpression* expr)
{
	Result.Result = StatementResult::Jump;
	Result.JumpAddress = expr->Offset;
}

void ExpressionEvaluator::Expr(JumpIfNotExpression* expr)
{
	if (!Eval(expr->Condition).Value.ToBool())
	{
		Result.Result = StatementResult::Jump;
		Result.JumpAddress = expr->Offset;
	}
}

void ExpressionEvaluator::Expr(StopExpression* expr)
{
	Result.Result = StatementResult::Stop;
}

void ExpressionEvaluator::Expr(AssertExpression* expr)
{
	if (!Eval(expr->Condition).Value.ToBool())
	{
		Frame::ThrowException("Script assert failed for " + Self->Name.ToString() + " line " + std::to_string(expr->Line));
	}
}

void ExpressionEvaluator::Expr(CaseExpression* expr)
{
	Result.Value = ExpressionValue::NothingValue();
}

void ExpressionEvaluator::Expr(NothingExpression* expr)
{
	Result.Value = ExpressionValue::NothingValue();
}

void ExpressionEvaluator::Expr(LabelTableExpression* expr)
{
	Frame::ThrowException("Label table expression is not implemented");
}

void ExpressionEvaluator::Expr(GotoLabelExpression* expr)
{
	Result.Result = StatementResult::GotoLabel;
	Result.Label = Eval(expr->Value).Value.ToName();
}

void ExpressionEvaluator::Expr(EatStringExpression* expr)
{
	Eval(expr->Value);
	Result.Value = ExpressionValue::NothingValue();
}

void ExpressionEvaluator::Expr(LetExpression* expr)
{
	ExpressionValue lvalue = Eval(expr->LeftSide).Value;
	ExpressionValue rvalue = Eval(expr->RightSide).Value;
	lvalue.Store(rvalue);
	Result.Value = std::move(lvalue);
}

void ExpressionEvaluator::Expr(LetBoolExpression* expr)
{
	ExpressionValue lvalue = Eval(expr->LeftSide).Value;
	ExpressionValue rvalue = Eval(expr->RightSide).Value;
	lvalue.Store(rvalue);
	Result.Value = std::move(lvalue);
}

void ExpressionEvaluator::Expr(DynArrayElementExpression* expr)
{
	Frame::ThrowException("Dynamic array element expression is not implemented");
}

void ExpressionEvaluator::Expr(NewExpression* expr)
{
	ExpressionValue outer = Eval(expr->ParentExpr).Value;
	ExpressionValue name = Eval(expr->NameExpr).Value;
	ExpressionValue flags = Eval(expr->FlagsExpr).Value;
	UClass* cls = UObject::Cast<UClass>(Eval(expr->ClassExpr).Value.ToObject());

	// To do: package needs to be grabbed from outer, or the "transient package" if it is None, a virtual package for runtime objects
	Package* package = engine->packages->GetPackage("Engine");

	UObject* newObj = package->NewObject(
		name.GetType() == ExpressionValueType::Nothing ? NameString() : name.ToName(),
		cls,
		flags.GetType() == ExpressionValueType::Nothing ? ObjectFlags::NoFlags : (ObjectFlags)flags.ToInt(),
		true);

	if (outer.GetType() != ExpressionValueType::Nothing)
		newObj->Outer() = outer.ToObject();

	Result.Value = ExpressionValue::ObjectValue(newObj);
}

void ExpressionEvaluator::Expr(ClassContextExpression* expr)
{
	ExpressionValue object = Eval(expr->ObjectExpr).Value;
	UClass* cls = UObject::TryCast<UClass>(object.ToObject());
	if (cls)
	{
		Result = Eval(expr->ContextExpr, Self, cls->GetDefaultObject<UObject>(), LocalVariables);
	}
	else
	{
		Frame::ThrowException("Class reference is None");
	}
}

void ExpressionEvaluator::Expr(MetaCastExpression* expr)
{
	UObject* value = Eval(expr->Value).Value.ToObject();
	if (value && value != expr->Class)
	{
		UClass* cls = UObject::TryCast<UClass>(value);
		while (cls)
		{
			if (cls == expr->Class)
				break;
			cls = static_cast<UClass*>(cls->BaseStruct);
		}
		if (!cls)
			value = nullptr;
	}
	Result.Value = ExpressionValue::ObjectValue(value);
}

void ExpressionEvaluator::Expr(Unknown0x15Expression* expr)
{
	Frame::ThrowException("Unknown0x15 expression encountered");
}

void ExpressionEvaluator::Expr(SelfExpression* expr)
{
	Result.Value = ExpressionValue::ObjectValue(Self);
}

void ExpressionEvaluator::Expr(SkipExpression* expr)
{
	Result = Eval(expr->Value);
}

void ExpressionEvaluator::Expr(ContextExpression* expr)
{
	auto value = Eval(expr->ObjectExpr).Value;
	UObject* context = value.ToObject();
	if (context)
	{
		Result = Eval(expr->ContextExpr, Self, context, LocalVariables);
	}
	else
	{
		Result.Result = StatementResult::AccessedNone;
	}
}

void ExpressionEvaluator::Expr(ArrayElementExpression* expr)
{
	int index = Eval(expr->Index).Value.ToInt();
	auto arrayval = Eval(expr->Array).Value;
	if (arrayval.IsVariable())
	{
		Result.Value = arrayval.ItemAt(index);
	}
	else
	{
		Frame::ThrowException("Array is not a variable in ArrayElementExpression");
	}
}

void ExpressionEvaluator::Expr(IntConstExpression* expr)
{
	Result.Value = ExpressionValue::IntValue(expr->Value);
}

void ExpressionEvaluator::Expr(FloatConstExpression* expr)
{
	Result.Value = ExpressionValue::FloatValue(expr->Value);
}

void ExpressionEvaluator::Expr(StringConstExpression* expr)
{
	Result.Value = ExpressionValue::StringValue(expr->Value);
}

void ExpressionEvaluator::Expr(ObjectConstExpression* expr)
{
	Result.Value = ExpressionValue::ObjectValue(expr->Object);
}

void ExpressionEvaluator::Expr(NameConstExpression* expr)
{
	Result.Value = ExpressionValue::NameValue(expr->Value);
}

void ExpressionEvaluator::Expr(RotationConstExpression* expr)
{
	Result.Value = ExpressionValue::RotatorValue({ expr->Pitch, expr->Yaw, expr->Roll });
}

void ExpressionEvaluator::Expr(VectorConstExpression* expr)
{
	Result.Value = ExpressionValue::VectorValue({ expr->X, expr->Y, expr->Z });
}

void ExpressionEvaluator::Expr(ByteConstExpression* expr)
{
	Result.Value = ExpressionValue::ByteValue(expr->Value);
}

void ExpressionEvaluator::Expr(IntZeroExpression* expr)
{
	Result.Value = ExpressionValue::IntValue(0);
}

void ExpressionEvaluator::Expr(IntOneExpression* expr)
{
	Result.Value = ExpressionValue::IntValue(1);
}

void ExpressionEvaluator::Expr(TrueExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(true);
}

void ExpressionEvaluator::Expr(FalseExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(false);
}

void ExpressionEvaluator::Expr(NativeParmExpression* expr)
{
	Frame::ThrowException("Native parm expression is not implemented");
}

void ExpressionEvaluator::Expr(NoObjectExpression* expr)
{
	Result.Value = ExpressionValue::ObjectValue(nullptr);
}

void ExpressionEvaluator::Expr(Unknown0x2bExpression* expr)
{
	Result = Eval(expr->Value); // This may have been a truncating instruction from back when strings had a fixed size (package version 61 and earlier)
}

void ExpressionEvaluator::Expr(IntConstByteExpression* expr)
{
	Result.Value = ExpressionValue::ByteValue(expr->Value);
}

void ExpressionEvaluator::Expr(BoolVariableExpression* expr)
{
	Result.Value = Eval(expr->Variable).Value;
}

void ExpressionEvaluator::Expr(DynamicCastExpression* expr)
{
	UObject* value = Eval(expr->Value).Value.ToObject();
	if (value && !value->IsA(expr->Class->Name))
		value = nullptr;
	Result.Value = ExpressionValue::ObjectValue(value);
}

void ExpressionEvaluator::Expr(IteratorExpression* expr)
{
	Eval(expr->Value);
	Result.Result = StatementResult::Iterator;
	Result.Iter = std::move(Frame::CreatedIterator);
	Result.JumpAddress = expr->Offset;
}

void ExpressionEvaluator::Expr(IteratorPopExpression* expr)
{
	Result.Result = StatementResult::IteratorPop;
}

void ExpressionEvaluator::Expr(IteratorNextExpression* expr)
{
	Result.Result = StatementResult::IteratorNext;
}

void ExpressionEvaluator::Expr(StructCmpEqExpression* expr)
{
	ExpressionValue val1 = Eval(expr->Value1).Value;
	ExpressionValue val2 = Eval(expr->Value2).Value;
	Result.Value = ExpressionValue::BoolValue(val1.IsEqual(val2));
}

void ExpressionEvaluator::Expr(StructCmpNeExpression* expr)
{
	ExpressionValue val1 = Eval(expr->Value1).Value;
	ExpressionValue val2 = Eval(expr->Value2).Value;
	Result.Value = ExpressionValue::BoolValue(!val1.IsEqual(val2));
}

void ExpressionEvaluator::Expr(StructMemberExpression* expr)
{
	if (expr->Field)
		Result.Value = Eval(expr->Value).Value.Member(expr->Field);
	else
		Frame::ThrowException("Null field encountered in struct member expression");
}

void ExpressionEvaluator::Expr(UnicodeStringConstExpression* expr)
{
	std::string s;
	s.reserve(expr->Value.size());
	for (wchar_t c : expr->Value)
		s.push_back(c < 128 ? c : '?');
	Result.Value = ExpressionValue::StringValue(s);
}

void ExpressionEvaluator::Expr(RotatorToVectorExpression* expr)
{
	Rotator rot = Eval(expr->Value).Value.ToRotator();
	Result.Value = ExpressionValue::VectorValue(Coords::Rotation(rot).XAxis);
}

void ExpressionEvaluator::Expr(ByteToIntExpression* expr)
{
	Result.Value = ExpressionValue::IntValue(Eval(expr->Value).Value.ToByte());
}

void ExpressionEvaluator::Expr(ByteToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(Eval(expr->Value).Value.ToByte() != 0);
}

void ExpressionEvaluator::Expr(ByteToFloatExpression* expr)
{
	Result.Value = ExpressionValue::FloatValue(Eval(expr->Value).Value.ToByte());
}

void ExpressionEvaluator::Expr(IntToByteExpression* expr)
{
	Result.Value = ExpressionValue::ByteValue(Eval(expr->Value).Value.ToInt());
}

void ExpressionEvaluator::Expr(IntToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(Eval(expr->Value).Value.ToInt());
}

void ExpressionEvaluator::Expr(IntToFloatExpression* expr)
{
	Result.Value = ExpressionValue::FloatValue((float)Eval(expr->Value).Value.ToInt());
}

void ExpressionEvaluator::Expr(BoolToByteExpression* expr)
{
	Result.Value = ExpressionValue::ByteValue(Eval(expr->Value).Value.ToBool());
}

void ExpressionEvaluator::Expr(BoolToIntExpression* expr)
{
	Result.Value = ExpressionValue::IntValue(Eval(expr->Value).Value.ToBool());
}

void ExpressionEvaluator::Expr(BoolToFloatExpression* expr)
{
	Result.Value = ExpressionValue::FloatValue(Eval(expr->Value).Value.ToBool());
}

void ExpressionEvaluator::Expr(FloatToByteExpression* expr)
{
	Result.Value = ExpressionValue::ByteValue((int)Eval(expr->Value).Value.ToFloat());
}

void ExpressionEvaluator::Expr(FloatToIntExpression* expr)
{
	Result.Value = ExpressionValue::IntValue((int)Eval(expr->Value).Value.ToFloat());
}

void ExpressionEvaluator::Expr(FloatToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue((bool)Eval(expr->Value).Value.ToFloat());
}

void ExpressionEvaluator::Expr(Unknown0x46Expression* expr)
{
	Frame::ThrowException("Unknown0x46 expression encountered");
}

void ExpressionEvaluator::Expr(ObjectToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(Eval(expr->Value).Value.ToObject() != nullptr);
}

void ExpressionEvaluator::Expr(NameToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(Eval(expr->Value).Value.ToName() != "None");
}

void ExpressionEvaluator::Expr(StringToByteExpression* expr)
{
	Result.Value = ExpressionValue::ByteValue(std::atoi(Eval(expr->Value).Value.ToString().c_str()));
}

void ExpressionEvaluator::Expr(StringToIntExpression* expr)
{
	Result.Value = ExpressionValue::IntValue(std::atoi(Eval(expr->Value).Value.ToString().c_str()));
}

void ExpressionEvaluator::Expr(StringToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(std::atoi(Eval(expr->Value).Value.ToString().c_str()));
}

void ExpressionEvaluator::Expr(StringToFloatExpression* expr)
{
	Result.Value = ExpressionValue::FloatValue((float)std::atof(Eval(expr->Value).Value.ToString().c_str()));
}

void ExpressionEvaluator::Expr(StringToVectorExpression* expr)
{
	std::string v = Eval(expr->Value).Value.ToString();
	auto pos1 = v.find_first_of(',');
	auto pos2 = v.find_first_of(',', pos1 + 1);
	if (pos1 != std::string::npos && pos2 != std::string::npos)
	{
		Result.Value = ExpressionValue::VectorValue({ (float)std::atof(v.substr(0, pos1).c_str()), (float)std::atof(v.substr(pos1 + 1, pos2 - pos1 - 1).c_str()), (float)std::atof(v.substr(pos2 + 1).c_str()) });
	}
	else
	{
		Result.Value = ExpressionValue::VectorValue({ 0.0f });
	}
}

void ExpressionEvaluator::Expr(StringToRotatorExpression* expr)
{
	std::string v = Eval(expr->Value).Value.ToString();
	auto pos1 = v.find_first_of(',');
	auto pos2 = v.find_first_of(',', pos1 + 1);
	if (pos1 != std::string::npos && pos2 != std::string::npos)
	{
		Result.Value = ExpressionValue::RotatorValue({ std::atoi(v.substr(0, pos1).c_str()), std::atoi(v.substr(pos1 + 1, pos2 - pos1 - 1).c_str()), std::atoi(v.substr(pos2 + 1).c_str()) });
	}
	else
	{
		Result.Value = ExpressionValue::RotatorValue({ 0, 0, 0 });
	}
}

void ExpressionEvaluator::Expr(VectorToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(Eval(expr->Value).Value.ToVector() != vec3(0.0f));
}

void ExpressionEvaluator::Expr(VectorToRotatorExpression* expr)
{
	Result.Value = ExpressionValue::RotatorValue(Rotator::FromVector(Eval(expr->Value).Value.ToVector()));
}

void ExpressionEvaluator::Expr(RotatorToBoolExpression* expr)
{
	Result.Value = ExpressionValue::BoolValue(Eval(expr->Value).Value.ToRotator() != Rotator(0, 0, 0));
}

void ExpressionEvaluator::Expr(ByteToStringExpression* expr)
{
	Result.Value = ExpressionValue::StringValue(std::to_string(Eval(expr->Value).Value.ToByte()));
}

void ExpressionEvaluator::Expr(IntToStringExpression* expr)
{
	Result.Value = ExpressionValue::StringValue(std::to_string(Eval(expr->Value).Value.ToInt()));
}

void ExpressionEvaluator::Expr(BoolToStringExpression* expr)
{
	Result.Value = ExpressionValue::StringValue(std::to_string(Eval(expr->Value).Value.ToBool()));
}

void ExpressionEvaluator::Expr(FloatToStringExpression* expr)
{
	Result.Value = ExpressionValue::StringValue(std::to_string(Eval(expr->Value).Value.ToFloat()));
}

void ExpressionEvaluator::Expr(ObjectToStringExpression* expr)
{
	UObject* obj = Eval(expr->Value).Value.ToObject();
	Result.Value = ExpressionValue::StringValue(obj ? obj->Class->Name.ToString() + "/" + obj->Name.ToString() : "None");
}

void ExpressionEvaluator::Expr(NameToStringExpression* expr)
{
	Result.Value = ExpressionValue::StringValue(Eval(expr->Value).Value.ToName().ToString());
}

void ExpressionEvaluator::Expr(VectorToStringExpression* expr)
{
	vec3 v = Eval(expr->Value).Value.ToVector();
	Result.Value = ExpressionValue::StringValue(std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z));
}

void ExpressionEvaluator::Expr(RotatorToStringExpression* expr)
{
	Rotator v = Eval(expr->Value).Value.ToRotator();
	Result.Value = ExpressionValue::StringValue(std::to_string(v.Pitch & 0xffff) + "," + std::to_string(v.Yaw & 0xffff) + "," + std::to_string(v.Roll & 0xffff));
}

void ExpressionEvaluator::Expr(VirtualFunctionExpression* expr)
{
	UClass* contextClass = UObject::TryCast<UClass>(Context);
	if (!contextClass)
		contextClass = Context->Class;

	// Search states first

	NameString stateName = Context->GetStateName();
	for (UClass* cls = contextClass; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
	{
		UState* state = cls->GetState(stateName);
		if (state)
		{
			UFunction* func = state->GetFunction(expr->Name);
			if (func)
			{
				Call(func, expr->Args);
				return;
			}
		}
	}

	// Search normal member functions next

	for (UClass* cls = contextClass; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
	{
		for (UField* field = cls->Children; field != nullptr; field = field->Next)
		{
			UFunction* func = UObject::TryCast<UFunction>(field);
			if (func && func->Name == expr->Name)
			{
				Call(func, expr->Args);
				return;
			}
		}
	}

	Frame::ThrowException("Script virtual function " + expr->Name.ToString() + " not found!");
}

void ExpressionEvaluator::Expr(FinalFunctionExpression* expr)
{
	Call(expr->Func, expr->Args);
}

void ExpressionEvaluator::Expr(GlobalFunctionExpression* expr)
{
	// Global function calls skip the states and only searches normal member functions

	UClass* contextClass = UObject::TryCast<UClass>(Context);
	if (!contextClass)
		contextClass = Context->Class;

	for (UClass* cls = contextClass; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
	{
		UFunction* func = cls->GetFunction(expr->Name);
		if (func)
		{
			Call(func, expr->Args);
			return;
		}
	}

	Frame::ThrowException("Script global function " + expr->Name.ToString() + " not found!");
}

void ExpressionEvaluator::Expr(NativeFunctionExpression* expr)
{
	Call(NativeFunctions::FuncByIndex[expr->nativeindex], expr->Args);
}

void ExpressionEvaluator::Call(UFunction* func, const Array<Expression*>& exprArgs)
{
	if (func->NativeFuncIndex == 130)
	{
		Result.Value = ExpressionValue::BoolValue(Eval(exprArgs[0], Self, Self, LocalVariables).Value.ToBool() && Eval(exprArgs[1], Self, Self, LocalVariables).Value.ToBool());
	}
	else if (func->NativeFuncIndex == 132)
	{
		Result.Value = ExpressionValue::BoolValue(Eval(exprArgs[0], Self, Self, LocalVariables).Value.ToBool() || Eval(exprArgs[1], Self, Self, LocalVariables).Value.ToBool());
	}
	else
	{
		Array<ExpressionValue> args;
		args.reserve(exprArgs.size());
		for (Expression* arg : exprArgs)
			args.push_back(Eval(arg, Self, Self, LocalVariables).Value);
		Result.Value = Frame::Call(func, Context, std::move(args));
	}
}

void ExpressionEvaluator::Expr(FunctionArgumentsExpression* expr)
{
	Result.Value = ExpressionValue::NothingValue();
}
