
#pragma once

#include "AstNode.h"
#include "AstStatementVisitor.h"
#include <vector>

class AstExpression;
class AstArrayInitializer;

class AstStatement : public AstNode
{
public:
	virtual void visit(AstStatementVisitor *visitor) = 0;
};

class AstLabeledStatement : public AstStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }

	std::string identifier;
	//AstStatement *statement = nullptr;
};

class AstDeclarationStatement : public AstStatement
{
public:
};

class AstLocalConstantDeclarator : public AstNode
{
public:
	std::string identifier;
	AstExpression *expression = nullptr;
};

class AstConstantDeclarationStatement : public AstDeclarationStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }

	AstName *type = nullptr;
	std::vector<AstLocalConstantDeclarator *> variables;
};

class AstLocalVariableDeclarator : public AstNode
{
public:
	std::string identifier;
	int array_dimension = 0;
	AstExpression *expression = nullptr;
	AstArrayInitializer *array_initializer = nullptr;
};

class AstLocalVariableDeclaration : public AstNode
{
public:
	AstName *type = nullptr;
	std::vector<AstLocalVariableDeclarator *> variables;
};

class AstVariableDeclarationStatement : public AstDeclarationStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }

	AstLocalVariableDeclaration *variable_declaration = nullptr;
};

class AstEmbeddedStatement : public AstStatement
{
public:
};

class AstIgnoreEventsDeclaration : public AstNode
{
public:
	std::vector<std::string> events;
};

class AstBlockStatement : public AstEmbeddedStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }
	void visit_children(AstStatementVisitor *visitor) { for (size_t i = 0; i < statements.size(); i++) statements[i]->visit(visitor); }

	std::vector<AstStatement *> statements;
	std::vector<AstMethodDeclaration*> methods;
	std::vector<AstIgnoreEventsDeclaration*> ignores;
};

class AstEmptyStatement : public AstEmbeddedStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }
};

class AstExpressionStatement : public AstEmbeddedStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }

	// Allowed types:
	// invocation-expression
	// object-creation-expression
	// assignment
	// post-increment-expression
	// post-decrement-expression
	// pre-increment-expression
	// pre-decrement-expression
	AstExpression *expression = nullptr;
};

class AstSelectionStatement : public AstEmbeddedStatement
{
public:
};

class AstIfStatement : public AstSelectionStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }

	AstExpression *boolean_expression = nullptr; // boolean-expression only
	AstStatement *then_statement = nullptr; // embedded statement only
	AstStatement *else_statement = nullptr; // embedded statement only
};

class AstSwitchLabel : public AstNode
{
public:
	AstExpression *constant_expression = nullptr; // null if it is a 'default' label
};

class AstSwitchSection : public AstNode
{
public:
	std::vector<AstSwitchLabel *> labels;
	std::vector<AstStatement *> statements;
};

class AstSwitchStatement : public AstSelectionStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }

	AstExpression *expression = nullptr;
	std::vector<AstSwitchSection *> sections;
};

class AstIterationStatement : public AstEmbeddedStatement
{
public:
};

class AstWhileStatement : public AstIterationStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }

	AstExpression *boolean_expression = nullptr;
	AstStatement *statement = nullptr; // embedded-statement only
};

class AstDoStatement : public AstIterationStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }

	AstStatement *statement = nullptr; // embedded-statement only
	AstExpression *boolean_expression = nullptr;
	bool is_until = false;
};

class AstForStatement : public AstIterationStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }

	AstLocalVariableDeclaration *init_variable_declaration = nullptr;
	std::vector<AstExpression *> init_expressions; // statement-expression only
	AstExpression *condition = nullptr; // boolean-expression only
	std::vector<AstExpression *> iterator_expressions; // statement-expression only
	AstStatement *statement = nullptr; // embedded-statement only
};

class AstForeachStatement : public AstIterationStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }

	AstExpression *expression = nullptr;
	AstStatement *statement = nullptr; // embedded-statement only
};

class AstJumpStatement : public AstEmbeddedStatement
{
public:
};

class AstBreakStatement : public AstJumpStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }
};

class AstContinueStatement : public AstJumpStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }
};

class AstGotoStatement : public AstJumpStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }

	std::string name;
};

class AstReturnStatement : public AstJumpStatement
{
public:
	void visit(AstStatementVisitor *visitor) { visitor->statement(this); }

	AstExpression *expression = nullptr;
};
