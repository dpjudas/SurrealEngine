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
					static_cast<uint8_t*>(dest) + prop->DataOffset.DataOffset,
					static_cast<uint8_t*>(Ptr) + prop->DataOffset.DataOffset);
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
					static_cast<uint8_t*>(Ptr) + prop->DataOffset.DataOffset,
					static_cast<uint8_t*>(src) + prop->DataOffset.DataOffset);
		}
	}

	void Reset()
	{
		if (Struct)
		{
			for (UProperty* prop : Struct->Properties)
				prop->Destruct(static_cast<uint8_t*>(Ptr) + prop->DataOffset.DataOffset);
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
	ExpressionValue() : Type(ExpressionValueType::Nothing) { Ptr = nullptr; BoolInfo.Ptr = nullptr; BoolInfo.Mask = 1; }

	ExpressionValue(const ExpressionValue& v) : Type(v.Type)
	{
		if (!v.VariableProperty)
		{
			Ptr = (Type != ExpressionValueType::Nothing) ? &Buffer : nullptr;
			switch (Type)
			{
			default: Buffer = v.Buffer; break;
			case ExpressionValueType::ValueVector: new(PtrByte) vec3(*v.PtrVector); break;
			case ExpressionValueType::ValueRotator: new(PtrByte) Rotator(*v.PtrRotator); break;
			case ExpressionValueType::ValueString: new(PtrByte) std::string(*v.PtrString); break;
			case ExpressionValueType::ValueName: new(PtrByte) NameString(*v.PtrName); break;
			case ExpressionValueType::ValueColor: new(PtrByte) Color(*v.PtrColor); break;
			case ExpressionValueType::ValueStruct: new(PtrByte) StructValue(*v.GetStructValue()); Ptr = GetStructValue()->Ptr; break;
			}
		}
		else
		{
			VariableProperty = v.VariableProperty;
			Ptr = v.Ptr;
		}
		BoolInfo.Ptr = (uint32_t*)Ptr;
		BoolInfo.Mask = v.BoolInfo.Mask;
	}

	ExpressionValue(ExpressionValue&& v) : Type(v.Type)
	{
		if (!v.VariableProperty)
		{
			Ptr = (Type != ExpressionValueType::Nothing) ? &Buffer : nullptr;
			switch (Type)
			{
			default: Buffer = v.Buffer; break;
			case ExpressionValueType::ValueVector: new(PtrByte) vec3(std::move(*v.PtrVector)); break;
			case ExpressionValueType::ValueRotator: new(PtrByte) Rotator(std::move(*v.PtrRotator)); break;
			case ExpressionValueType::ValueString: new(PtrByte) std::string(std::move(*v.PtrString)); break;
			case ExpressionValueType::ValueName: new(PtrByte) NameString(std::move(*v.PtrName)); break;
			case ExpressionValueType::ValueColor: new(PtrByte) Color(std::move(*v.PtrColor)); break;
			case ExpressionValueType::ValueStruct: new(PtrByte) StructValue(std::move(*v.GetStructValue())); Ptr = GetStructValue()->Ptr; break;
			}
		}
		else
		{
			VariableProperty = v.VariableProperty;
			Ptr = v.Ptr;
		}
		BoolInfo.Ptr = (uint32_t*)Ptr;
		BoolInfo.Mask = v.BoolInfo.Mask;
	}

	ExpressionValue& operator=(const ExpressionValue& v)
	{
		if (this != &v)
		{
			Deinit();

			Type = v.Type;
			if (!v.VariableProperty)
			{
				Ptr = (Type != ExpressionValueType::Nothing) ? &Buffer : nullptr;
				switch (Type)
				{
				default: Buffer = v.Buffer; break;
				case ExpressionValueType::ValueVector: new(PtrByte) vec3(*v.PtrVector); break;
				case ExpressionValueType::ValueRotator: new(PtrByte) Rotator(*v.PtrRotator); break;
				case ExpressionValueType::ValueString: new(PtrByte) std::string(*v.PtrString); break;
				case ExpressionValueType::ValueName: new(PtrByte) NameString(*v.PtrName); break;
				case ExpressionValueType::ValueColor: new(PtrByte) Color(*v.PtrColor); break;
				case ExpressionValueType::ValueStruct: new(PtrByte) StructValue(*v.GetStructValue()); Ptr = GetStructValue()->Ptr; break;
				}
			}
			else
			{
				VariableProperty = v.VariableProperty;
				Ptr = v.Ptr;
			}
			BoolInfo.Ptr = (uint32_t*)Ptr;
			BoolInfo.Mask = v.BoolInfo.Mask;
		}
		return *this;
	}

	~ExpressionValue()
	{
		Deinit();
	}

	uint8_t ToByte() const;
	int32_t ToInt() const;
	bool ToBool() const;
	float ToFloat() const;
	UObject* ToObject() const;
	const vec3& ToVector() const;
	const Rotator& ToRotator() const;
	const std::string& ToString() const;
	const NameString& ToName() const;
	const Color& ToColor() const;
	const IpAddr& ToIpAddr() const;

	template<typename T> T ToType();

	void Store(const ExpressionValue& rvalue);
	void Load();

	bool IsEqual(const ExpressionValue& value) const;

	static ExpressionValue NothingValue() { return ExpressionValue(); }
	static ExpressionValue ByteValue(uint8_t value) { ExpressionValue v(ExpressionValueType::ValueByte); *v.PtrByte = value; return v; }
	static ExpressionValue IntValue(int32_t value) { ExpressionValue v(ExpressionValueType::ValueInt); *v.PtrInt = value; return v; }
	static ExpressionValue BoolValue(bool value) { ExpressionValue v(ExpressionValueType::ValueBool); v.BoolInfo.Set(value); return v; }
	static ExpressionValue FloatValue(float value) { ExpressionValue v(ExpressionValueType::ValueFloat); *v.PtrFloat = value; return v; }
	static ExpressionValue ObjectValue(UObject* value) { ExpressionValue v(ExpressionValueType::ValueObject); *v.PtrObject = value; return v; }
	static ExpressionValue VectorValue(vec3 value) { ExpressionValue v(ExpressionValueType::ValueVector); *v.PtrVector = value; return v; }
	static ExpressionValue RotatorValue(Rotator value) { ExpressionValue v(ExpressionValueType::ValueRotator); *v.PtrRotator = value; return v; }
	static ExpressionValue StringValue(std::string value) { ExpressionValue v(ExpressionValueType::ValueString); *v.PtrString = value; return v; }
	static ExpressionValue NameValue(NameString value) { ExpressionValue v(ExpressionValueType::ValueName); *v.PtrName = value; return v; }
	static ExpressionValue ColorValue(Color value) { ExpressionValue v(ExpressionValueType::ValueColor); *v.PtrColor = value; return v; }

	static ExpressionValue DefaultValue(UProperty* prop);
	static ExpressionValue PropertyValue(UProperty* prop);
	static ExpressionValue Variable(void* data, UProperty* prop);

	ExpressionValueType GetType() const { return Type; }
	bool IsVariable() const { return VariableProperty; }

	ExpressionValue ItemAt(int index)
	{
		index = clamp(index, 0, (int)VariableProperty->ArrayDimension - 1);
		return ExpressionValue(PtrByte + VariableProperty->ElementSize() * index, VariableProperty);
	}

	ExpressionValue Member(UProperty* field)
	{
		return ExpressionValue::Variable(Ptr, field);
	}

	void ConstructVariable()
	{
		VariableProperty->Construct(Ptr);
	}

	void DestructVariable()
	{
		VariableProperty->Destruct(Ptr);
	}

private:
	ExpressionValue(ExpressionValueType type) : Type(type)
	{
		Ptr = (Type != ExpressionValueType::Nothing) ? &Buffer : nullptr;
		switch (Type)
		{
		default: break;
		case ExpressionValueType::ValueVector: new(PtrByte) vec3(); break;
		case ExpressionValueType::ValueRotator: new(PtrByte) Rotator(); break;
		case ExpressionValueType::ValueString: new(PtrByte) std::string(); break;
		case ExpressionValueType::ValueName: new(PtrByte) NameString(); break;
		case ExpressionValueType::ValueColor: new(PtrByte) Color(); break;
		case ExpressionValueType::ValueStruct: new(PtrByte) StructValue(); Ptr = GetStructValue()->Ptr; break;
		}
		BoolInfo.Ptr = (uint32_t*)Ptr;
		BoolInfo.Mask = 1;
	}

	ExpressionValue(void* ptr, UProperty* prop) : Type(prop->ValueType)
	{
		Ptr = ptr;
		VariableProperty = prop;
		BoolInfo.Ptr = (uint32_t*)Ptr;
		BoolInfo.Mask = VariableProperty->DataOffset.BitfieldMask;
	}

	void Deinit()
	{
		if (!VariableProperty)
		{
			switch (Type)
			{
			default: break;
			case ExpressionValueType::ValueVector: PtrVector->~vec3(); break;
			case ExpressionValueType::ValueRotator: PtrRotator->~Rotator(); break;
			case ExpressionValueType::ValueString: PtrString->~basic_string(); break;
			case ExpressionValueType::ValueName: PtrName->~NameString(); break;
			case ExpressionValueType::ValueColor: PtrColor->~Color(); break;
			case ExpressionValueType::ValueStruct: GetStructValue()->~StructValue(); break;
			}
			Type = ExpressionValueType::Nothing;
		}
		else
		{
			VariableProperty = nullptr;
		}
	}

	const StructValue* GetStructValue() const { return reinterpret_cast<const StructValue*>(Buffer.Struct); }
	StructValue* GetStructValue() { return reinterpret_cast<StructValue*>(Buffer.Struct); }

	union
	{
		uint8_t Byte;
		int32_t Int;
		uint32_t Bool32;
		float Float;
		UObject* Object;
		uint8_t Vector[sizeof(vec3)];
		uint8_t Rotator_[sizeof(Rotator)];
		uint8_t String[sizeof(std::string)];
		uint8_t Name[sizeof(NameString)];
		uint8_t Color_[sizeof(Color)];
		uint8_t Struct[sizeof(StructValue)];
	} Buffer;

	ExpressionValueType Type = ExpressionValueType::Nothing;

	union
	{
		void* Ptr;
		uint8_t* PtrByte;
		int32_t* PtrInt;
		uint32_t* PtrBool32;
		float* PtrFloat;
		UObject** PtrObject;
		vec3* PtrVector;
		Rotator* PtrRotator;
		std::string* PtrString;
		NameString* PtrName;
		Color* PtrColor;
		IpAddr* PtrIpAddr;
	};

	UProperty* VariableProperty = nullptr;
	BitfieldBool BoolInfo;
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
template<> inline const NameString& ExpressionValue::ToType() { return ToName(); }
template<> inline const Color& ExpressionValue::ToType() { return ToColor(); }
template<> inline const IpAddr& ExpressionValue::ToType() { return ToIpAddr(); }

// Pass by reference
template<> inline uint8_t& ExpressionValue::ToType() { return *PtrByte; }
template<> inline int32_t& ExpressionValue::ToType() { return *PtrInt; }
template<> inline BitfieldBool& ExpressionValue::ToType() { return BoolInfo; }
template<> inline float& ExpressionValue::ToType() { return *PtrFloat; }
template<> inline UObject*& ExpressionValue::ToType() { return *PtrObject; }
template<> inline vec3& ExpressionValue::ToType() { return *PtrVector; }
template<> inline Rotator& ExpressionValue::ToType() { return *PtrRotator; }
template<> inline std::string& ExpressionValue::ToType() { return *PtrString; }
template<> inline NameString& ExpressionValue::ToType() { return *PtrName; }
template<> inline Color& ExpressionValue::ToType() { return *PtrColor; }
template<> inline IpAddr& ExpressionValue::ToType() { return *PtrIpAddr; }

// Optional arguments
template<> inline uint8_t* ExpressionValue::ToType() { return PtrByte; }
template<> inline int32_t* ExpressionValue::ToType() { return PtrInt; }
template<> inline BitfieldBool* ExpressionValue::ToType() { return BoolInfo.Ptr ? &BoolInfo : nullptr; }
template<> inline float* ExpressionValue::ToType() { return PtrFloat; }
template<> inline UObject** ExpressionValue::ToType() { return PtrObject; }
template<> inline vec3* ExpressionValue::ToType() { return PtrVector; }
template<> inline Rotator* ExpressionValue::ToType() { return PtrRotator; }
template<> inline std::string* ExpressionValue::ToType() { return PtrString; }
template<> inline NameString* ExpressionValue::ToType() { return PtrName; }
template<> inline Color* ExpressionValue::ToType() { return PtrColor; }
template<> inline IpAddr* ExpressionValue::ToType() { return PtrIpAddr; }

inline ExpressionValue ExpressionValue::DefaultValue(UProperty* prop)
{
	switch (prop->ValueType)
	{
	default:
	{
		ExpressionValue v(prop->ValueType);
		memset(v.Ptr, 0, sizeof(Buffer));
		if (v.Type != ExpressionValueType::Nothing)
			return v;
		throw std::runtime_error("Unsupported expression value property type");
	}
	case ExpressionValueType::ValueVector: return VectorValue(vec3(0.0f));
	case ExpressionValueType::ValueRotator: return RotatorValue(Rotator(0, 0, 0));
	case ExpressionValueType::ValueString: return StringValue({});
	case ExpressionValueType::ValueName: return NameValue({});
	case ExpressionValueType::ValueColor: { Color c; c.R = c.G = c.B = c.A = 0; return ColorValue(c); }
	case ExpressionValueType::ValueStruct: throw std::runtime_error("Default value for a struct type is not implemented");
	}
}

inline ExpressionValue ExpressionValue::PropertyValue(UProperty* prop)
{
	ExpressionValue v(prop->ValueType);
	if (v.Type != ExpressionValueType::Nothing)
		return v;
	throw std::runtime_error("Unsupported expression value property type");
}

inline ExpressionValue ExpressionValue::Variable(void* data, UProperty* prop)
{
	return ExpressionValue(static_cast<uint8_t*>(data) + prop->DataOffset.DataOffset, prop);
}

inline void ExpressionValue::Store(const ExpressionValue& rvalue)
{
	switch (rvalue.Type)
	{
	case ExpressionValueType::Nothing: break;
	case ExpressionValueType::ValueByte: *PtrByte = rvalue.ToByte(); break;
	case ExpressionValueType::ValueInt: *PtrInt = rvalue.ToInt(); break;
	case ExpressionValueType::ValueBool: BoolInfo.Set(rvalue.ToBool()); break;
	case ExpressionValueType::ValueFloat: *PtrFloat = rvalue.ToFloat(); break;
	case ExpressionValueType::ValueObject: *PtrObject = rvalue.ToObject(); break;
	case ExpressionValueType::ValueVector: *PtrVector = rvalue.ToVector(); break;
	case ExpressionValueType::ValueRotator: *PtrRotator = rvalue.ToRotator(); break;
	case ExpressionValueType::ValueString: *PtrString = rvalue.ToString(); break;
	case ExpressionValueType::ValueName: *PtrName = rvalue.ToName(); break;
	case ExpressionValueType::ValueColor: *PtrColor = rvalue.ToColor(); break;
	case ExpressionValueType::ValueStruct:
		if (rvalue.VariableProperty)
		{
			UStruct* Struct = static_cast<UStructProperty*>(rvalue.VariableProperty)->Struct;
			if (Struct)
			{
				for (UProperty* prop : Struct->Properties)
					prop->CopyValue(
						static_cast<uint8_t*>(Ptr) + prop->DataOffset.DataOffset,
						static_cast<uint8_t*>(rvalue.Ptr) + prop->DataOffset.DataOffset);
			}
		}
		else
		{
			rvalue.GetStructValue()->Store(Ptr);
		}
		break;
	}
}

inline void ExpressionValue::Load()
{
	if (VariableProperty)
	{
		ExpressionValue value(Type);
		switch (Type)
		{
		default:
		case ExpressionValueType::Nothing: break;
		case ExpressionValueType::ValueByte: *value.PtrByte = *PtrByte; break;
		case ExpressionValueType::ValueInt: *value.PtrInt = *PtrInt; break;
		case ExpressionValueType::ValueBool: value.BoolInfo.Set(BoolInfo.Get()); break;
		case ExpressionValueType::ValueFloat: *value.PtrFloat = *PtrFloat; break;
		case ExpressionValueType::ValueObject: *value.PtrObject = *PtrObject; break;
		case ExpressionValueType::ValueVector: *value.PtrVector = *PtrVector; break;
		case ExpressionValueType::ValueRotator: *value.PtrRotator = *PtrRotator; break;
		case ExpressionValueType::ValueString: *value.PtrString = *PtrString; break;
		case ExpressionValueType::ValueName: *value.PtrName = *PtrName; break;
		case ExpressionValueType::ValueColor: *value.PtrColor = *PtrColor; break;
		case ExpressionValueType::ValueStruct:
			value.GetStructValue()->Load(dynamic_cast<UStructProperty*>(VariableProperty)->Struct, Ptr);
			value.Ptr = value.GetStructValue()->Ptr;
			break;
		}
		*this = std::move(value);
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
	case ExpressionValueType::ValueStruct: throw std::runtime_error("IsEqual not implemented for complex structs");
	}
}

inline uint8_t ExpressionValue::ToByte() const
{
	if (Type == ExpressionValueType::ValueByte)
		return *PtrByte;
	else if (Type == ExpressionValueType::ValueInt)
		return (uint8_t)*PtrInt;
	else if (Type == ExpressionValueType::ValueFloat)
		return (uint8_t)*PtrFloat;
	else
		throw std::runtime_error("Not a byte compatible value");
}

inline int32_t ExpressionValue::ToInt() const
{
	if (Type == ExpressionValueType::ValueInt)
		return *PtrInt;
	else if (Type == ExpressionValueType::ValueByte)
		return *PtrByte;
	else if (Type == ExpressionValueType::ValueFloat)
		return (int)*PtrFloat;
	else
		throw std::runtime_error("Not an int compatible value");
}

inline bool ExpressionValue::ToBool() const
{
	if (Type == ExpressionValueType::ValueBool)
		return BoolInfo.Get();
	else
		throw std::runtime_error("Not a bool value");
}

inline float ExpressionValue::ToFloat() const
{
	if (Type == ExpressionValueType::ValueFloat)
		return *PtrFloat;
	else if (Type == ExpressionValueType::ValueInt)
		return (float)*PtrInt;
	else if (Type == ExpressionValueType::ValueByte)
		return *PtrByte;
	else
		throw std::runtime_error("Not a float compatible value");
}

inline UObject* ExpressionValue::ToObject() const
{
	if (Type == ExpressionValueType::ValueObject)
		return *PtrObject;
	else
		throw std::runtime_error("Not an object value");
}

inline const vec3& ExpressionValue::ToVector() const
{
	if (Type == ExpressionValueType::ValueVector)
		return *PtrVector;
	else
		throw std::runtime_error("Not a vector value");
}

inline const Rotator& ExpressionValue::ToRotator() const
{
	if (Type == ExpressionValueType::ValueRotator)
		return *PtrRotator;
	else
		throw std::runtime_error("Not a rotator value");
}

inline const std::string& ExpressionValue::ToString() const
{
	if (Type == ExpressionValueType::ValueString)
		return *PtrString;
	else if (Type == ExpressionValueType::ValueName)
		return PtrName->ToString();
	else
		throw std::runtime_error("Not a string value");
}

inline const NameString& ExpressionValue::ToName() const
{
	if (Type == ExpressionValueType::ValueName)
		return *PtrName;
	else
		throw std::runtime_error("Not a name value");
}

inline const Color& ExpressionValue::ToColor() const
{
	if (Type == ExpressionValueType::ValueColor)
		return *PtrColor;
	else
		throw std::runtime_error("Not a color value");
}

inline const IpAddr& ExpressionValue::ToIpAddr() const
{
	if (Type == ExpressionValueType::ValueStruct)
		return *PtrIpAddr;
	else
		throw std::runtime_error("Not a ipaddr/struct value");
}
