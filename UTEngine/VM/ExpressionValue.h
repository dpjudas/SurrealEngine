#pragma once

#include "UObject/UObject.h"
#include "UObject/UProperty.h"

class UProperty;

class StructValue
{
public:
	StructValue() = default;

	StructValue(const StructValue& other)
	{
		Load(other.Struct, other.Ptr);
	}

	StructValue(StructValue&& other)
	{
		*this = std::move(other);
	}

	~StructValue()
	{
		Reset();
	}

	void Store(void* dest) const
	{
		if (Struct)
		{
			for (UProperty* prop : Struct->Properties)
				prop->CopyValue(
					static_cast<uint8_t*>(dest) + prop->DataOffset,
					static_cast<uint8_t*>(Ptr) + prop->DataOffset);
		}
	}

	void Load(UStruct* type, void* src)
	{
		Reset();
		Struct = type;
		if (Struct)
		{
			Ptr = new uint64_t[(Struct->StructSize + 7) / 8];
			for (UProperty* prop : Struct->Properties)
				prop->CopyConstruct(
					static_cast<uint8_t*>(Ptr) + prop->DataOffset,
					static_cast<uint8_t*>(src) + prop->DataOffset);
		}
	}

	void Reset()
	{
		if (Struct)
		{
			for (UProperty* prop : Struct->Properties)
				prop->Destruct(static_cast<uint8_t*>(Ptr) + prop->DataOffset);
			delete[](uint64_t*)Ptr;
			Struct = nullptr;
		}
	}

	StructValue& operator=(const StructValue& other)
	{
		if (this != &other)
		{
			Load(other.Struct, other.Ptr);
		}
		return *this;
	}

	StructValue& operator=(StructValue&& other)
	{
		if (this != &other)
		{
			Struct = other.Struct;
			Ptr = other.Ptr;
			other.Struct = nullptr;
			other.Ptr = nullptr;
		}
		return *this;
	}

	UStruct* Struct = nullptr;
	void* Ptr = nullptr;
};

class ExpressionValue
{
public:
	ExpressionValueType Type = ExpressionValueType::Nothing;

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

	void Store(const ExpressionValue& rvalue);
	void Load();

	bool IsEqual(const ExpressionValue& value) const;

	static ExpressionValue NothingValue() { ExpressionValue v; v.Type = ExpressionValueType::Nothing; return v; }
	static ExpressionValue ByteValue(uint8_t value) { ExpressionValue v; v.Type = ExpressionValueType::ValueByte; v.Value.Byte = value; return v; }
	static ExpressionValue IntValue(int32_t value) { ExpressionValue v; v.Type = ExpressionValueType::ValueInt; v.Value.Int = value; return v; }
	static ExpressionValue BoolValue(bool value) { ExpressionValue v; v.Type = ExpressionValueType::ValueBool; v.Value.Bool = value; return v; }
	static ExpressionValue FloatValue(float value) { ExpressionValue v; v.Type = ExpressionValueType::ValueFloat; v.Value.Float = value; return v; }
	static ExpressionValue ObjectValue(UObject* value) { ExpressionValue v; v.Type = ExpressionValueType::ValueObject; v.Value.Object = value; return v; }
	static ExpressionValue VectorValue(vec3 value) { ExpressionValue v; v.Type = ExpressionValueType::ValueVector; v.Value.Vector = value; return v; }
	static ExpressionValue RotatorValue(Rotator value) { ExpressionValue v; v.Type = ExpressionValueType::ValueRotator; v.Value.Rotator_ = value; return v; }
	static ExpressionValue StringValue(std::string value) { ExpressionValue v; v.Type = ExpressionValueType::ValueString; v.ValueString = value; return v; }
	static ExpressionValue NameValue(std::string value) { ExpressionValue v; v.Type = ExpressionValueType::ValueName; v.ValueString = value; return v; }
	static ExpressionValue ColorValue(Color value) { ExpressionValue v; v.Type = ExpressionValueType::ValueColor; v.Value.Color_ = value; return v; }

