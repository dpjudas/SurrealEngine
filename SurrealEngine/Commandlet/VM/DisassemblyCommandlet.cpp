
#include "Precomp.h"
#include "DisassemblyCommandlet.h"
#include "DebuggerApp.h"
#include "VM/Expression.h"
#include "VM/NativeFunc.h"
#include "VM/Frame.h"
#include "VM/Bytecode.h"
#include "UObject/UProperty.h"

DisassemblyCommandlet::DisassemblyCommandlet()
{
	SetShortFormName("d");
	SetLongFormName("disassembly");
	SetShortDescription("Print disassembly for function");
}

void DisassemblyCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	Frame* frame = console->GetCurrentFrame();
	if (frame && frame->Func)
	{
		int index = 0;
		for (Expression* expr : frame->Func->Code->Statements)
		{
			if (expr == Frame::StepExpression)
			{
				if (args == "full")
				{
					PrintExpression::Print(console, "Statement[" + std::to_string(index) + "]", expr);
				}
				else
				{
					PrintPrettyExpression::Print(console, expr);
					console->WriteOutput(console->NewLine());
				}
			}
			index++;
		}
	}
}

void DisassemblyCommandlet::OnPrintHelp(DebuggerApp* console)
{
}

/////////////////////////////////////////////////////////////////////////////

void PrintExpression::Print(DebuggerApp* console, const std::string& name, Expression* expr, int depth)
{
	PrintExpression builder;
	builder.console = console;
	builder.name = name;
	builder.depth = depth;

	if (expr == Frame::StepExpression)
		console->WriteOutput("--> ");
	else
		console->WriteOutput("    ");

	if (expr)
	{
		expr->Visit(&builder);
	}
	else
	{
		builder.WriteRow("Null", name, "");
	}
}

void PrintExpression::WriteRow(std::string type, std::string name, std::string value)
{
	size_t valuesize = (size_t)std::max(60 - depth * 2, 0);
	if (value.size() < valuesize)
		value.resize(valuesize, ' ');

	if (name.size() < 20)
		name.resize(20, ' ');

	std::string spaces;
	spaces.resize(depth * 2, ' ');

	console->WriteOutput(spaces + value + " " + name + " " + type + console->NewLine());
}

void PrintExpression::Expr(LocalVariableExpression* expr)
{
	if (expr->Variable)
		WriteRow("Local variable", name, expr->Variable->Name.ToString());
	else
		WriteRow("Local variable", name, "(null)");
}

void PrintExpression::Expr(InstanceVariableExpression* expr)
{
	if (expr->Variable)
		WriteRow("Instance variable", name, expr->Variable->Name.ToString());
	else
		WriteRow("Instance variable", name, "(null)");
}

void PrintExpression::Expr(DefaultVariableExpression* expr)
{
	if (expr->Variable)
		WriteRow("Default variable", name, expr->Variable->Name.ToString());
	else
		WriteRow("Default variable", name, "(null)");
}

