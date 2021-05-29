#pragma once

#include "UObject/UObject.h"
#include "UObject/UProperty.h"

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

	void* VariablePtr = nullptr;
	UProperty* VariableProperty = nullptr;

	uint8_t ToByte() const;
	int32_t ToInt() const;
	bool ToBool() const;
	float ToFloat() const;
	UObject* ToObject() const;
	const vec3& ToVector() const;
	const Rotator& ToRotator() const;
	const std::string& ToString() const;
	const std::string& ToName() const;
	const Color& ToColor() const;

	template<typename T> T ToType();

	// Pass by value
	template<> uint8_t ToType() { return ToByte(); }
	template<> int32_t ToType() { return ToInt(); }
	template<> bool ToType() { return ToBool(); }
	template<> float ToType() { return ToFloat(); }
	template<> UObject* ToType() { return ToObject(); }
	template<> const vec3& ToType() { return ToVector(); }
	template<> const Rotator& ToType() { return ToRotator(); }
	template<> const std::string& ToType() { return ToString(); }
	template<> const Color& ToType() { return ToColor(); }

	// Pass by reference
	template<> uint8_t& ToType() { return (Type != ExpressionValueType::Variable) ? Value.Byte : *static_cast<uint8_t*>(VariablePtr); }
	template<> int32_t& ToType() { return (Type != ExpressionValueType::Variable) ? Value.Int : *static_cast<int32_t*>(VariablePtr); }
	template<> bool& ToType() { return (Type != ExpressionValueType::Variable) ? Value.Bool : *static_cast<bool*>(VariablePtr); }
	template<> float& ToType() { return (Type != ExpressionValueType::Variable) ? Value.Float : *static_cast<float*>(VariablePtr); }
	template<> UObject*& ToType() { return (Type != ExpressionValueType::Variable) ? Value.Object : *static_cast<UObject**>(VariablePtr); }
	template<> vec3& ToType() { return (Type != ExpressionValueType::Variable) ? Value.Vector : *static_cast<vec3*>(VariablePtr); }
	template<> Rotator& ToType() { return (Type != ExpressionValueType::Variable) ? Value.Rotator : *static_cast<Rotator*>(VariablePtr); }
	template<> std::string& ToType() { return (Type != ExpressionValueType::Variable) ? ValueString : *static_cast<std::string*>(VariablePtr); }
	template<> Color& ToType() { return (Type != ExpressionValueType::Variable) ? Value.Color : *static_cast<Color*>(VariablePtr); }

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
	static ExpressionValue ColorValue(Color value) { ExpressionValue v; v.Type = ExpressionValueType::ValueColor; v.Value.Color = value; return v; }

	static ExpressionValue Variable(void* data, UProperty* property)
	{
		ExpressionValue v;
		v.Type = ExpressionValueType::Variable;
		v.VariablePtr = static_cast<uint8_t*>(data) + property->DataOffset;
		v.VariableProperty = property;
		return v;
	}
};

inline uint8_t ExpressionValue::ToByte() const
{
	if (Type == ExpressionValueType::ValueByte)
		return Value.Byte;
	else if (Type == ExpressionValueType::Variable)
		return *static_cast<uint8_t*>(VariablePtr);
	else
		throw std::runtime_error("Not a byte value");
}

inline int32_t ExpressionValue::ToInt() const
{
	if (Type == ExpressionValueType::ValueInt)
		return Value.Int;
	else if (Type == ExpressionValueType::Variable)
		return *static_cast<int32_t*>(VariablePtr);
	else
		throw std::runtime_error("Not a int value");
}

inline bool ExpressionValue::ToBool() const
{
	if (Type == ExpressionValueType::ValueBool)
		return Value.Bool;
	else if (Type == ExpressionValueType::Variable)
		return *static_cast<bool*>(VariablePtr);
	else
		throw std::runtime_error("Not a bool value");
}

inline float ExpressionValue::ToFloat() const
{
	if (Type == ExpressionValueType::ValueFloat)
		return Value.Float;
	else if (Type == ExpressionValueType::Variable)
		return *static_cast<float*>(VariablePtr);
	else
		throw std::runtime_error("Not a float value");
}

inline UObject* ExpressionValue::ToObject() const
{
	if (Type == ExpressionValueType::ValueObject)
		return Value.Object;
	else if (Type == ExpressionValueType::Variable)
		return *static_cast<UObject**>(VariablePtr);
	else
		throw std::runtime_error("Not an object value");
}

inline const vec3& ExpressionValue::ToVector() const
{
	if (Type == ExpressionValueType::ValueVector)
		return Value.Vector;
	else if (Type == ExpressionValueType::Variable)
		return *static_cast<vec3*>(VariablePtr);
	else
		throw std::runtime_error("Not a vector value");
}

inline const Rotator& ExpressionValue::ToRotator() const
{
	if (Type == ExpressionValueType::ValueRotator)
		return Value.Rotator;
	else if (Type == ExpressionValueType::Variable)
		return *static_cast<Rotator*>(VariablePtr);
	else
		throw std::runtime_error("Not a rotator value");
}

inline const std::string& ExpressionValue::ToString() const
{
	if (Type == ExpressionValueType::ValueString)
		return ValueString;
	else if (Type == ExpressionValueType::Variable)
		return *static_cast<std::string*>(VariablePtr);
	else
		throw std::runtime_error("Not a string value");
}

inline const std::string& ExpressionValue::ToName() const
{
	if (Type == ExpressionValueType::ValueName)
		return ValueString;
	else if (Type == ExpressionValueType::Variable)
		return *static_cast<std::string*>(VariablePtr);
	else
		throw std::runtime_error("Not a name value");
}

inline const Color& ExpressionValue::ToColor() const
{
	if (Type == ExpressionValueType::ValueColor)
		return Value.Color;
	else if (Type == ExpressionValueType::Variable)
		return *static_cast<Color*>(VariablePtr);
	else
		throw std::runtime_error("Not a name value");
}