	static ExpressionValue PropertyValue(UProperty* prop);
	static ExpressionValue Variable(void* data, UProperty* prop);

private:
	union
	{
		uint8_t Byte;
		int32_t Int;
		bool Bool;
		float Float;
		UObject* Object;
		vec3 Vector;
		Rotator Rotator_;
		Color Color_;
	} Value;
	std::string ValueString;
	StructValue ValueStruct;
};

// Pass by value
template<> inline uint8_t ExpressionValue::ToType() { return ToByte(); }
template<> inline int32_t ExpressionValue::ToType() { return ToInt(); }
template<> inline bool ExpressionValue::ToType() { return ToBool(); }
template<> inline float ExpressionValue::ToType() { return ToFloat(); }
template<> inline UObject* ExpressionValue::ToType() { return ToObject(); }
template<> inline const vec3& ExpressionValue::ToType() { return ToVector(); }
template<> inline const Rotator& ExpressionValue::ToType() { return ToRotator(); }
template<> inline const std::string& ExpressionValue::ToType() { return ToString(); }
template<> inline const Color& ExpressionValue::ToType() { return ToColor(); }

// Pass by reference
template<> inline uint8_t& ExpressionValue::ToType() { return !VariablePtr ? Value.Byte : *static_cast<uint8_t*>(VariablePtr); }
template<> inline int32_t& ExpressionValue::ToType() { return !VariablePtr ? Value.Int : *static_cast<int32_t*>(VariablePtr); }
template<> inline bool& ExpressionValue::ToType() { return !VariablePtr ? Value.Bool : *static_cast<bool*>(VariablePtr); }
template<> inline float& ExpressionValue::ToType() { return !VariablePtr ? Value.Float : *static_cast<float*>(VariablePtr); }
template<> inline UObject*& ExpressionValue::ToType() { return !VariablePtr ? Value.Object : *static_cast<UObject**>(VariablePtr); }
template<> inline vec3& ExpressionValue::ToType() { return !VariablePtr ? Value.Vector : *static_cast<vec3*>(VariablePtr); }
template<> inline Rotator& ExpressionValue::ToType() { return !VariablePtr ? Value.Rotator_ : *static_cast<Rotator*>(VariablePtr); }
template<> inline std::string& ExpressionValue::ToType() { return !VariablePtr ? ValueString : *static_cast<std::string*>(VariablePtr); }
template<> inline Color& ExpressionValue::ToType() { return !VariablePtr ? Value.Color_ : *static_cast<Color*>(VariablePtr); }

// Optional arguments
template<> inline uint8_t* ExpressionValue::ToType() { return Type != ExpressionValueType::Nothing ? &ToType<uint8_t&>() : nullptr; }
template<> inline int32_t* ExpressionValue::ToType() { return Type != ExpressionValueType::Nothing ? &ToType<int32_t&>() : nullptr; }
template<> inline bool* ExpressionValue::ToType() { return Type != ExpressionValueType::Nothing ? &ToType<bool&>() : nullptr; }
template<> inline float* ExpressionValue::ToType() { return Type != ExpressionValueType::Nothing ? &ToType<float&>() : nullptr; }
template<> inline UObject** ExpressionValue::ToType() { return Type != ExpressionValueType::Nothing ? &ToType<UObject*&>() : nullptr; }
template<> inline vec3* ExpressionValue::ToType() { return Type != ExpressionValueType::Nothing ? &ToType<vec3&>() : nullptr; }
template<> inline Rotator* ExpressionValue::ToType() { return Type != ExpressionValueType::Nothing ? &ToType<Rotator&>() : nullptr; }
template<> inline std::string* ExpressionValue::ToType() { return Type != ExpressionValueType::Nothing ? &ToType<std::string&>() : nullptr; }
template<> inline Color* ExpressionValue::ToType() { return Type != ExpressionValueType::Nothing ? &ToType<Color&>() : nullptr; }

inline ExpressionValue ExpressionValue::PropertyValue(UProperty* prop)
{
	ExpressionValue v;
	v.Type = prop->ValueType;
	if (v.Type != ExpressionValueType::Nothing)
		return v;
	throw std::runtime_error("Unsupported expression value property type");
}

inline ExpressionValue ExpressionValue::Variable(void* data, UProperty* prop)
{
	ExpressionValue v = PropertyValue(prop);
	v.VariablePtr = static_cast<uint8_t*>(data) + prop->DataOffset;
	v.VariableProperty = prop;
	return v;
}

