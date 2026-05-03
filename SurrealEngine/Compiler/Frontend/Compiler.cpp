
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

		/*
		for (auto& unit : parsed_files)
		{
			for (AstNode* member : unit->class_decl->members)
			{
				if (auto method = dynamic_cast<AstMethodDeclaration*>(member))
				{
					logInfo("Function: " + method->identifier);
				}
				else if (auto field = dynamic_cast<AstFieldDeclaration*>(member))
				{
					for (AstVariableDeclarator* var : field->declarators)
					{
						logInfo("Property: " + var->identifier);
					}
				}
				if (auto struct_decl = dynamic_cast<AstStructDeclaration*>(member))
				{
					logInfo("Struct: " + struct_decl->identifier);
				}
				if (auto enum_decl = dynamic_cast<AstEnumDeclaration*>(member))
				{
					logInfo("Enum: " + enum_decl->identifier);
				}
				if (auto state_decl = dynamic_cast<AstStateDeclaration*>(member))
				{
					logInfo("State: " + state_decl->identifier);
				}
			}
		}
		*/

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
