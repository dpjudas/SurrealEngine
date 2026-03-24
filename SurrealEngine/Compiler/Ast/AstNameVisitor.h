
#pragma once

class AstClassDeclaration;
class AstStructDeclaration;
class AstEnumDeclaration;
class AstEnumValueDeclaration;
class AstConstantDeclaration;
class AstFieldDeclaration;
class AstMethodDeclaration;
class AstOperatorDeclaration;
class AstStateDeclaration;

class AstNameVisitor
{
public:
	virtual void name(AstClassDeclaration *node) = 0;
	virtual void name(AstStructDeclaration *node) = 0;
	virtual void name(AstEnumDeclaration *node) = 0;
	virtual void name(AstEnumValueDeclaration *node) = 0;

	virtual void name(AstConstantDeclaration *node) = 0;
	virtual void name(AstFieldDeclaration *node) = 0;
	virtual void name(AstMethodDeclaration *node) = 0;
	virtual void name(AstOperatorDeclaration *node) = 0;
	virtual void name(AstStateDeclaration* node) = 0;
};
