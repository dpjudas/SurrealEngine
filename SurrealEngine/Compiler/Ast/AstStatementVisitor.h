
#pragma once

class AstLabeledStatement;
class AstConstantDeclarationStatement;
class AstVariableDeclarationStatement;
class AstBlockStatement;
class AstEmptyStatement;
class AstExpressionStatement;
class AstIfStatement;
class AstSwitchStatement;
class AstWhileStatement;
class AstDoStatement;
class AstForStatement;
class AstForeachStatement;
class AstBreakStatement;
class AstContinueStatement;
class AstGotoStatement;
class AstReturnStatement;

class AstStatementVisitor
{
public:
	virtual void statement(AstLabeledStatement *node) = 0;
	virtual void statement(AstConstantDeclarationStatement *node) = 0;
	virtual void statement(AstVariableDeclarationStatement *node) = 0;
	virtual void statement(AstBlockStatement *node) = 0;
	virtual void statement(AstEmptyStatement *node) = 0;
	virtual void statement(AstExpressionStatement *node) = 0;
	virtual void statement(AstIfStatement *node) = 0;
	virtual void statement(AstSwitchStatement *node) = 0;
	virtual void statement(AstWhileStatement *node) = 0;
	virtual void statement(AstDoStatement *node) = 0;
	virtual void statement(AstForStatement *node) = 0;
	virtual void statement(AstForeachStatement *node) = 0;
	virtual void statement(AstBreakStatement *node) = 0;
	virtual void statement(AstContinueStatement *node) = 0;
	virtual void statement(AstGotoStatement *node) = 0;
	virtual void statement(AstReturnStatement *node) = 0;
};