inline void ExpressionValue::Store(const ExpressionValue& rvalue)
{
	switch (rvalue.Type)
	{
	default:
	case ExpressionValueType::Nothing: break;
	case ExpressionValueType::ValueByte: *static_cast<uint8_t*>(VariablePtr) = rvalue.ToByte(); break;
	case ExpressionValueType::ValueInt: *static_cast<int32_t*>(VariablePtr) = rvalue.ToInt(); break;
	case ExpressionValueType::ValueBool: *static_cast<bool*>(VariablePtr) = rvalue.ToBool(); break;
	case ExpressionValueType::ValueFloat: *static_cast<float*>(VariablePtr) = rvalue.ToFloat(); break;
	case ExpressionValueType::ValueObject: *static_cast<UObject**>(VariablePtr) = rvalue.ToObject(); break;
	case ExpressionValueType::ValueVector: *static_cast<vec3*>(VariablePtr) = rvalue.ToVector(); break;
	case ExpressionValueType::ValueRotator: *static_cast<Rotator*>(VariablePtr) = rvalue.ToRotator(); break;
	case ExpressionValueType::ValueString: *static_cast<std::string*>(VariablePtr) = rvalue.ToString(); break;
	case ExpressionValueType::ValueName: *static_cast<std::string*>(VariablePtr) = rvalue.ToName(); break;
	case ExpressionValueType::ValueColor: *static_cast<Color*>(VariablePtr) = rvalue.ToColor(); break;
	case ExpressionValueType::ValueStruct:
		if (rvalue.VariablePtr)
		{
			UStruct* Struct = static_cast<UStructProperty*>(VariableProperty)->Struct;
			if (Struct)
			{
				for (UProperty* prop : Struct->Properties)
					prop->CopyValue(
						static_cast<uint8_t*>(VariablePtr) + prop->DataOffset,
						static_cast<uint8_t*>(rvalue.VariablePtr) + prop->DataOffset);
			}
		}
		else
		{
			rvalue.ValueStruct.Store(VariablePtr);
		}
		break;
	}
}

inline void ExpressionValue::Load()
{
	if (VariablePtr)
	{
		switch (Type)
		{
		default:
		case ExpressionValueType::Nothing: break;
		case ExpressionValueType::ValueByte: Value.Byte = *static_cast<uint8_t*>(VariablePtr); break;
		case ExpressionValueType::ValueInt: Value.Int = *static_cast<int32_t*>(VariablePtr); break;
		case ExpressionValueType::ValueBool: Value.Bool = *static_cast<bool*>(VariablePtr); break;
		case ExpressionValueType::ValueFloat: Value.Float = *static_cast<float*>(VariablePtr); break;
		case ExpressionValueType::ValueObject: Value.Object = *static_cast<UObject**>(VariablePtr); break;
		case ExpressionValueType::ValueVector: Value.Vector = *static_cast<vec3*>(VariablePtr); break;
		case ExpressionValueType::ValueRotator: Value.Rotator_ = *static_cast<Rotator*>(VariablePtr); break;
		case ExpressionValueType::ValueString: ValueString = *static_cast<std::string*>(VariablePtr); break;
		case ExpressionValueType::ValueName: ValueString = *static_cast<std::string*>(VariablePtr); break;
		case ExpressionValueType::ValueColor: Value.Color_ = *static_cast<Color*>(VariablePtr); break;
		case ExpressionValueType::ValueStruct: ValueStruct.Load(dynamic_cast<UStructProperty*>(VariableProperty)->Struct, VariablePtr); break;
		}

		VariablePtr = nullptr;
		VariableProperty = nullptr;
	}
}

inline bool ExpressionValue::IsEqual(const ExpressionValue& value) const
{
	switch (Type)
	{
	default:
	case ExpressionValueType::Nothing: return value.Type == ExpressionValueType::Nothing;
	case ExpressionValueType::ValueByte:
	case ExpressionValueType::ValueInt: return ToInt() == value.ToInt();
	case ExpressionValueType::ValueBool: return ToBool() == value.ToBool();
	case ExpressionValueType::ValueFloat: return ToFloat() == value.ToFloat();
	case ExpressionValueType::ValueObject: return ToObject() == value.ToObject();
	case ExpressionValueType::ValueVector: return ToVector() == value.ToVector();
	case ExpressionValueType::ValueRotator: return ToRotator() == value.ToRotator();
	case ExpressionValueType::ValueString: return ToString() == value.ToString();
	case ExpressionValueType::ValueName: return ToName() == value.ToName();
	case ExpressionValueType::ValueColor: return ToColor() == value.ToColor();
	}
}

