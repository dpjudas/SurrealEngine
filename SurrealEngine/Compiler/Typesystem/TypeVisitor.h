
#pragma once

class VoidType;
class NativeHandleType;
class ClassType;
class ArrayType;
class NullType;
class BoxedType;
class SByteType;
class ByteType;
class Int16Type;
class UInt16Type;
class Int32Type;
class UInt32Type;
class Int64Type;
class UInt64Type;
class CharType;
class SingleType;
class DoubleType;
class BooleanType;
class StructType;
class EnumType;
class OpenClassType;
class OpenStructType;

class TypeVisitor
{
public:
	virtual void type(VoidType *type) = 0;
	virtual void type(NativeHandleType *type) = 0;
	virtual void type(ClassType *type) = 0;
	virtual void type(ArrayType *type) = 0;
	virtual void type(NullType *type) = 0;
	virtual void type(BoxedType *type) = 0;
	virtual void type(SByteType *type) = 0;
	virtual void type(ByteType *type) = 0;
	virtual void type(Int16Type *type) = 0;
	virtual void type(UInt16Type *type) = 0;
	virtual void type(Int32Type *type) = 0;
	virtual void type(UInt32Type *type) = 0;
	virtual void type(Int64Type *type) = 0;
	virtual void type(UInt64Type *type) = 0;
	virtual void type(CharType *type) = 0;
	virtual void type(SingleType *type) = 0;
	virtual void type(DoubleType *type) = 0;
	virtual void type(BooleanType *type) = 0;
	virtual void type(StructType *type) = 0;
	virtual void type(EnumType *type) = 0;
	virtual void type(OpenClassType *type) = 0;
	virtual void type(OpenStructType *type) = 0;
};
