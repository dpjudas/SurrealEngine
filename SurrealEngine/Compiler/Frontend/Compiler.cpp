
#include "Precomp.h"
#include "Compiler.h"
#include "CompilerException.h"
#include "Compiler/Lex/Tokenizer.h"
#include "Compiler/Ast/Ast.h"
#include "Compiler/Parse/Parser.h"
//#include "Compiler/Sema/SemanticAnalysis.h"

Compiler::Compiler()
{
}

Compiler::~Compiler()
{
}

void Compiler::add_code(const std::string &code, const std::string &filename)
{
	sources.push_back(SourceFile(code, filename));
}

class StatementDebugInfoBuilder : public AstStatementVisitor
{
public:
	StatementDebugInfoBuilder(FunctionDebugInfo* funcDebugInfo) : funcDebugInfo(funcDebugInfo)
	{
	}

	void statement(AstLabeledStatement* node) override
	{
	}

	void statement(AstConstantDeclarationStatement* node) override
	{
	}

	void statement(AstVariableDeclarationStatement* node) override
	{
		// To do: does unrealscript have initializers in the variable declarations?
	}

	void statement(AstBlockStatement* node) override
	{
		for (AstStatement* statement : node->statements)
		{
			statement->visit(this);
		}
	}

	void statement(AstEmptyStatement* node) override
	{
	}

	void statement(AstExpressionStatement* node) override
	{
		funcDebugInfo->Statements.push_back({ ExprToken::Skip, node->line });
	}

	void statement(AstIfStatement* node) override
	{
		funcDebugInfo->Statements.push_back({ ExprToken::JumpIfNot, node->line });
		if (node->then_statement)
		{
			node->then_statement->visit(this);
			if (node->else_statement)
			{
				funcDebugInfo->Statements.push_back({ ExprToken::Jump, node->line });
			}
		}
		if (node->else_statement)
		{
			node->else_statement->visit(this);
		}
	}

	void statement(AstSwitchStatement* node) override
	{
		funcDebugInfo->Statements.push_back({ ExprToken::Switch, node->line });
		// What happens to ExprToken::Case? are they children?
	}

	void statement(AstWhileStatement* node) override
	{
		funcDebugInfo->Statements.push_back({ ExprToken::JumpIfNot, node->line });
		if (node->statement)
			node->statement->visit(this);
		funcDebugInfo->Statements.push_back({ ExprToken::Jump, node->line });
	}

	void statement(AstDoStatement* node) override
	{
		if (node->statement)
			node->statement->visit(this);
		funcDebugInfo->Statements.push_back({ ExprToken::JumpIfNot, node->line });
	}

	void statement(AstForStatement* node) override
	{
		// logInfo(node, "for");
	}

	void statement(AstForeachStatement* node) override
	{
		funcDebugInfo->Statements.push_back({ ExprToken::Iterator, node->line });
		if (node->statement)
		{
			node->statement->visit(this);
		}
		funcDebugInfo->Statements.push_back({ ExprToken::IteratorNext, node->line });
		funcDebugInfo->Statements.push_back({ ExprToken::IteratorPop, node->line });
	}

	void statement(AstBreakStatement* node) override
	{
		funcDebugInfo->Statements.push_back({ ExprToken::Jump, node->line });
	}

	void statement(AstContinueStatement* node) override
	{
		funcDebugInfo->Statements.push_back({ ExprToken::Jump, node->line });
	}

	void statement(AstGotoStatement* node) override
	{
		funcDebugInfo->Statements.push_back({ ExprToken::Jump, node->line });
	}

	void statement(AstReturnStatement* node) override
	{
		funcDebugInfo->Statements.push_back({ ExprToken::Return, node->line });
	}

private:
	FunctionDebugInfo* funcDebugInfo = nullptr;
};

bool Compiler::compile()
{
	try
	{
		std::vector<std::shared_ptr<AstCompilationUnit> > parsed_files;

		bool encountered_errors = false;
		for (size_t i = 0; i < sources.size(); i++)
		{
			try
			{
				Parser parser(sources[i].code);
				std::shared_ptr<AstCompilationUnit> ast = parser.parse();
				parsed_files.push_back(ast);
			}
			catch (ParseException &exception)
			{
				messages.push_back(CompilerMessage(CompilerMessage::error, exception.message(), sources[i].filename, exception.line));
				encountered_errors = true;
			}
		}

		if (encountered_errors)
			return false;

		for (auto& unit : parsed_files)
		{
			auto debug_info = std::make_unique<ClassDebugInfo>();
			debug_info->Name = unit->class_decl->identifier;

			for (AstNode* member : unit->class_decl->members)
			{
				if (auto method = dynamic_cast<AstMethodDeclaration*>(member))
				{
					FunctionDebugInfo funcDebugInfo;

					StatementDebugInfoBuilder builder(&funcDebugInfo);
					if (method->block)
					{
						for (AstStatement* statement : method->block->statements)
						{
							statement->visit(&builder);
						}
					}

					if (funcDebugInfo.Statements.empty() || funcDebugInfo.Statements.back().Token != ExprToken::Return)
						funcDebugInfo.Statements.push_back({ ExprToken::Return, method->line });

					debug_info->Functions[method->identifier] = std::move(funcDebugInfo);
				}
				else if (auto field = dynamic_cast<AstFieldDeclaration*>(member))
				{
					/*for (AstVariableDeclarator* var : field->declarators)
					{
						logInfo("Property: " + var->identifier);
					}*/
				}
				if (auto struct_decl = dynamic_cast<AstStructDeclaration*>(member))
				{
					//logInfo("Struct: " + struct_decl->identifier);
				}
				if (auto enum_decl = dynamic_cast<AstEnumDeclaration*>(member))
				{
					//logInfo("Enum: " + enum_decl->identifier);
				}
				if (auto state_decl = dynamic_cast<AstStateDeclaration*>(member))
				{
					//logInfo("State: " + state_decl->identifier);
				}
			}

			sources_debug_info.push_back(std::move(debug_info));
		}

#if 0
		SemanticAnalysis sema(type_system);
		sema.analyze(parsed_files);

		codegen.reset(new CodeGen(type_system));
		codegen->codegen(parsed_files, externBindings);
#endif

		return true;
	}
	catch (CompilerException &exception)
	{
		messages.push_back(CompilerMessage(CompilerMessage::error, exception.message()));
		return false;
	}
}

void Compiler::logInfo(const std::string& text)
{
	messages.push_back(CompilerMessage(CompilerMessage::info, text));
}