inline uint8_t ExpressionValue::ToByte() const
{
	if (Type == ExpressionValueType::ValueByte)
		return VariablePtr ? *static_cast<uint8_t*>(VariablePtr) : Value.Byte;
	else if (Type == ExpressionValueType::ValueInt)
		return (uint8_t)(VariablePtr ? *static_cast<int32_t*>(VariablePtr) : Value.Int);
	else if (Type == ExpressionValueType::ValueFloat)
		return (uint8_t)(VariablePtr ? *static_cast<float*>(VariablePtr) : Value.Float);
	else
		throw std::runtime_error("Not a byte value");
}

inline int32_t ExpressionValue::ToInt() const
{
	if (Type == ExpressionValueType::ValueInt)
		return VariablePtr ? *static_cast<int32_t*>(VariablePtr) : Value.Int;
	else if (Type == ExpressionValueType::ValueByte)
		return VariablePtr ? *static_cast<uint8_t*>(VariablePtr) : Value.Byte;
	else if (Type == ExpressionValueType::ValueFloat)
		return (int)(VariablePtr ? *static_cast<float*>(VariablePtr) : Value.Float);
	else
		throw std::runtime_error("Not a int value");
}

inline bool ExpressionValue::ToBool() const
{
	if (Type == ExpressionValueType::ValueBool)
		return VariablePtr ? *static_cast<bool*>(VariablePtr) : Value.Bool;
	else
		throw std::runtime_error("Not a bool value");
}

inline float ExpressionValue::ToFloat() const
{
	if (Type == ExpressionValueType::ValueFloat)
		return VariablePtr ? *static_cast<float*>(VariablePtr) : Value.Float;
	else if (Type == ExpressionValueType::ValueInt)
		return (float)(VariablePtr ? *static_cast<int32_t*>(VariablePtr) : Value.Int);
	else if (Type == ExpressionValueType::ValueByte)
		return VariablePtr ? *static_cast<uint8_t*>(VariablePtr) : Value.Byte;
	else
		throw std::runtime_error("Not a float value");
}

inline UObject* ExpressionValue::ToObject() const
{
	if (Type == ExpressionValueType::ValueObject)
		return VariablePtr ? *static_cast<UObject**>(VariablePtr) : Value.Object;
	else
		throw std::runtime_error("Not an object value");
}

inline const vec3& ExpressionValue::ToVector() const
{
	if (Type == ExpressionValueType::ValueVector)
		return VariablePtr ? *static_cast<vec3*>(VariablePtr) : Value.Vector;
	else
		throw std::runtime_error("Not a vector value");
}

inline const Rotator& ExpressionValue::ToRotator() const
{
	if (Type == ExpressionValueType::ValueRotator)
		return VariablePtr ? *static_cast<Rotator*>(VariablePtr) : Value.Rotator_;
	else
		throw std::runtime_error("Not a rotator value");
}

inline const std::string& ExpressionValue::ToString() const
{
	if (Type == ExpressionValueType::ValueString || Type == ExpressionValueType::ValueName)
		return VariablePtr ? *static_cast<std::string*>(VariablePtr) : ValueString;
	else
		throw std::runtime_error("Not a string value");
}

inline const std::string& ExpressionValue::ToName() const
{
	if (Type == ExpressionValueType::ValueString || Type == ExpressionValueType::ValueName)
		return VariablePtr ? *static_cast<std::string*>(VariablePtr) : ValueString;
	else
		throw std::runtime_error("Not a name value");
}

inline const Color& ExpressionValue::ToColor() const
{
	if (Type == ExpressionValueType::ValueColor)
		return VariablePtr ? *static_cast<Color*>(VariablePtr) : Value.Color_;
	else
		throw std::runtime_error("Not a color value");
}
