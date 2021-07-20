
#include "Precomp.h"
#include "ExpressionItemBuilder.h"
#include "VM/Expression.h"
#include "VM/NativeFunc.h"
#include "VM/Frame.h"
#include "VM/Bytecode.h"
#include "UI/Controls/ListView/ListViewItem.h"
#include "UObject/UProperty.h"

std::unique_ptr<TextListViewItem> ExpressionItemBuilder::createItem(const std::string& name, Expression* expr)
{
	ExpressionItemBuilder builder;
	builder.item = std::make_unique<TextListViewItem>();
	builder.item->setText(1, name);
	if (expr)
		expr->Visit(&builder);
	else
		builder.item->setText(0, "Null");

	if (expr == Frame::StepExpression)
		builder.item->setText(0, "-->" + builder.item->getText(0)); // To do: use an arrow icon for this

	return std::move(builder.item);
}

void ExpressionItemBuilder::Expr(LocalVariableExpression* expr)
{
	item->setText(0, "Local variable");
	if (expr->Variable)
		item->setText(2, expr->Variable->Name);
	else
		item->setText(2, "(null)");
}

void ExpressionItemBuilder::Expr(InstanceVariableExpression* expr)
{
	item->setText(0, "Instance variable");
	if (expr->Variable)
		item->setText(2, expr->Variable->Name);
	else
		item->setText(2, "(null)");
}

void ExpressionItemBuilder::Expr(DefaultVariableExpression* expr)
{
	item->setText(0, "Default variable");
	if (expr->Variable)
		item->setText(2, expr->Variable->Name);
	else
		item->setText(2, "(null)");
}

void ExpressionItemBuilder::Expr(ReturnExpression* expr)
{
	item->setText(0, "Return");
}

void ExpressionItemBuilder::Expr(SwitchExpression* expr)
{
	item->setText(0, "Switch");
	item->add(createItem("Condition", expr->Condition));
}

void ExpressionItemBuilder::Expr(JumpExpression* expr)
{
	item->setText(0, "Jump");
}

void ExpressionItemBuilder::Expr(JumpIfNotExpression* expr)
{
	item->setText(0, "Jump if not");
	item->add(createItem("Condition", expr->Condition));
}

void ExpressionItemBuilder::Expr(StopExpression* expr)
{
	item->setText(0, "Stop");
}

void ExpressionItemBuilder::Expr(AssertExpression* expr)
{
	item->setText(0, "Assert");
	item->add(createItem("Condition", expr->Condition));
}

