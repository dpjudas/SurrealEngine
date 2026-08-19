
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

class StatementPrinter : public AstStatementVisitor
{
public:
	StatementPrinter(Compiler* compiler, int depth) : compiler(compiler), depth(depth)
	{
	}

	void statement(AstLabeledStatement* node) override
	{
		// logInfo(node, "Label");
	}

	void statement(AstConstantDeclarationStatement* node) override
	{
		// logInfo(node, "ConstantDecl");
	}

	void statement(AstVariableDeclarationStatement* node) override
	{
		// logInfo(node, "VariableDecl");
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
		logInfo(node, "Empty");
	}

	void statement(AstExpressionStatement* node) override
	{
		logInfo(node, "Expression");
	}

	void statement(AstIfStatement* node) override
	{
		logInfo(node, "Jump If Not");
		if (node->then_statement)
		{
			node->then_statement->visit(this);
			if (node->else_statement)
			{
				logInfo(node, "Jump"); // jump past else
			}
		}
		if (node->else_statement)
		{
			node->else_statement->visit(this);
		}
	}

	void statement(AstSwitchStatement* node) override
	{
		logInfo(node, "Switch");
	}

	void statement(AstWhileStatement* node) override
	{
		logInfo(node, "Jump If Not");
		if (node->statement)
			node->statement->visit(this);
		logInfo(node, "Jump");
	}

	void statement(AstDoStatement* node) override
	{
		logInfo(node, "do");
	}

	void statement(AstForStatement* node) override
	{
		logInfo(node, "for");
	}

	void statement(AstForeachStatement* node) override
	{
		logInfo(node, "Iterator");
		if (node->statement)
		{
			node->statement->visit(this);
		}
		logInfo(node, "Iterator next");
		logInfo(node, "Iterator pop");
	}

	void statement(AstBreakStatement* node) override
	{
		logInfo(node, "Jump");
	}

	void statement(AstContinueStatement* node) override
	{
		logInfo(node, "Jump");
	}

	void statement(AstGotoStatement* node) override
	{
		logInfo(node, "Jump");
	}

	void statement(AstReturnStatement* node) override
	{
		logInfo(node, "Return");
	}

private:
	void logInfo(AstStatement* node, const std::string& text)
	{
		std::string spaces;
		spaces.resize(depth, '\t');
		compiler->logInfo("line " + std::to_string(node->line) + ": " + spaces + text);
	}

	Compiler* compiler = nullptr;
	int depth = 0;
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
			for (AstNode* member : unit->class_decl->members)
			{
				if (auto method = dynamic_cast<AstMethodDeclaration*>(member))
				{
					if (unit->class_decl->identifier == "Teleporter" && method->identifier == "Touch")
					{
						logInfo(unit->class_decl->identifier + "." + method->identifier);

						StatementPrinter printer(this, 1);
						if (method->block)
						{
							for (AstStatement* statement : method->block->statements)
							{
								statement->visit(&printer);
							}
						}
						return true;
					}
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
