
#include "Precomp.h"
#include "Bytecode.h"

Bytecode::Bytecode(const std::vector<uint8_t>& bytecode, Package* package)
{
	BytecodeStream stream(bytecode.data(), bytecode.size(), package);
	while (!stream.IsEnd())
	{
		Statements.push_back(ReadToken(&stream, 0));
		Statements.back()->StatementIndex = (int)Statements.size() - 1;
	}
}

Expression* Bytecode::ReadToken(BytecodeStream* stream, int depth)
{
	if (depth == 64)
		throw std::runtime_error("Bytecode parsing error");
	depth++;

	uint16_t exproffset = stream->GetOffset();
	ExprToken token = stream->ReadToken();

	if (token >= ExprToken::FirstNative)
	{
		NativeFunctionExpression* expr = Create<NativeFunctionExpression>(exproffset);
		expr->nativeindex = (int)token;
		while (stream->PeekToken() != ExprToken::EndFunctionParms)
		{
			expr->Args.push_back(ReadToken(stream, depth));
		}
		stream->ReadToken();
		return expr;
	}
	else if (token >= ExprToken::ExtendedNative)
	{
		int part2 = stream->ReadUInt8();
		NativeFunctionExpression* expr = Create<NativeFunctionExpression>(exproffset);
		expr->nativeindex = (((int)token - (int)ExprToken::ExtendedNative) << 8) + part2;
		while (stream->PeekToken() != ExprToken::EndFunctionParms)
		{
			expr->Args.push_back(ReadToken(stream, depth));
		}
		stream->ReadToken();
		return expr;
	}
	else if (token == ExprToken::VirtualFunction)
	{
		VirtualFunctionExpression* expr = Create<VirtualFunctionExpression>(exproffset);
		expr->Name = stream->ReadName();
		while (stream->PeekToken() != ExprToken::EndFunctionParms)
		{
			expr->Args.push_back(ReadToken(stream, depth));
		}
		stream->ReadToken();
		return expr;
	}
	else if (token == ExprToken::FinalFunction)
	{
		FinalFunctionExpression* expr = Create<FinalFunctionExpression>(exproffset);
		expr->Func = stream->ReadObject<UFunction>();
		while (stream->PeekToken() != ExprToken::EndFunctionParms)
		{
			expr->Args.push_back(ReadToken(stream, depth));
		}
		stream->ReadToken();
		return expr;
	}
	else if (token == ExprToken::GlobalFunction)
	{
		GlobalFunctionExpression* expr = Create<GlobalFunctionExpression>(exproffset);
		expr->Name = stream->ReadName();
		while (stream->PeekToken() != ExprToken::EndFunctionParms)
		{
			expr->Args.push_back(ReadToken(stream, depth));
		}
		stream->ReadToken();
		return expr;
	}
	else if (token == ExprToken::LetBool && stream->GetVersion() <= 63)
	{
		FunctionArgumentsExpression* expr = Create<FunctionArgumentsExpression>(exproffset);
		while (true)
		{
			FunctionArgInfo info;
			info.size = stream->ReadUInt8();
			if (info.size == 0)
				break;
			info.flags = stream->ReadUInt8();
			expr->args.push_back(info);
		}
		return expr;
	}
	else
	{
		switch (token)
		{
		case ExprToken::LocalVariable:
		{
			LocalVariableExpression* expr = Create<LocalVariableExpression>(exproffset);
			expr->Variable = stream->ReadObject<UProperty>();
			return expr;
		}
		case ExprToken::InstanceVariable:
		{
			InstanceVariableExpression* expr = Create<InstanceVariableExpression>(exproffset);
			expr->Variable = stream->ReadObject<UProperty>();
			return expr;
		}
		case ExprToken::DefaultVariable:
		{
			DefaultVariableExpression* expr = Create<DefaultVariableExpression>(exproffset);
			expr->Variable = stream->ReadObject<UProperty>();
			return expr;
		}
		case ExprToken::Return:
		{
			ReturnExpression* expr = Create<ReturnExpression>(exproffset);
			if (stream->GetVersion() > 61)
				expr->Value = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::Switch:
		{
			SwitchExpression* expr = Create<SwitchExpression>(exproffset);
			expr->Size = static_cast<int>(stream->ReadUInt8()) + 1;
			expr->Condition = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::Jump:
		{
			JumpExpression* expr = Create<JumpExpression>(exproffset);
			expr->Offset = stream->ReadUInt16();
			return expr;
		}
		case ExprToken::JumpIfNot:
		{
			JumpIfNotExpression* expr = Create<JumpIfNotExpression>(exproffset);
			expr->Offset = stream->ReadUInt16();
			expr->Condition = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::Assert:
		{
			AssertExpression* expr = Create<AssertExpression>(exproffset);
			expr->Line = stream->ReadUInt16();
			expr->Condition = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::Case:
		{
			CaseExpression* expr = Create<CaseExpression>(exproffset);
			expr->NextOffset = stream->ReadUInt16();
			if (expr->NextOffset != 0xffff)
				expr->Value = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::LabelTable:
		{
			LabelTableExpression* expr = Create<LabelTableExpression>(exproffset);
			while (true)
			{
				LabelEntry entry;
				entry.Name = stream->ReadName();
				entry.Offset = stream->ReadUInt32();
				if (entry.Name == "None") break;
				expr->Labels.push_back(entry);
			}
			return expr;
		}
		case ExprToken::GotoLabel:
		{
			GotoLabelExpression* expr = Create<GotoLabelExpression>(exproffset);
			expr->Value = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::EatString:
		{
			EatStringExpression* expr = Create<EatStringExpression>(exproffset);
			expr->Value = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::Let:
		{
			LetExpression* expr = Create<LetExpression>(exproffset);
			expr->LeftSide = ReadToken(stream, depth);
			expr->RightSide = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::DynArrayElement:
		{
			DynArrayElementExpression* expr = Create<DynArrayElementExpression>(exproffset);
			expr->Index = ReadToken(stream, depth);
			expr->Array = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::New:
		{
			NewExpression* expr = Create<NewExpression>(exproffset);
			expr->ParentExpr = ReadToken(stream, depth);
			expr->NameExpr = ReadToken(stream, depth);
			expr->FlagsExpr = ReadToken(stream, depth);
			expr->ClassExpr = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::ClassContext:
		{
			ClassContextExpression* expr = Create<ClassContextExpression>(exproffset);
			expr->ObjectExpr = ReadToken(stream, depth);
			expr->NullExprCodeOffset = stream->ReadUInt16();
			expr->ZeroFillSize = stream->ReadUInt8();
			expr->ContextExpr = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::MetaCast:
		{
			MetaCastExpression* expr = Create<MetaCastExpression>(exproffset);
			expr->Class = stream->ReadObject<UClass>();
			expr->Value = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::LetBool:
		{
			LetBoolExpression* expr = Create<LetBoolExpression>(exproffset);
			expr->LeftSide = ReadToken(stream, depth);
			expr->RightSide = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::Unknown0x15:
		{
			Unknown0x15Expression* expr = Create<Unknown0x15Expression>(exproffset);
			//expr->Value = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::Skip:
		{
			SkipExpression* expr = Create<SkipExpression>(exproffset);
			expr->Skip = stream->ReadUInt16();
			expr->Value = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::Context:
		{
			ContextExpression* expr = Create<ContextExpression>(exproffset);
			expr->ObjectExpr = ReadToken(stream, depth);
			expr->NullExprCodeOffset = stream->ReadUInt16();
			expr->ZeroFillSize = stream->ReadUInt8();
			expr->ContextExpr = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::ArrayElement:
		{
			ArrayElementExpression* expr = Create<ArrayElementExpression>(exproffset);
			expr->Index = ReadToken(stream, depth);
			expr->Array = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::IntConst:
		{
			IntConstExpression* expr = Create<IntConstExpression>(exproffset);
			expr->Value = stream->ReadUInt32();
			return expr;
		}
		case ExprToken::FloatConst:
		{
			FloatConstExpression* expr = Create<FloatConstExpression>(exproffset);
			expr->Value = stream->ReadFloat();
			return expr;
		}
		case ExprToken::StringConst:
		{
			StringConstExpression* expr = Create<StringConstExpression>(exproffset);
			expr->Value = stream->ReadAsciiZ();
			return expr;
		}
		case ExprToken::ObjectConst:
		{
			ObjectConstExpression* expr = Create<ObjectConstExpression>(exproffset);
			expr->Object = stream->ReadObject<UObject>();
			return expr;
		}
		case ExprToken::NameConst:
		{
			NameConstExpression* expr = Create<NameConstExpression>(exproffset);
			expr->Value = stream->ReadName();
			return expr;
		}
		case ExprToken::RotationConst:
		{
			RotationConstExpression* expr = Create<RotationConstExpression>(exproffset);
			expr->Pitch = stream->ReadUInt32();
			expr->Yaw = stream->ReadUInt32();
			expr->Roll = stream->ReadUInt32();
			return expr;
		}
		case ExprToken::VectorConst:
		{
			VectorConstExpression* expr = Create<VectorConstExpression>(exproffset);
			expr->X = stream->ReadFloat();
			expr->Y = stream->ReadFloat();
			expr->Z = stream->ReadFloat();
			return expr;
		}
		case ExprToken::ByteConst:
		{
			ByteConstExpression* expr = Create<ByteConstExpression>(exproffset);
			expr->Value = stream->ReadUInt8();
			return expr;
		}
		case ExprToken::NativeParm:
		{
			NativeParmExpression* expr = Create<NativeParmExpression>(exproffset);
			expr->Object = stream->ReadObject<UObject>();
			return expr;
		}
		case ExprToken::Unknown0x2b:
		{
			Unknown0x2bExpression* expr = Create<Unknown0x2bExpression>(exproffset);
			expr->Unknown = stream->ReadUInt8();
			expr->Value = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::IntConstByte:
		{
			IntConstByteExpression* expr = Create<IntConstByteExpression>(exproffset);
			expr->Value = stream->ReadUInt8();
			return expr;
		}
		case ExprToken::DynamicCast:
		{
			DynamicCastExpression* expr = Create<DynamicCastExpression>(exproffset);
			expr->Class = stream->ReadObject<UClass>();
			expr->Value = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::Iterator:
		{
			IteratorExpression* expr = Create<IteratorExpression>(exproffset);
			expr->Value = ReadToken(stream, depth);
			expr->Offset = stream->ReadUInt16();
			return expr;
		}
		case ExprToken::StructCmpEq:
		{
			StructCmpEqExpression* expr = Create<StructCmpEqExpression>(exproffset);
			expr->Struct = stream->ReadObject<UObject>();
			expr->Value1 = ReadToken(stream, depth);
			expr->Value2 = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::StructCmpNe:
		{
			StructCmpNeExpression* expr = Create<StructCmpNeExpression>(exproffset);
			expr->Struct = stream->ReadObject<UObject>();
			expr->Value1 = ReadToken(stream, depth);
			expr->Value2 = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::UnicodeStringConst:
		{
			UnicodeStringConstExpression* expr = Create<UnicodeStringConstExpression>(exproffset);
			expr->Value = stream->ReadUnicodeZ();
			return expr;
		}
		case ExprToken::StructMember:
		{
			StructMemberExpression* expr = Create<StructMemberExpression>(exproffset);
			expr->Field = stream->ReadObject<UProperty>();
			expr->Value = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::Stop: { return Create<StopExpression>(exproffset); }
		case ExprToken::Nothing: { return Create<NothingExpression>(exproffset); }
		case ExprToken::Self: { return Create<SelfExpression>(exproffset); }
		case ExprToken::IntZero: { return Create<IntZeroExpression>(exproffset); }
		case ExprToken::IntOne: { return Create<IntOneExpression>(exproffset); }
		case ExprToken::True: { return Create<TrueExpression>(exproffset); }
		case ExprToken::False: { return Create<FalseExpression>(exproffset); }
		case ExprToken::NoObject: { return Create<NoObjectExpression>(exproffset); }
		case ExprToken::BoolVariable:
		{
			BoolVariableExpression* expr = Create<BoolVariableExpression>(exproffset);
			expr->Variable = ReadToken(stream, depth);
			return expr;
		}
		case ExprToken::IteratorPop: { return Create<IteratorPopExpression>(exproffset); }
		case ExprToken::IteratorNext: { return Create<IteratorNextExpression>(exproffset); }
		case ExprToken::RotatorToVector: { auto expr = Create<RotatorToVectorExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::ByteToInt: { auto expr = Create<ByteToIntExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::ByteToBool: { auto expr = Create<ByteToBoolExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::ByteToFloat: { auto expr = Create<ByteToFloatExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::IntToByte: { auto expr = Create<IntToByteExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::IntToBool: { auto expr = Create<IntToBoolExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::IntToFloat: { auto expr = Create<IntToFloatExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::BoolToByte: { auto expr = Create<BoolToByteExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::BoolToInt: { auto expr = Create<BoolToIntExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::BoolToFloat: { auto expr = Create<BoolToFloatExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::FloatToByte: { auto expr = Create<FloatToByteExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::FloatToInt: { auto expr = Create<FloatToIntExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::FloatToBool: { auto expr = Create<FloatToBoolExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::Unknown0x46: { auto expr = Create<Unknown0x46Expression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::ObjectToBool: { auto expr = Create<ObjectToBoolExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::NameToBool: { auto expr = Create<NameToBoolExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::StringToByte: { auto expr = Create<StringToByteExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::StringToInt: { auto expr = Create<StringToIntExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::StringToBool: { auto expr = Create<StringToBoolExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::StringToFloat: { auto expr = Create<StringToFloatExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::StringToVector: { auto expr = Create<StringToVectorExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::StringToRotator: { auto expr = Create<StringToRotatorExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::VectorToBool: { auto expr = Create<VectorToBoolExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::VectorToRotator: { auto expr = Create<VectorToRotatorExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::RotatorToBool: { auto expr = Create<RotatorToBoolExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::ByteToString: { auto expr = Create<ByteToStringExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::IntToString: { auto expr = Create<IntToStringExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::BoolToString: { auto expr = Create<BoolToStringExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::FloatToString: { auto expr = Create<FloatToStringExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::ObjectToString: { auto expr = Create<ObjectToStringExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::NameToString: { auto expr = Create<NameToStringExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::VectorToString: { auto expr = Create<VectorToStringExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		case ExprToken::RotatorToString: { auto expr = Create<RotatorToStringExpression>(exproffset); expr->Value = ReadToken(stream, depth); return expr; }
		default:
			throw std::runtime_error("Unknown script bytecode token encountered");
		}
	}
}
