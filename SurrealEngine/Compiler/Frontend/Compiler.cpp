
#include "Precomp.h"
#include "Compiler.h"
#include "CompilerException.h"
#include "Compiler/Lex/Tokenizer.h"
#include "Compiler/Ast/Ast.h"
#include "Compiler/Parse/Parser.h"
#include "Compiler/Sema/SemanticAnalysis.h"

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

		try
		{
			SemanticAnalysis sema(type_system);
			sema.analyze(parsed_files);
		}
		catch (SemaException& exception)
		{
			messages.push_back(CompilerMessage(CompilerMessage::error, exception.message(), "unknown.uc"/*sources[exception.sourceIndex].filename*/, exception.line));
			return false;
		}

		codegen = std::make_unique<CodeGen>(type_system);
		codegen->codegen(parsed_files);

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

std::unique_ptr<ClassDebugInfo> Compiler::move_debug_info(int index)
{
	return std::move(codegen->sources_debug_info[index]);
}