void PrintExpression::Expr(ReturnExpression* expr)
{
	WriteRow("", name, "Return");
	if (expr->Value)
		Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(SwitchExpression* expr)
{
	WriteRow("", name, "Switch");
	Print(console, "Condition", expr->Condition, depth + 1);
}

void PrintExpression::Expr(JumpExpression* expr)
{
	WriteRow("", name, "Jump");
}

void PrintExpression::Expr(JumpIfNotExpression* expr)
{
	WriteRow("", name, "Jump if not");
	Print(console, "Condition", expr->Condition, depth + 1);
}

void PrintExpression::Expr(StopExpression* expr)
{
	WriteRow("", name, "Stop");
}

void PrintExpression::Expr(AssertExpression* expr)
{
	WriteRow("", name, "Assert");
	Print(console, "Condition", expr->Condition, depth + 1);
}

void PrintExpression::Expr(CaseExpression* expr)
{
	WriteRow("", name, "Case");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(NothingExpression* expr)
{
	WriteRow("", name, "Nothing");
}

void PrintExpression::Expr(LabelTableExpression* expr)
{
	WriteRow("", name, "Label table");
}

void PrintExpression::Expr(GotoLabelExpression* expr)
{
	WriteRow("", name, "Goto label");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(EatStringExpression* expr)
{
	WriteRow("", name, "Eat string");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(LetExpression* expr)
{
	WriteRow("", name, "Let");
	Print(console, "Left side", expr->LeftSide, depth + 1);
	Print(console, "Right side", expr->RightSide, depth + 1);
}

void PrintExpression::Expr(DynArrayElementExpression* expr)
{
	WriteRow("", name, "Dynamic array element");
	Print(console, "Index", expr->Index, depth + 1);
	Print(console, "Array", expr->Array, depth + 1);
}

void PrintExpression::Expr(NewExpression* expr)
{
	WriteRow("", name, "New");
	Print(console, "Parent", expr->ParentExpr, depth + 1);
	Print(console, "Name", expr->NameExpr, depth + 1);
	Print(console, "Flags", expr->FlagsExpr, depth + 1);
	Print(console, "Class", expr->ClassExpr, depth + 1);
}

void PrintExpression::Expr(ClassContextExpression* expr)
{
	WriteRow("", name, "Class context");
	Print(console, "Object", expr->ObjectExpr, depth + 1);
	Print(console, "Context", expr->ContextExpr, depth + 1);
}

void PrintExpression::Expr(MetaCastExpression* expr)
{
	WriteRow("", name, "Meta cast");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(LetBoolExpression* expr)
{
	WriteRow("", name, "Let bool");
	Print(console, "Left side", expr->LeftSide, depth + 1);
	Print(console, "Right side", expr->RightSide, depth + 1);
}

void PrintExpression::Expr(Unknown0x15Expression* expr)
{
	WriteRow("", name, "0x15");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(SelfExpression* expr)
{
	WriteRow("", name, "Self");
}

void PrintExpression::Expr(SkipExpression* expr)
{
	WriteRow("", name, "Skip");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(ContextExpression* expr)
{
	WriteRow("", name, "Context");
	Print(console, "Object", expr->ObjectExpr, depth + 1);
	Print(console, "Context", expr->ContextExpr, depth + 1);
}

void PrintExpression::Expr(ArrayElementExpression* expr)
{
	WriteRow("", name, "Array element");
	Print(console, "Index", expr->Index, depth + 1);
	Print(console, "Array", expr->Array, depth + 1);
}

void PrintExpression::Expr(IntConstExpression* expr)
{
	WriteRow("Int const", name, std::to_string(expr->Value));
}

void PrintExpression::Expr(FloatConstExpression* expr)
{
	WriteRow("Float const", name, std::to_string(expr->Value));
}

void PrintExpression::Expr(StringConstExpression* expr)
{
	WriteRow("String const", name, "\"" + expr->Value + "\"");
}

void PrintExpression::Expr(ObjectConstExpression* expr)
{
	if (expr->Object)
		WriteRow("Object const", name, "{ name=\"" + expr->Object->Name.ToString() + "\", class=" + UObject::GetUClassName(expr->Object).ToString() + " }");
	else
		WriteRow("Object const", name, "null");
}

void PrintExpression::Expr(NameConstExpression* expr)
{
	WriteRow("Name const", name, expr->Value.ToString());
}

void PrintExpression::Expr(RotationConstExpression* expr)
{
	WriteRow("Rotation const", name, "{ " + std::to_string(expr->Pitch) + ", " + std::to_string(expr->Yaw) + ", " + std::to_string(expr->Roll) + " }");
}

void PrintExpression::Expr(VectorConstExpression* expr)
{
	WriteRow("Vector const", name, "{ " + std::to_string(expr->X) + ", " + std::to_string(expr->Y) + ", " + std::to_string(expr->Z) + " }");
}

void PrintExpression::Expr(ByteConstExpression* expr)
{
	WriteRow("Byte const", name, std::to_string(expr->Value));
}

void PrintExpression::Expr(IntZeroExpression* expr)
{
	WriteRow("Int zero", name, "0");
}

void PrintExpression::Expr(IntOneExpression* expr)
{
	WriteRow("Int one", name, "1");
}

void PrintExpression::Expr(TrueExpression* expr)
{
	WriteRow("True", name, "true");
}

void PrintExpression::Expr(FalseExpression* expr)
{
	WriteRow("False", name, "false");
}

void PrintExpression::Expr(NativeParmExpression* expr)
{
	if (expr->Object)
		WriteRow("Native parm", name, expr->Object->Name.ToString());
	else
		WriteRow("Native parm", name, "(null)");
}

void PrintExpression::Expr(NoObjectExpression* expr)
{
	WriteRow("No object", name, "null");
}

void PrintExpression::Expr(Unknown0x2bExpression* expr)
{
	WriteRow("", name, "0x2b");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(IntConstByteExpression* expr)
{
	WriteRow("Int const byte", name, std::to_string(expr->Value));
}

void PrintExpression::Expr(BoolVariableExpression* expr)
{
	WriteRow("", name, "Bool variable");
	Print(console, "Variable", expr->Variable, depth + 1);
}

void PrintExpression::Expr(DynamicCastExpression* expr)
{
	WriteRow("", name, "Dynamic cast");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(IteratorExpression* expr)
{
	WriteRow("", name, "Iterator");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(IteratorPopExpression* expr)
{
	WriteRow("", name, "Iterator pop");
}

void PrintExpression::Expr(IteratorNextExpression* expr)
{
	WriteRow("", name, "Iterator next");
}

void PrintExpression::Expr(StructCmpEqExpression* expr)
{
	WriteRow("", name, "Struct cmp eq");
	Print(console, "Value1", expr->Value1, depth + 1);
	Print(console, "Value2", expr->Value2, depth + 1);
}

void PrintExpression::Expr(StructCmpNeExpression* expr)
{
	WriteRow("", name, "Struct cmp ne");
	Print(console, "Value1", expr->Value1, depth + 1);
	Print(console, "Value2", expr->Value2, depth + 1);
}

void PrintExpression::Expr(UnicodeStringConstExpression* expr)
{
	WriteRow("Unicode string const", name, "");
}

void PrintExpression::Expr(StructMemberExpression* expr)
{
	WriteRow("", name, "Struct member");
	Print(console, "Struct", expr->Value, depth + 1);
	depth++;
	console->WriteOutput("    ");
	WriteRow("Property", "Field", expr->Field->Name.ToString());
	depth--;
}

void PrintExpression::Expr(RotatorToVectorExpression* expr)
{
	WriteRow("", name, "Rotator to vector");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(ByteToIntExpression* expr)
{
	WriteRow("", name, "Byte to int");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(ByteToBoolExpression* expr)
{
	WriteRow("", name, "Byte to bool");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(ByteToFloatExpression* expr)
{
	WriteRow("", name, "Byte to float");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(IntToByteExpression* expr)
{
	WriteRow("", name, "Int to byte");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(IntToBoolExpression* expr)
{
	WriteRow("", name, "Int to bool");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(IntToFloatExpression* expr)
{
	WriteRow("", name, "Int to float");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(BoolToByteExpression* expr)
{
	WriteRow("", name, "Bool to byte");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(BoolToIntExpression* expr)
{
	WriteRow("", name, "Bool to int");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(BoolToFloatExpression* expr)
{
	WriteRow("", name, "Bool to float");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(FloatToByteExpression* expr)
{
	WriteRow("", name, "Float to byte");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(FloatToIntExpression* expr)
{
	WriteRow("", name, "Float to int");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(FloatToBoolExpression* expr)
{
	WriteRow("", name, "Float to bool");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(Unknown0x46Expression* expr)
{
	WriteRow("", name, "0x46");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(ObjectToBoolExpression* expr)
{
	WriteRow("", name, "Object to bool");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(NameToBoolExpression* expr)
{
	WriteRow("", name, "Name to bool");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(StringToByteExpression* expr)
{
	WriteRow("", name, "String to byte");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(StringToIntExpression* expr)
{
	WriteRow("", name, "String to int");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(StringToBoolExpression* expr)
{
	WriteRow("", name, "String to bool");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(StringToFloatExpression* expr)
{
	WriteRow("", name, "String to float");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(StringToVectorExpression* expr)
{
	WriteRow("", name, "String to vector");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(StringToRotatorExpression* expr)
{
	WriteRow("", name, "String to rotator");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(VectorToBoolExpression* expr)
{
	WriteRow("", name, "Vector to bool");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(VectorToRotatorExpression* expr)
{
	WriteRow("", name, "Vector to rotator");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(RotatorToBoolExpression* expr)
{
	WriteRow("", name, "Rotator to bool");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(ByteToStringExpression* expr)
{
	WriteRow("", name, "Byte to string");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(IntToStringExpression* expr)
{
	WriteRow("", name, "Int to string");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(BoolToStringExpression* expr)
{
	WriteRow("", name, "Bool to string");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(FloatToStringExpression* expr)
{
	WriteRow("", name, "Float to string");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(ObjectToStringExpression* expr)
{
	WriteRow("", name, "Object to string");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(NameToStringExpression* expr)
{
	WriteRow("", name, "Name to string");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(VectorToStringExpression* expr)
{
	WriteRow("", name, "Vector to string");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(RotatorToStringExpression* expr)
{
	WriteRow("", name, "Rotator to string");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(VirtualFunctionExpression* expr)
{
	WriteRow("Virtual call", name, expr->Name.ToString());
	int index = 0;
	for (auto arg : expr->Args)
	{
		Print(console, "Arg[" + std::to_string(index) + "]", arg, depth + 1);
		index++;
	}
}

void PrintExpression::Expr(FinalFunctionExpression* expr)
{
	WriteRow("Final call", name, GetFullFuncName(expr->Func));
	int index = 0;
	for (auto arg : expr->Args)
	{
		Print(console, "Arg[" + std::to_string(index) + "]", arg, depth + 1);
		index++;
	}
}

void PrintExpression::Expr(GlobalFunctionExpression* expr)
{
	WriteRow("Global call", name, expr->Name.ToString());
	int index = 0;
	for (auto arg : expr->Args)
	{
		Print(console, "Arg[" + std::to_string(index) + "]", arg, depth + 1);
		index++;
	}
}

void PrintExpression::Expr(NativeFunctionExpression* expr)
{
	WriteRow("Native call", name, GetFullFuncName(NativeFunctions::FuncByIndex[expr->nativeindex]));
	int index = 0;
	for (auto arg : expr->Args)
	{
		Print(console, "Arg[" + std::to_string(index) + "]", arg, depth + 1);
		index++;
	}
}

void PrintExpression::Expr(FunctionArgumentsExpression* expr)
{
	WriteRow("", name, "Function arguments");
}

void PrintExpression::Expr(ConstructExpression* expr)
{
	WriteRow("Construct struct", name, expr->Struct->Name.ToString());
	int index = 0;
	for (auto arg : expr->Args)
	{
		if (arg.Name)
		{
			Print(console, "Arg['" + arg.Name->Name.ToString() + "']", arg.Value, depth + 1);
		}
		else
		{
			Print(console, "Arg[" + std::to_string(index) + "]", arg.Value, depth + 1);
		}
		index++;
	}
}

std::string PrintExpression::GetFullFuncName(UFunction* func)
{
	std::string name;
	for (UStruct* s = func; s != nullptr; s = s->StructParent)
	{
		if (name.empty())
			name = s->Name.ToString();
		else
			name = s->Name.ToString() + "." + name;
	}
	return name;
}

/////////////////////////////////////////////////////////////////////////////

void PrintPrettyExpression::Print(DebuggerApp* console, Expression* expr)
{
	PrintPrettyExpression builder;
	builder.console = console;

	if (expr)
	{
		expr->Visit(&builder);
	}
	else
	{
		console->WriteOutput("Null");
	}
}

void PrintPrettyExpression::Expr(LocalVariableExpression* expr)
{
	if (expr->Variable)
		console->WriteOutput(expr->Variable->Name.ToString());
	else
		console->WriteOutput("(null)");
}

void PrintPrettyExpression::Expr(InstanceVariableExpression* expr)
{
	if (expr->Variable)
		console->WriteOutput("" + expr->Variable->Name.ToString());
	else
		console->WriteOutput("(null)");
}

void PrintPrettyExpression::Expr(DefaultVariableExpression* expr)
{
	if (expr->Variable)
		console->WriteOutput("default."+ expr->Variable->Name.ToString());
	else
		console->WriteOutput("default.(null)");
}

void PrintPrettyExpression::Expr(ReturnExpression* expr)
{
	console->WriteOutput("return");
	if (expr->Value)
	{
		console->WriteOutput(" ");
		Print(console, expr->Value);
	}
}

void PrintPrettyExpression::Expr(SwitchExpression* expr)
{
	console->WriteOutput("switch (");
	Print(console, expr->Condition);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(JumpExpression* expr)
{
	console->WriteOutput("jump");
}

void PrintPrettyExpression::Expr(JumpIfNotExpression* expr)
{
	console->WriteOutput("if (");
	Print(console, expr->Condition);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(StopExpression* expr)
{
	console->WriteOutput("stop");
}

void PrintPrettyExpression::Expr(AssertExpression* expr)
{
	console->WriteOutput("assert(");
	Print(console, expr->Condition);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(CaseExpression* expr)
{
	console->WriteOutput("case ");
	Print(console, expr->Value);
	console->WriteOutput(":");
}

void PrintPrettyExpression::Expr(NothingExpression* expr)
{
	console->WriteOutput("nothing");
}

void PrintPrettyExpression::Expr(LabelTableExpression* expr)
{
	console->WriteOutput("label table");
}

void PrintPrettyExpression::Expr(GotoLabelExpression* expr)
{
	console->WriteOutput("goto ");
	Print(console, expr->Value);
}

void PrintPrettyExpression::Expr(EatStringExpression* expr)
{
	console->WriteOutput("eat string(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(LetExpression* expr)
{
	Print(console, expr->LeftSide);
	console->WriteOutput(" = ");
	Print(console, expr->RightSide);
}

void PrintPrettyExpression::Expr(DynArrayElementExpression* expr)
{
	Print(console, expr->Array);
	console->WriteOutput("[");
	Print(console, expr->Index);
	console->WriteOutput("]");
}

void PrintPrettyExpression::Expr(NewExpression* expr)
{
	console->WriteOutput("new(parent: ");
	Print(console, expr->ParentExpr);
	console->WriteOutput(", name: ");
	Print(console, expr->NameExpr);
	console->WriteOutput(", flags: ");
	Print(console, expr->FlagsExpr);
	console->WriteOutput(", class: ");
	Print(console, expr->ClassExpr);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(ClassContextExpression* expr)
{
	console->WriteOutput("class<");
	Print(console, expr->ObjectExpr);
	console->WriteOutput(".");
	Print(console, expr->ContextExpr);
	console->WriteOutput(">");
}

void PrintPrettyExpression::Expr(MetaCastExpression* expr)
{
	console->WriteOutput("meta cast(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(LetBoolExpression* expr)
{
	Print(console, expr->LeftSide);
	console->WriteOutput(" = ");
	Print(console, expr->RightSide);
}

void PrintPrettyExpression::Expr(Unknown0x15Expression* expr)
{
	console->WriteOutput("unknown(0x15)");
}

void PrintPrettyExpression::Expr(SelfExpression* expr)
{
	console->WriteOutput("this");
}

void PrintPrettyExpression::Expr(SkipExpression* expr)
{
	console->WriteOutput("skip(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(ContextExpression* expr)
{
	Print(console, expr->ObjectExpr);
	console->WriteOutput(".");
	Print(console, expr->ContextExpr);
}

void PrintPrettyExpression::Expr(ArrayElementExpression* expr)
{
	Print(console, expr->Array);
	console->WriteOutput("[");
	Print(console, expr->Index);
	console->WriteOutput("]");
}

void PrintPrettyExpression::Expr(IntConstExpression* expr)
{
	console->WriteOutput(std::to_string(expr->Value));
}

void PrintPrettyExpression::Expr(FloatConstExpression* expr)
{
	console->WriteOutput(std::to_string(expr->Value));
}

void PrintPrettyExpression::Expr(StringConstExpression* expr)
{
	console->WriteOutput("\"" + expr->Value + "\"");
}

void PrintPrettyExpression::Expr(ObjectConstExpression* expr)
{
	if (expr->Object)
		console->WriteOutput("{ name=\"" + expr->Object->Name.ToString() + "\", class=" + UObject::GetUClassName(expr->Object).ToString() + " }");
	else
		console->WriteOutput("null");
}

void PrintPrettyExpression::Expr(NameConstExpression* expr)
{
	console->WriteOutput(expr->Value.ToString());
}

void PrintPrettyExpression::Expr(RotationConstExpression* expr)
{
	console->WriteOutput("{ " + std::to_string(expr->Pitch) + ", " + std::to_string(expr->Yaw) + ", " + std::to_string(expr->Roll) + " }");
}

void PrintPrettyExpression::Expr(VectorConstExpression* expr)
{
	console->WriteOutput("{ " + std::to_string(expr->X) + ", " + std::to_string(expr->Y) + ", " + std::to_string(expr->Z) + " }");
}

void PrintPrettyExpression::Expr(ByteConstExpression* expr)
{
	console->WriteOutput(std::to_string(expr->Value));
}

void PrintPrettyExpression::Expr(IntZeroExpression* expr)
{
	console->WriteOutput("0");
}

void PrintPrettyExpression::Expr(IntOneExpression* expr)
{
	console->WriteOutput("1");
}

void PrintPrettyExpression::Expr(TrueExpression* expr)
{
	console->WriteOutput("true");
}

void PrintPrettyExpression::Expr(FalseExpression* expr)
{
	console->WriteOutput("false");
}

void PrintPrettyExpression::Expr(NativeParmExpression* expr)
{
	if (expr->Object)
		console->WriteOutput(expr->Object->Name.ToString());
	else
		console->WriteOutput("(null)");
}

void PrintPrettyExpression::Expr(NoObjectExpression* expr)
{
	console->WriteOutput("null");
}

void PrintPrettyExpression::Expr(Unknown0x2bExpression* expr)
{
	console->WriteOutput("unknown(0x2b, ");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(IntConstByteExpression* expr)
{
	console->WriteOutput(std::to_string(expr->Value));
}

void PrintPrettyExpression::Expr(BoolVariableExpression* expr)
{
	Print(console, expr->Variable);
}

void PrintPrettyExpression::Expr(DynamicCastExpression* expr)
{
	console->WriteOutput("dynamic cast(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(IteratorExpression* expr)
{
	console->WriteOutput("iterator(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(IteratorPopExpression* expr)
{
	console->WriteOutput("iterator pop");
}

void PrintPrettyExpression::Expr(IteratorNextExpression* expr)
{
	console->WriteOutput("iterator next");
}

void PrintPrettyExpression::Expr(StructCmpEqExpression* expr)
{
	Print(console, expr->Value1);
	console->WriteOutput(" == ");
	Print(console, expr->Value2);
}

void PrintPrettyExpression::Expr(StructCmpNeExpression* expr)
{
	Print(console, expr->Value1);
	console->WriteOutput(" != ");
	Print(console, expr->Value2);
}

void PrintPrettyExpression::Expr(UnicodeStringConstExpression* expr)
{
	console->WriteOutput("unicode string");
}

void PrintPrettyExpression::Expr(StructMemberExpression* expr)
{
	Print(console, expr->Value);
	console->WriteOutput(".");
	console->WriteOutput(expr->Field->Name.ToString());
}

void PrintPrettyExpression::Expr(RotatorToVectorExpression* expr)
{
	console->WriteOutput("rotatorToVector(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(ByteToIntExpression* expr)
{
	console->WriteOutput("byteToInt(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(ByteToBoolExpression* expr)
{
	console->WriteOutput("byteToBool(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(ByteToFloatExpression* expr)
{
	console->WriteOutput("byteToFloat(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(IntToByteExpression* expr)
{
	console->WriteOutput("intToByte(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(IntToBoolExpression* expr)
{
	console->WriteOutput("intToBool(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(IntToFloatExpression* expr)
{
	console->WriteOutput("intToFloat(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(BoolToByteExpression* expr)
{
	console->WriteOutput("boolToByte(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(BoolToIntExpression* expr)
{
	console->WriteOutput("boolToInt(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(BoolToFloatExpression* expr)
{
	console->WriteOutput("boolToFloat(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(FloatToByteExpression* expr)
{
	console->WriteOutput("floatToByte(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(FloatToIntExpression* expr)
{
	console->WriteOutput("floatToInt(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(FloatToBoolExpression* expr)
{
	console->WriteOutput("floatToBool(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(Unknown0x46Expression* expr)
{
	console->WriteOutput("unknown(0x46, ");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(ObjectToBoolExpression* expr)
{
	console->WriteOutput("objectToBool(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(NameToBoolExpression* expr)
{
	console->WriteOutput("nameToBool(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(StringToByteExpression* expr)
{
	console->WriteOutput("stringToByte(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(StringToIntExpression* expr)
{
	console->WriteOutput("stringToInt(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(StringToBoolExpression* expr)
{
	console->WriteOutput("stringToBool(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(StringToFloatExpression* expr)
{
	console->WriteOutput("stringToFloat(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(StringToVectorExpression* expr)
{
	console->WriteOutput("stringToVector(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(StringToRotatorExpression* expr)
{
	console->WriteOutput("stringToRotator(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(VectorToBoolExpression* expr)
{
	console->WriteOutput("vectorToBool(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(VectorToRotatorExpression* expr)
{
	console->WriteOutput("vectorToRotator(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(RotatorToBoolExpression* expr)
{
	console->WriteOutput("rotatorToBool(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(ByteToStringExpression* expr)
{
	console->WriteOutput("byteToString(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(IntToStringExpression* expr)
{
	console->WriteOutput("intToString(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(BoolToStringExpression* expr)
{
	console->WriteOutput("boolToString(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(FloatToStringExpression* expr)
{
	console->WriteOutput("floatToString(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(ObjectToStringExpression* expr)
{
	console->WriteOutput("objectToString(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(NameToStringExpression* expr)
{
	console->WriteOutput("nameToString(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(VectorToStringExpression* expr)
{
	console->WriteOutput("vectorToString(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(RotatorToStringExpression* expr)
{
	console->WriteOutput("rotatorToString(");
	Print(console, expr->Value);
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(VirtualFunctionExpression* expr)
{
	console->WriteOutput(expr->Name.ToString() + "(");
	int index = 0;
	for (auto arg : expr->Args)
	{
		if (index != 0)
			console->WriteOutput(", ");
		Print(console, arg);
		index++;
	}
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(FinalFunctionExpression* expr)
{
	console->WriteOutput(GetFullFuncName(expr->Func) + "(");
	int index = 0;
	for (auto arg : expr->Args)
	{
		if (index != 0)
			console->WriteOutput(", ");
		Print(console, arg);
		index++;
	}
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(GlobalFunctionExpression* expr)
{
	console->WriteOutput(expr->Name.ToString() + "(");
	int index = 0;
	for (auto arg : expr->Args)
	{
		if (index != 0)
			console->WriteOutput(", ");
		Print(console, arg);
		index++;
	}
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(NativeFunctionExpression* expr)
{
	console->WriteOutput(GetFullFuncName(NativeFunctions::FuncByIndex[expr->nativeindex]) + "(");
	int index = 0;
	for (auto arg : expr->Args)
	{
		if (index != 0)
			console->WriteOutput(", ");
		Print(console, arg);
		index++;
	}
	console->WriteOutput(")");
}

void PrintPrettyExpression::Expr(FunctionArgumentsExpression* expr)
{
	console->WriteOutput("function arguments");
}

void PrintPrettyExpression::Expr(ConstructExpression* expr)
{
	console->WriteOutput("construct " + expr->Struct->Name.ToString() + "(");
	int index = 0;
	for (auto arg : expr->Args)
	{
		if (index != 0)
			console->WriteOutput(", ");
		if (arg.Name)
			console->WriteOutput(arg.Name->Name.ToString() + ": ");
		Print(console, arg.Value);
		index++;
	}
	console->WriteOutput(")");
}

std::string PrintPrettyExpression::GetFullFuncName(UFunction* func)
{
	std::string name;
	for (UStruct* s = func; s != nullptr; s = s->StructParent)
	{
		if (name.empty())
			name = s->Name.ToString();
		else
			name = s->Name.ToString() + "." + name;
	}
	return name;
}
