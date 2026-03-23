
#pragma once

class EnumValueTypeMember;
class ConstantTypeMember;
class FieldTypeMember;
class MethodTypeMember;
class OperatorTypeMember;

class TypeMemberVisitor
{
public:
	virtual void member(EnumValueTypeMember *member) = 0;
	virtual void member(ConstantTypeMember *member) = 0;
	virtual void member(FieldTypeMember *member) = 0;
	virtual void member(MethodTypeMember *member) = 0;
	virtual void member(OperatorTypeMember *member) = 0;
};
