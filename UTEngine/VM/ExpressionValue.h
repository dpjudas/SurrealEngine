#pragma once

#include "UObject/UObject.h"

class UProperty;

enum class ExpressionValueType
{
	Nothing,
	Variable,
	ValueByte,
	ValueInt,
	ValueBool,
	ValueFloat,
	ValueObject,
	ValueVector,
	ValueRotator,
	ValueString,
	ValueName,
	ValueColor
};

class ExpressionValue
{
public:
	ExpressionValueType Type = ExpressionValueType::Nothing;
	union
	{
		uint8_t Byte;
		int32_t Int;
		bool Bool;
		float Float;
		UObject* Object;
		vec3 Vector;
		Rotator Rotator;
		Color Color;
	} Value;
	std::string ValueString;

	UObject* VariableObject = nullptr;
	UProperty* VariableProperty = nullptr;

	uint8_t ToByte() const;
	int32_t ToInt() const;
	bool ToBool() const;
	float ToFloat() const;
	UObject* ToObject() const;
	vec3 ToVector() const;
	Rotator ToRotator() const;
	std::string ToString() const;
	std::string ToName() const;

	template<typename T> T ToType();

	// Pass by value
	template<> uint8_t ToType() { return Value.Byte; }
	template<> int32_t ToType() { return Value.Int; }
	template<> bool ToType() { return Value.Bool; }
	template<> float ToType() { return Value.Float; }
	template<> UObject* ToType() { return Value.Object; }
	template<> const vec3& ToType() { return Value.Vector; }
	template<> const Rotator& ToType() { return Value.Rotator; }
	template<> const std::string& ToType() { return ValueString; }
	template<> const Color& ToType() { return Value.Color; }

	// Pass by reference (To do: this needs to use the variable)
	template<> uint8_t& ToType() { return Value.Byte; }
	template<> int32_t& ToType() { return Value.Int; }
	template<> bool& ToType() { return Value.Bool; }
	template<> float& ToType() { return Value.Float; }
	template<> UObject*& ToType() { return Value.Object; }
	template<> vec3& ToType() { return Value.Vector; }
	template<> Rotator& ToType() { return Value.Rotator; }
	template<> std::string& ToType() { return ValueString; }
	template<> Color& ToType() { return Value.Color; }

	// Optional arguments
	template<> uint8_t* ToType() { return &Value.Byte; }
	template<> int32_t* ToType() { return &Value.Int; }
	template<> bool* ToType() { return &Value.Bool; }
	template<> float* ToType() { return &Value.Float; }
	template<> UObject** ToType() { return &Value.Object; }
	template<> vec3* ToType() { return &Value.Vector; }
	template<> Rotator* ToType() { return &Value.Rotator; }
	template<> std::string* ToType() { return &ValueString; }
	template<> Color* ToType() { return &Value.Color; }

	static ExpressionValue NothingValue() { ExpressionValue v; v.Type = ExpressionValueType::Nothing; return v; }
	static ExpressionValue ByteValue(uint8_t value) { ExpressionValue v; v.Type = ExpressionValueType::ValueByte; v.Value.Byte = value; return v; }
	static ExpressionValue IntValue(int32_t value) { ExpressionValue v; v.Type = ExpressionValueType::ValueInt; v.Value.Int = value; return v; }
	static ExpressionValue BoolValue(bool value) { ExpressionValue v; v.Type = ExpressionValueType::ValueBool; v.Value.Bool = value; return v; }
	static ExpressionValue FloatValue(float value) { ExpressionValue v; v.Type = ExpressionValueType::ValueFloat; v.Value.Float = value; return v; }
	static ExpressionValue ObjectValue(UObject* value) { ExpressionValue v; v.Type = ExpressionValueType::ValueObject; v.Value.Object = value; return v; }
	static ExpressionValue VectorValue(vec3 value) { ExpressionValue v; v.Type = ExpressionValueType::ValueVector; v.Value.Vector = value; return v; }
	static ExpressionValue RotatorValue(Rotator value) { ExpressionValue v; v.Type = ExpressionValueType::ValueRotator; v.Value.Rotator = value; return v; }
	static ExpressionValue StringValue(std::string value) { ExpressionValue v; v.Type = ExpressionValueType::ValueString; v.ValueString = value; return v; }
	static ExpressionValue NameValue(std::string value) { ExpressionValue v; v.Type = ExpressionValueType::ValueName; v.ValueString = value; return v; }
	static ExpressionValue Variable(UObject* obj, UProperty* property) { ExpressionValue v; v.Type = ExpressionValueType::Variable; v.VariableObject = obj; v.VariableProperty = property; return v; }
};

inline uint8_t ExpressionValue::ToByte() const
{
	if (Type == ExpressionValueType::ValueByte)
		return Value.Byte;
	else if (Type == ExpressionValueType::Variable)
		throw std::runtime_error("Variables not implemented");
	else
		throw std::runtime_error("Not a byte value");
}

inline int32_t ExpressionValue::ToInt() const
{
	if (Type == ExpressionValueType::ValueInt)
		return Value.Int;
	else if (Type == ExpressionValueType::Variable)
		throw std::runtime_error("Variables not implemented");
	else
		throw std::runtime_error("Not a int value");
}

inline bool ExpressionValue::ToBool() const
{
	if (Type == ExpressionValueType::ValueBool)
		return Value.Bool;
	else if (Type == ExpressionValueType::Variable)
		throw std::runtime_error("Variables not implemented");
	else
		throw std::runtime_error("Not a bool value");
}

inline float ExpressionValue::ToFloat() const
{
	if (Type == ExpressionValueType::ValueFloat)
		return Value.Float;
	else if (Type == ExpressionValueType::Variable)
		throw std::runtime_error("Variables not implemented");
	else
		throw std::runtime_error("Not a float value");
}

inline UObject* ExpressionValue::ToObject() const
{
	if (Type == ExpressionValueType::ValueObject)
		return Value.Object;
	else if (Type == ExpressionValueType::Variable)
		throw std::runtime_error("Variables not implemented");
	else
		throw std::runtime_error("Not an object value");
}

inline vec3 ExpressionValue::ToVector() const
{
	if (Type == ExpressionValueType::ValueObject)
		return Value.Vector;
	else if (Type == ExpressionValueType::Variable)
		throw std::runtime_error("Variables not implemented");
	else
		throw std::runtime_error("Not a vector value");
}

inline Rotator ExpressionValue::ToRotator() const
{
	if (Type == ExpressionValueType::ValueObject)
		return Value.Rotator;
	else if (Type == ExpressionValueType::Variable)
		throw std::runtime_error("Variables not implemented");
	else
		throw std::runtime_error("Not a rotator value");
}

inline std::string ExpressionValue::ToString() const
{
	if (Type == ExpressionValueType::ValueString)
		return ValueString;
	else if (Type == ExpressionValueType::Variable)
		throw std::runtime_error("Variables not implemented");
	else
		throw std::runtime_error("Not a string value");
}

inline std::string ExpressionValue::ToName() const
{
	if (Type == ExpressionValueType::ValueString)
		return ValueString;
	else if (Type == ExpressionValueType::Variable)
		throw std::runtime_error("Variables not implemented");
	else
		throw std::runtime_error("Not a name value");
}
