
#pragma once

#include "AstNode.h"
#include "AstExpressionVisitor.h"
#include "Compiler/Typesystem/TypeExpression.h"
#include <string>
#include <vector>

class AstName;
class AstKeywordType;
class AstBlockStatement;
class AstArrayInitializer;
class TypeSystem;
class TypeName;
class Type;
class MethodTypeMember;
class FunctionMember;

class AstExpression : public AstNode
{
public:
	virtual void visit(AstExpressionVisitor *visitor) = 0;

	ExpressionResult result;
};

class AstArrayCreationExpression : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstName *type = nullptr;
	bool has_expression_list = false;
	std::vector<AstExpression *> expression_list;
	std::vector<int> rank_specifiers;
	AstArrayInitializer *initializer = nullptr;
};

enum class AstLiteralType
{
	boolean,
	integer,
	real,
	character,
	string,
	null,
};

class AstLiteral : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstLiteralType type = AstLiteralType::null;
	std::string value;
};

class AstSimpleName : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	std::string identifier;
	//AstTypeArgumentList *type_args;
};

class AstInvocationArgument : public AstNode
{
public:
	AstInvocationArgument() : is_ref(), is_out(), expression() { }

	bool is_ref;
	bool is_out;
	AstExpression *expression;
};

class AstInvocationExpression : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstExpression *expression = nullptr;
	std::vector<AstInvocationArgument *> args;
};

class AstElementAccess : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstExpression *expression = nullptr;
	std::vector<AstExpression *> args;
};

class AstThisAccess : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstBaseAccess : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	std::string identifier;
	std::vector<std::string> args;
};

class AstMemberAccess : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstKeywordType *predefined_type = nullptr;
	AstExpression *expression = nullptr;
	std::string identifier;
};

class AstPostIncrementExpression : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstExpression *expression = nullptr;
};

class AstPostDecrementExpression : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstExpression *expression = nullptr;
};

class AstNewExpression : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstName *type = nullptr;
	std::vector<AstExpression *> args;
};

class AstTypeofExpression : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstName *type = nullptr;
};

class AstSizeofExpression : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstName *type = nullptr;
};

class AstCheckedExpression : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstExpression *expression = nullptr;
};

class AstUncheckedExpression : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstExpression *expression = nullptr;
};

class AstDefaultValueExpression : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstName *type = nullptr;
};

class AstParenthesizedExpression : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstExpression *expression = nullptr;
};

class AstAnonymousMethodParameter : public AstNode
{
public:
	// parameter-modifier
	std::string type;
	std::string identifier;
};

class AstAnonymousMethodExpression : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	std::vector<AstAnonymousMethodParameter *> parameters;
	AstBlockStatement *block = nullptr;
};

class AstUnaryExpression : public AstExpression
{
public:
	AstExpression *operand = nullptr;
};

class AstBinaryExpression : public AstExpression
{
public:
	AstExpression *operand1 = nullptr;
	AstExpression *operand2 = nullptr;
};

class AstTrinaryExpression : public AstExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstExpression *operand1 = nullptr;
	AstExpression *operand2 = nullptr;
	AstExpression *operand3 = nullptr;
};

class AstUnaryPlusExpression : public AstUnaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstUnaryMinusExpression : public AstUnaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstUnaryLogicalNotExpression : public AstUnaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstUnaryBitwiseComplementExpression : public AstUnaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstUnaryPreIncrementExpression : public AstUnaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstUnaryPreDecrementExpression : public AstUnaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstCastExpression : public AstUnaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	AstName *type = nullptr;
};

class AstMultiplicationExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstDivisionExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstRemainderExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstAdditionExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstSubtractionExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstShiftLeftExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstShiftRightExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstLessExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstGreaterExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstLessEqualExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstGreaterEqualExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstIsExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstAsExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstEqualExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstNotEqualExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstLogicalAndExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstLogicalOrExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstLogicalXorExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstConditionalAndExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstConditionalOrExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstNullCoalescingExpression : public AstBinaryExpression
{
public:
	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }
};

class AstAssignmentExpression : public AstBinaryExpression
{
public:
	AstAssignmentExpression() = default;
	AstAssignmentExpression(const std::string &assignment_type) : assignment_type(assignment_type) { }

	void visit(AstExpressionVisitor *visitor) { visitor->expression(this); }

	std::string assignment_type; // operator
};