void ExpressionItemBuilder::Expr(CaseExpression* expr)
{
	item->setText(0, "Case");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(NothingExpression* expr)
{
	item->setText(0, "Nothing");
}

void ExpressionItemBuilder::Expr(LabelTableExpression* expr)
{
	item->setText(0, "Label table");
}

void ExpressionItemBuilder::Expr(GotoLabelExpression* expr)
{
	item->setText(0, "Goto label");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(EatStringExpression* expr)
{
	item->setText(0, "Eat string");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(LetExpression* expr)
{
	item->setText(0, "Let");
	item->add(createItem("Left side", expr->LeftSide));
	item->add(createItem("Right side", expr->RightSide));
}

void ExpressionItemBuilder::Expr(DynArrayElementExpression* expr)
{
	item->setText(0, "Dynamic array element");
	item->add(createItem("Index", expr->Index));
	item->add(createItem("Array", expr->Array));
}

void ExpressionItemBuilder::Expr(NewExpression* expr)
{
	item->setText(0, "New");
	item->add(createItem("Parent", expr->ParentExpr));
	item->add(createItem("Name", expr->NameExpr));
	item->add(createItem("Flags", expr->FlagsExpr));
	item->add(createItem("Class", expr->ClassExpr));
}

void ExpressionItemBuilder::Expr(ClassContextExpression* expr)
{
	item->setText(0, "Class context");
	item->add(createItem("Object", expr->ObjectExpr));
	item->add(createItem("Context", expr->ContextExpr));
}

void ExpressionItemBuilder::Expr(MetaCastExpression* expr)
{
	item->setText(0, "Meta cast");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(LetBoolExpression* expr)
{
	item->setText(0, "Let bool");
	item->add(createItem("Left side", expr->LeftSide));
	item->add(createItem("Right side", expr->RightSide));
}

void ExpressionItemBuilder::Expr(Unknown0x15Expression* expr)
{
	item->setText(0, "0x15");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(SelfExpression* expr)
{
	item->setText(0, "Self");
}

void ExpressionItemBuilder::Expr(SkipExpression* expr)
{
	item->setText(0, "Skip");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(ContextExpression* expr)
{
	item->setText(0, "Context");
	item->add(createItem("Object", expr->ObjectExpr));
	item->add(createItem("Context", expr->ContextExpr));
}

void ExpressionItemBuilder::Expr(ArrayElementExpression* expr)
{
	item->setText(0, "Array element");
	item->add(createItem("Index", expr->Index));
	item->add(createItem("Array", expr->Array));
}

void ExpressionItemBuilder::Expr(IntConstExpression* expr)
{
	item->setText(0, "Int const");
	item->setText(2, std::to_string(expr->Value));
}

void ExpressionItemBuilder::Expr(FloatConstExpression* expr)
{
	item->setText(0, "Float const");
	item->setText(2, std::to_string(expr->Value));
}

void ExpressionItemBuilder::Expr(StringConstExpression* expr)
{
	item->setText(0, "String const");
	item->setText(2, expr->Value);
}

void ExpressionItemBuilder::Expr(ObjectConstExpression* expr)
{
	item->setText(0, "Object const");
	if (expr->Object)
		item->setText(2, "{ name=\"" + expr->Object->Name + "\", class=" + UObject::GetUClassName(expr->Object) + " }");
	else
		item->setText(2, "null");
}

void ExpressionItemBuilder::Expr(NameConstExpression* expr)
{
	item->setText(0, "Name const");
	item->setText(2, expr->Value);
}

void ExpressionItemBuilder::Expr(RotationConstExpression* expr)
{
	item->setText(0, "Rotation const");
	item->setText(2, "{ " + std::to_string(expr->Pitch) + ", " + std::to_string(expr->Yaw) + ", " + std::to_string(expr->Roll) + " }");
}

void ExpressionItemBuilder::Expr(VectorConstExpression* expr)
{
	item->setText(0, "Vector const");
	item->setText(2, "{ " + std::to_string(expr->X) + ", " + std::to_string(expr->Y) + ", " + std::to_string(expr->Z) + " }");
}

void ExpressionItemBuilder::Expr(ByteConstExpression* expr)
{
	item->setText(0, "Byte const");
	item->setText(2, std::to_string(expr->Value));
}

void ExpressionItemBuilder::Expr(IntZeroExpression* expr)
{
	item->setText(0, "Int zero");
	item->setText(2, "0");
}

void ExpressionItemBuilder::Expr(IntOneExpression* expr)
{
	item->setText(0, "Int one");
	item->setText(2, "1");
}

void ExpressionItemBuilder::Expr(TrueExpression* expr)
{
	item->setText(0, "True");
	item->setText(2, "true");
}

void ExpressionItemBuilder::Expr(FalseExpression* expr)
{
	item->setText(0, "False");
	item->setText(2, "false");
}

void ExpressionItemBuilder::Expr(NativeParmExpression* expr)
{
	item->setText(0, "Native parm");
	if (expr->Object)
		item->setText(2, expr->Object->Name);
	else
		item->setText(2, "(null)");
}

void ExpressionItemBuilder::Expr(NoObjectExpression* expr)
{
	item->setText(0, "No object");
	item->setText(2, "null");
}

void ExpressionItemBuilder::Expr(Unknown0x2bExpression* expr)
{
	item->setText(0, "0x2b");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(IntConstByteExpression* expr)
{
	item->setText(0, "Int const byte");
	item->setText(2, std::to_string(expr->Value));
}

void ExpressionItemBuilder::Expr(BoolVariableExpression* expr)
{
	item->setText(0, "Bool variable");
	item->add(createItem("Variable", expr->Variable));
}

void ExpressionItemBuilder::Expr(DynamicCastExpression* expr)
{
	item->setText(0, "Dynamic cast");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(IteratorExpression* expr)
{
	item->setText(0, "Iterator");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(IteratorPopExpression* expr)
{
	item->setText(0, "Iterator pop");
}

void ExpressionItemBuilder::Expr(IteratorNextExpression* expr)
{
	item->setText(0, "Iterator next");
}

void ExpressionItemBuilder::Expr(StructCmpEqExpression* expr)
{
	item->setText(0, "Struct cmp eq");
	item->add(createItem("Value1", expr->Value1));
	item->add(createItem("Value2", expr->Value2));
}

void ExpressionItemBuilder::Expr(StructCmpNeExpression* expr)
{
	item->setText(0, "Struct cmp ne");
	item->add(createItem("Value1", expr->Value1));
	item->add(createItem("Value2", expr->Value2));
}

void ExpressionItemBuilder::Expr(UnicodeStringConstExpression* expr)
{
	item->setText(0, "Unicode string const");
}

void ExpressionItemBuilder::Expr(StructMemberExpression* expr)
{
	item->setText(0, "Struct member");
	item->add(createItem("Struct", expr->Value));

	auto fieldItem = std::make_unique<TextListViewItem>();
	fieldItem->setText(0, "Property");
	fieldItem->setText(1, "Field");
	fieldItem->setText(2, expr->Field->Name);
	item->add(std::move(fieldItem));
}

void ExpressionItemBuilder::Expr(RotatorToVectorExpression* expr)
{
	item->setText(0, "Rotator to vector");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(ByteToIntExpression* expr)
{
	item->setText(0, "Byte to int");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(ByteToBoolExpression* expr)
{
	item->setText(0, "Byte to bool");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(ByteToFloatExpression* expr)
{
	item->setText(0, "Byte to float");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(IntToByteExpression* expr)
{
	item->setText(0, "Int to byte");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(IntToBoolExpression* expr)
{
	item->setText(0, "Int to bool");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(IntToFloatExpression* expr)
{
	item->setText(0, "Int to float");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(BoolToByteExpression* expr)
{
	item->setText(0, "Bool to byte");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(BoolToIntExpression* expr)
{
	item->setText(0, "Bool to int");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(BoolToFloatExpression* expr)
{
	item->setText(0, "Bool to float");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(FloatToByteExpression* expr)
{
	item->setText(0, "Float to byte");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(FloatToIntExpression* expr)
{
	item->setText(0, "Float to int");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(FloatToBoolExpression* expr)
{
	item->setText(0, "Float to bool");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(Unknown0x46Expression* expr)
{
	item->setText(0, "0x46");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(ObjectToBoolExpression* expr)
{
	item->setText(0, "Object to bool");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(NameToBoolExpression* expr)
{
	item->setText(0, "Name to bool");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(StringToByteExpression* expr)
{
	item->setText(0, "String to byte");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(StringToIntExpression* expr)
{
	item->setText(0, "String to int");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(StringToBoolExpression* expr)
{
	item->setText(0, "String to bool");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(StringToFloatExpression* expr)
{
	item->setText(0, "String to float");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(StringToVectorExpression* expr)
{
	item->setText(0, "String to vector");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(StringToRotatorExpression* expr)
{
	item->setText(0, "String to rotator");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(VectorToBoolExpression* expr)
{
	item->setText(0, "Vector to bool");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(VectorToRotatorExpression* expr)
{
	item->setText(0, "Vector to rotator");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(RotatorToBoolExpression* expr)
{
	item->setText(0, "Rotator to bool");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(ByteToStringExpression* expr)
{
	item->setText(0, "Byte to string");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(IntToStringExpression* expr)
{
	item->setText(0, "Int to string");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(BoolToStringExpression* expr)
{
	item->setText(0, "Bool to string");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(FloatToStringExpression* expr)
{
	item->setText(0, "Float to string");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(ObjectToStringExpression* expr)
{
	item->setText(0, "Object to string");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(NameToStringExpression* expr)
{
	item->setText(0, "Name to string");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(VectorToStringExpression* expr)
{
	item->setText(0, "Vector to string");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(RotatorToStringExpression* expr)
{
	item->setText(0, "Rotator to string");
	item->add(createItem("Value", expr->Value));
}

void ExpressionItemBuilder::Expr(VirtualFunctionExpression* expr)
{
	item->setText(0, "Virtual function");
	item->setText(2, expr->Name);
	int index = 0;
	for (auto arg : expr->Args)
	{
		item->add(createItem("Arg[" + std::to_string(index) + "]", arg));
		index++;
	}
}

void ExpressionItemBuilder::Expr(FinalFunctionExpression* expr)
{
	item->setText(0, "Final function");
	item->setText(2, GetFullFuncName(expr->Func));
	int index = 0;
	for (auto arg : expr->Args)
	{
		item->add(createItem("Arg[" + std::to_string(index) + "]", arg));
		index++;
	}
}

void ExpressionItemBuilder::Expr(GlobalFunctionExpression* expr)
{
	item->setText(0, "Global function");
	item->setText(2, expr->Name);
	int index = 0;
	for (auto arg : expr->Args)
	{
		item->add(createItem("Arg[" + std::to_string(index) + "]", arg));
		index++;
	}
}

void ExpressionItemBuilder::Expr(NativeFunctionExpression* expr)
{
	item->setText(0, "Native function");
	item->setText(2, GetFullFuncName(NativeFunctions::FuncByIndex[expr->nativeindex]));
	int index = 0;
	for (auto arg : expr->Args)
	{
		item->add(createItem("Arg[" + std::to_string(index) + "]", arg));
		index++;
	}
}

std::string ExpressionItemBuilder::GetFullFuncName(UFunction* func)
{
	std::string name;
	for (UStruct* s = func; s != nullptr; s = s->StructParent)
	{
		if (name.empty())
			name = s->Name;
		else
			name = s->Name + "." + name;
	}
	return name;
}
