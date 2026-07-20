
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
					PrintPrettyExpression::Print([&](const std::string& text) { console->WriteOutput(text); }, expr);
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

void PrintExpression::Expr(StringToNameExpression* expr)
{
	WriteRow("", name, "String to name");
	Print(console, "Value", expr->Value, depth + 1);
}

void PrintExpression::Expr(DynArrayToIntExpression* expr)
{
	WriteRow("", name, "DynArray to int");
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

void PrintPrettyExpression::Print(std::function<void(const std::string&)> writeOutput, Expression* expr)
{
	PrintPrettyExpression builder;
	builder.writeOutput = writeOutput;

	if (expr)
	{
		expr->Visit(&builder);
	}
	else
	{
		writeOutput("Null");
	}
}

void PrintPrettyExpression::Expr(LocalVariableExpression* expr)
{
	if (expr->Variable)
		writeOutput(expr->Variable->Name.ToString());
	else
		writeOutput("(null)");
}

void PrintPrettyExpression::Expr(InstanceVariableExpression* expr)
{
	if (expr->Variable)
		writeOutput("" + expr->Variable->Name.ToString());
	else
		writeOutput("(null)");
}

void PrintPrettyExpression::Expr(DefaultVariableExpression* expr)
{
	if (expr->Variable)
		writeOutput("default."+ expr->Variable->Name.ToString());
	else
		writeOutput("default.(null)");
}

void PrintPrettyExpression::Expr(ReturnExpression* expr)
{
	writeOutput("return");
	if (expr->Value)
	{
		writeOutput(" ");
		Print(writeOutput, expr->Value);
	}
}

void PrintPrettyExpression::Expr(SwitchExpression* expr)
{
	writeOutput("switch (");
	Print(writeOutput, expr->Condition);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(JumpExpression* expr)
{
	writeOutput("jump");
}

void PrintPrettyExpression::Expr(JumpIfNotExpression* expr)
{
	writeOutput("if (");
	Print(writeOutput, expr->Condition);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(StopExpression* expr)
{
	writeOutput("stop");
}

void PrintPrettyExpression::Expr(AssertExpression* expr)
{
	writeOutput("assert(");
	Print(writeOutput, expr->Condition);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(CaseExpression* expr)
{
	writeOutput("case ");
	Print(writeOutput, expr->Value);
	writeOutput(":");
}

void PrintPrettyExpression::Expr(NothingExpression* expr)
{
	writeOutput("nothing");
}

void PrintPrettyExpression::Expr(LabelTableExpression* expr)
{
	writeOutput("label table");
}

void PrintPrettyExpression::Expr(GotoLabelExpression* expr)
{
	writeOutput("goto ");
	Print(writeOutput, expr->Value);
}

void PrintPrettyExpression::Expr(EatStringExpression* expr)
{
	writeOutput("eat string(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(LetExpression* expr)
{
	Print(writeOutput, expr->LeftSide);
	writeOutput(" = ");
	Print(writeOutput, expr->RightSide);
}

void PrintPrettyExpression::Expr(DynArrayElementExpression* expr)
{
	Print(writeOutput, expr->Array);
	writeOutput("[");
	Print(writeOutput, expr->Index);
	writeOutput("]");
}

void PrintPrettyExpression::Expr(NewExpression* expr)
{
	writeOutput("new(parent: ");
	Print(writeOutput, expr->ParentExpr);
	writeOutput(", name: ");
	Print(writeOutput, expr->NameExpr);
	writeOutput(", flags: ");
	Print(writeOutput, expr->FlagsExpr);
	writeOutput(", class: ");
	Print(writeOutput, expr->ClassExpr);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(ClassContextExpression* expr)
{
	writeOutput("class<");
	Print(writeOutput, expr->ObjectExpr);
	writeOutput(".");
	Print(writeOutput, expr->ContextExpr);
	writeOutput(">");
}

void PrintPrettyExpression::Expr(MetaCastExpression* expr)
{
	writeOutput("meta cast(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(LetBoolExpression* expr)
{
	Print(writeOutput, expr->LeftSide);
	writeOutput(" = ");
	Print(writeOutput, expr->RightSide);
}

void PrintPrettyExpression::Expr(Unknown0x15Expression* expr)
{
	writeOutput("unknown(0x15)");
}

void PrintPrettyExpression::Expr(SelfExpression* expr)
{
	writeOutput("this");
}

void PrintPrettyExpression::Expr(SkipExpression* expr)
{
	writeOutput("skip(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(ContextExpression* expr)
{
	Print(writeOutput, expr->ObjectExpr);
	writeOutput(".");
	Print(writeOutput, expr->ContextExpr);
}

void PrintPrettyExpression::Expr(ArrayElementExpression* expr)
{
	Print(writeOutput, expr->Array);
	writeOutput("[");
	Print(writeOutput, expr->Index);
	writeOutput("]");
}

void PrintPrettyExpression::Expr(IntConstExpression* expr)
{
	writeOutput(std::to_string(expr->Value));
}

void PrintPrettyExpression::Expr(FloatConstExpression* expr)
{
	writeOutput(std::to_string(expr->Value));
}

void PrintPrettyExpression::Expr(StringConstExpression* expr)
{
	writeOutput("\"" + expr->Value + "\"");
}

void PrintPrettyExpression::Expr(ObjectConstExpression* expr)
{
	if (expr->Object)
		writeOutput("{ name=\"" + expr->Object->Name.ToString() + "\", class=" + UObject::GetUClassName(expr->Object).ToString() + " }");
	else
		writeOutput("null");
}

void PrintPrettyExpression::Expr(NameConstExpression* expr)
{
	writeOutput(expr->Value.ToString());
}

void PrintPrettyExpression::Expr(RotationConstExpression* expr)
{
	writeOutput("{ " + std::to_string(expr->Pitch) + ", " + std::to_string(expr->Yaw) + ", " + std::to_string(expr->Roll) + " }");
}

void PrintPrettyExpression::Expr(VectorConstExpression* expr)
{
	writeOutput("{ " + std::to_string(expr->X) + ", " + std::to_string(expr->Y) + ", " + std::to_string(expr->Z) + " }");
}

void PrintPrettyExpression::Expr(ByteConstExpression* expr)
{
	writeOutput(std::to_string(expr->Value));
}

void PrintPrettyExpression::Expr(IntZeroExpression* expr)
{
	writeOutput("0");
}

void PrintPrettyExpression::Expr(IntOneExpression* expr)
{
	writeOutput("1");
}

void PrintPrettyExpression::Expr(TrueExpression* expr)
{
	writeOutput("true");
}

void PrintPrettyExpression::Expr(FalseExpression* expr)
{
	writeOutput("false");
}

void PrintPrettyExpression::Expr(NativeParmExpression* expr)
{
	if (expr->Object)
		writeOutput(expr->Object->Name.ToString());
	else
		writeOutput("(null)");
}

void PrintPrettyExpression::Expr(NoObjectExpression* expr)
{
	writeOutput("null");
}

void PrintPrettyExpression::Expr(Unknown0x2bExpression* expr)
{
	writeOutput("unknown(0x2b, ");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(IntConstByteExpression* expr)
{
	writeOutput(std::to_string(expr->Value));
}

void PrintPrettyExpression::Expr(BoolVariableExpression* expr)
{
	Print(writeOutput, expr->Variable);
}

void PrintPrettyExpression::Expr(DynamicCastExpression* expr)
{
	writeOutput("dynamic cast(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(IteratorExpression* expr)
{
	writeOutput("iterator(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(IteratorPopExpression* expr)
{
	writeOutput("iterator pop");
}

void PrintPrettyExpression::Expr(IteratorNextExpression* expr)
{
	writeOutput("iterator next");
}

void PrintPrettyExpression::Expr(StructCmpEqExpression* expr)
{
	Print(writeOutput, expr->Value1);
	writeOutput(" == ");
	Print(writeOutput, expr->Value2);
}

void PrintPrettyExpression::Expr(StructCmpNeExpression* expr)
{
	Print(writeOutput, expr->Value1);
	writeOutput(" != ");
	Print(writeOutput, expr->Value2);
}

void PrintPrettyExpression::Expr(UnicodeStringConstExpression* expr)
{
	writeOutput("unicode string");
}

void PrintPrettyExpression::Expr(StructMemberExpression* expr)
{
	Print(writeOutput, expr->Value);
	writeOutput(".");
	writeOutput(expr->Field->Name.ToString());
}

void PrintPrettyExpression::Expr(RotatorToVectorExpression* expr)
{
	writeOutput("rotatorToVector(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(ByteToIntExpression* expr)
{
	writeOutput("byteToInt(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(ByteToBoolExpression* expr)
{
	writeOutput("byteToBool(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(ByteToFloatExpression* expr)
{
	writeOutput("byteToFloat(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(IntToByteExpression* expr)
{
	writeOutput("intToByte(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(IntToBoolExpression* expr)
{
	writeOutput("intToBool(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(IntToFloatExpression* expr)
{
	writeOutput("intToFloat(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(BoolToByteExpression* expr)
{
	writeOutput("boolToByte(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(BoolToIntExpression* expr)
{
	writeOutput("boolToInt(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(BoolToFloatExpression* expr)
{
	writeOutput("boolToFloat(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(FloatToByteExpression* expr)
{
	writeOutput("floatToByte(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(FloatToIntExpression* expr)
{
	writeOutput("floatToInt(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(FloatToBoolExpression* expr)
{
	writeOutput("floatToBool(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(Unknown0x46Expression* expr)
{
	writeOutput("unknown(0x46, ");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(ObjectToBoolExpression* expr)
{
	writeOutput("objectToBool(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(NameToBoolExpression* expr)
{
	writeOutput("nameToBool(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(StringToByteExpression* expr)
{
	writeOutput("stringToByte(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(StringToIntExpression* expr)
{
	writeOutput("stringToInt(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(StringToBoolExpression* expr)
{
	writeOutput("stringToBool(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(StringToFloatExpression* expr)
{
	writeOutput("stringToFloat(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(StringToVectorExpression* expr)
{
	writeOutput("stringToVector(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(StringToRotatorExpression* expr)
{
	writeOutput("stringToRotator(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(VectorToBoolExpression* expr)
{
	writeOutput("vectorToBool(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(VectorToRotatorExpression* expr)
{
	writeOutput("vectorToRotator(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(RotatorToBoolExpression* expr)
{
	writeOutput("rotatorToBool(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(ByteToStringExpression* expr)
{
	writeOutput("byteToString(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(IntToStringExpression* expr)
{
	writeOutput("intToString(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(BoolToStringExpression* expr)
{
	writeOutput("boolToString(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(FloatToStringExpression* expr)
{
	writeOutput("floatToString(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(ObjectToStringExpression* expr)
{
	writeOutput("objectToString(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(NameToStringExpression* expr)
{
	writeOutput("nameToString(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(VectorToStringExpression* expr)
{
	writeOutput("vectorToString(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(RotatorToStringExpression* expr)
{
	writeOutput("rotatorToString(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(StringToNameExpression* expr)
{
	writeOutput("stringToName(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(DynArrayToIntExpression* expr)
{
	writeOutput("dynarrayToInt(");
	Print(writeOutput, expr->Value);
	writeOutput(")");
}

void PrintPrettyExpression::Expr(VirtualFunctionExpression* expr)
{
	writeOutput(expr->Name.ToString() + "(");
	int index = 0;
	for (auto arg : expr->Args)
	{
		if (index != 0)
			writeOutput(", ");
		Print(writeOutput, arg);
		index++;
	}
	writeOutput(")");
}

void PrintPrettyExpression::Expr(FinalFunctionExpression* expr)
{
	writeOutput(expr->Func->Name.ToString() + "(");
	int index = 0;
	for (auto arg : expr->Args)
	{
		if (index != 0)
			writeOutput(", ");
		Print(writeOutput, arg);
		index++;
	}
	writeOutput(")");
}

void PrintPrettyExpression::Expr(GlobalFunctionExpression* expr)
{
	writeOutput(expr->Name.ToString() + "(");
	int index = 0;
	for (auto arg : expr->Args)
	{
		if (index != 0)
			writeOutput(", ");
		Print(writeOutput, arg);
		index++;
	}
	writeOutput(")");
}

void PrintPrettyExpression::Expr(NativeFunctionExpression* expr)
{
	if (NativeFunctions::FuncByIndex[expr->nativeindex])
		writeOutput(NativeFunctions::FuncByIndex[expr->nativeindex]->Name.ToString() + "(");
	else
		writeOutput("UnknownNative" + std::to_string(expr->nativeindex) + "(");
	int index = 0;
	for (auto arg : expr->Args)
	{
		if (index != 0)
			writeOutput(", ");
		Print(writeOutput, arg);
		index++;
	}
	writeOutput(")");
}

void PrintPrettyExpression::Expr(FunctionArgumentsExpression* expr)
{
	writeOutput("function arguments");
}

void PrintPrettyExpression::Expr(ConstructExpression* expr)
{
	writeOutput("construct " + expr->Struct->Name.ToString() + "(");
	int index = 0;
	for (auto arg : expr->Args)
	{
		if (index != 0)
			writeOutput(", ");
		if (arg.Name)
			writeOutput(arg.Name->Name.ToString() + ": ");
		Print(writeOutput, arg.Value);
		index++;
	}
	writeOutput(")");
}
