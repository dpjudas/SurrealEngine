
#pragma once

#include <memory>
#include <string>

#include "Compiler/Lex/Tokenizer.h"
#include "Compiler/Lex/Token.h"
#include "Compiler/Ast/Ast.h"
#include "Compiler/Frontend/CompilerException.h"

class ParseException : public CompilerException
{
public:
	ParseException(const std::string &message, int line, int column) : CompilerException(message), line(line), column(column) { }
	int line;
	int column;
};

class SavedParserPos
{
public:
	SavedTokenizerPos tokenpos;
	Token token;
};

class Parser
{
public:
	Parser(const std::string &source);

	std::shared_ptr<AstCompilationUnit> parse();

private:
	struct TypeModifierPreparseData
	{
		bool is_abstract = false;
		bool is_static = false;
		bool is_event = false;
		bool is_const = false;
		AstAccessType access_type = {};
	};

	template<typename T, typename... Types>
	T* newNode(Types&&... args)
	{
		ast->allocatedNodes.push_back(std::make_unique<T>(std::forward<Types>(args)...));
		T* node = static_cast<T*>(ast->allocatedNodes.back().get());
		node->line = token.line;
		node->column = token.column;
		return node;
	}

	TypeModifierPreparseData preparse_type();

	AstClassDeclaration *parse_class_declaration();
	AstStructDeclaration *parse_struct_declaration();
	AstEnumDeclaration *parse_enum_declaration();

	AstNode *parse_class_member();
	AstNode* parse_state_declaration();
	AstNode *parse_const_declaration();
	AstNode *parse_method_declaration();
	AstNode *parse_field_declaration();
	AstNode *parse_operator_declaration(const TypeModifierPreparseData &preparse, AstName *return_type);

	std::vector<AstMethodParameter *> parse_formal_parameter_list(const char *end_operator);

	AstStatement *parse_statement();
	AstBlockStatement *parse_block_statement();
	AstConstantDeclarationStatement *parse_constant_declaration_statement();
	AstVariableDeclarationStatement *parse_variable_declaration_statement(AstName *type);
	AstIfStatement *parse_if_statement();
	AstSwitchStatement *parse_switch_statement();
	AstWhileStatement *parse_while_statement();
	AstDoStatement *parse_do_statement();
	AstForStatement *parse_for_statement();
	AstForeachStatement *parse_foreach_statement();
	AstBreakStatement *parse_break_statement();
	AstContinueStatement *parse_continue_statement();
	AstGotoStatement *parse_goto_statement();
	AstReturnStatement *parse_return_statement();
	AstExpressionStatement *parse_expression_statement();

	AstLocalVariableDeclaration *parse_local_variable_declaration(AstName *type);

	AstArrayInitializer *parse_array_initializer();

	enum ExpressionEndConditionFlags
	{
		semicolon_end    = 0x01, // ;
		paranthesis_end  = 0x02, // )
		bracket_end      = 0x04, // ]
		colon_end        = 0x08, // :
		comma_end        = 0x10, // ,
		brace_end        = 0x20  // }
	};

	typedef int ExpressionEndCondition;

	AstExpression *parse_expression(ExpressionEndCondition end_condition);

	AstExpression *parse_primary();
	AstExpression *parse_unary();
	AstExpression *parse_binary(int lhs_precedence, AstExpression *lhs, ExpressionEndCondition end_condition);

	AstBinaryExpression *create_token_expression();
	int get_token_precedence();
	bool is_end_condition(ExpressionEndCondition end_condition) const;

	AstName *parse_name();
	AstIdentifierName *parse_identifier_name();

	AstTypeParameterList *try_parse_type_parameter_list();

	void set_line_info(AstNode* node);

	void skip_until_body();
	void skip_body();

	void next();
	bool is_type_keyword() const;
	bool is_keyword() const;
	bool is_keyword(const char *keyword) const;
	bool is_identifier() const;
	bool is_identifier(const char *identifier) const;
	bool is_operator(const char *oper) const;

	[[noreturn]] void throw_parse_exception(const std::string &message);

	SavedParserPos save_position();
	void restore_position(const SavedParserPos &pos);

	Tokenizer tokenizer;
	Token token;
	std::shared_ptr<AstCompilationUnit> ast;
};
