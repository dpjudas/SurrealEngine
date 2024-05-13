#pragma once

#include "UClass.h"

struct PropertyHeader;

enum class PropertyFlags : uint32_t
{
	Edit = 0x00000001, // Property is user - settable in the editor.
	Const = 0x00000002, // Actor's property always matches class's default actor property.
	Input = 0x00000004, // Variable is writable by the input system.
	ExportObject = 0x00000008, // Object can be exported with actor.
	OptionalParm = 0x00000010, // Optional parameter(if Param is set).
	Net = 0x00000020, // Property is relevant to network replication
	ConstRef = 0x00000040, // Reference to a constant object.
	Parm = 0x00000080, // Function / When call parameter
	OutParm = 0x00000100, // Value is copied out after function call.
	SkipParm = 0x00000200, // Property is a short - circuitable evaluation function parm.
	ReturnParm = 0x00000400, // Return value.
	CoerceParm = 0x00000800, // Coerce args into this function parameter
	Native = 0x00001000, // Property is native : C++ code is responsible for serializing it.
	Transient = 0x00002000, // Property is transient : shouldn't be saved, zerofilled at load time.
	Config = 0x00004000, // Property should be loaded / saved as permanent profile.
	Localized = 0x00008000, // Property should be loaded as localizable text
	Travel = 0x00010000, // Property travels across levels / servers.
	EditConst = 0x00020000, // Property is uneditable in the editor
	GlobalConfig = 0x00040000, // Load config from base class, not subclass.
	OnDemand = 0x00100000, // Object or dynamic array loaded on demand only.
	New = 0x00200000, // Automatically create inner object
	NeedCtorLink = 0x00400000 // Fields need construction / destruction
};

inline PropertyFlags operator|(PropertyFlags a, PropertyFlags b) { return (PropertyFlags)((uint32_t)a | (uint32_t)b); }
inline PropertyFlags operator&(PropertyFlags a, PropertyFlags b) { return (PropertyFlags)((uint32_t)a & (uint32_t)b); }
inline bool AllFlags(PropertyFlags value, PropertyFlags flags) { return (value & flags) == flags; }
inline bool AnyFlags(PropertyFlags value, PropertyFlags flags) { return (uint32_t)(value & flags) != 0; }

enum class ExpressionValueType
{
	Nothing,
	ValueByte,
	ValueInt,
	ValueBool,
	ValueFloat,
	ValueObject,
	ValueVector,
	ValueRotator,
	ValueString,
	ValueName,
	ValueColor,
	ValueStruct
};

class UProperty : public UField
{
public:
	using UField::UField;
	void Load(ObjectStream* stream) override;

	virtual void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header);
	virtual void LoadStructMemberValue(void* data, ObjectStream* stream);

	virtual size_t Alignment() { return 4; }
	size_t Size() { return ElementSize() * ArrayDimension; }
	virtual size_t ElementSize() { return 4; }
	virtual void GetExportText(std::string& buf, const std::string& whitespace, UObject* obj, UObject* default, int i)
	{
		if (i >= ArrayDimension)
			Exception::Throw("UProperty::GetExportText index out of bounds");

		size_t elementSize = ElementSize();
		int offset = i * (int)elementSize;

		void* objval = static_cast<uint8_t*>(obj->PropertyData.Ptr(this)) + offset;
		void* defval = nullptr;
		try
		{
			defval = (default) ? static_cast<uint8_t*>(default->GetProperty(Name)) + offset : nullptr;
		}
		catch (...) 
		{
			// ignore exceptions, its ok if GetProperty fails here
		}

		std::string innerbuf;
		GetExportText(innerbuf, whitespace, objval, defval, i);
		if (innerbuf.size() > 0)
			innerbuf += "\r\n";

		buf += innerbuf;
	}
	virtual void GetExportText(std::string& buf, const std::string& whitespace, void* objval, void* defval, int i)
	{
		if (!defval && IsDefaultValue(objval))
			return;
		else if (defval && Compare(objval, defval))
			return;

		buf += whitespace + Name.ToString();
		if (ArrayDimension > 1)
			buf += '(' + std::to_string(i) + ')';
		buf += '=' + PrintValue(objval);
	}
	virtual bool Compare(void* v1, void* v2)
	{
		return memcmp(v1, v2, ElementSize()) == 0;
	}
	virtual void Construct(void* data) { memset(data, 0, Size()); }
	virtual void CopyConstruct(void* data, void* src) { memcpy(data, src, Size()); }
	virtual void CopyValue(void* data, void* src)
	{
		if (data != src)
		{
			Destruct(data);
			CopyConstruct(data, src);
		}
	}
	virtual void Destruct(void* data) { }
	virtual bool IsDefaultValue(void* val) { return false; }

	virtual std::string PrintValue(const void* data) { return "?"; }

	static void ThrowIfTypeMismatch(const PropertyHeader& header, UnrealPropertyType type);

	int ArrayDimension = 0;
	PropertyFlags PropFlags = {};
	NameString Category;
	uint16_t ReplicationOffset = 0;

	PropertyDataOffset DataOffset = { 0, 1 };
	ExpressionValueType ValueType = ExpressionValueType::Nothing;
};

class UPointerProperty : public UProperty // 469 extension?
{
public:
	using UProperty::UProperty;
	void Load(ObjectStream* stream) override;
	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;
	size_t Alignment() override { return sizeof(void*); }
	size_t ElementSize() override { return sizeof(void*); }
	std::string PrintValue(const void* data) override { return "pointer"; }
};

class UByteProperty : public UProperty
{
public:
	UByteProperty(NameString name, UClass* base, ObjectFlags flags) : UProperty(std::move(name), base, flags) { ValueType = ExpressionValueType::ValueByte; }
	void Load(ObjectStream* stream) override;
	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;
	size_t Alignment() override { return 1; }
	size_t ElementSize() override { return 1; }

	std::string PrintValue(const void* data) override 
	{ 
		uint8_t val = *(uint8_t*)data;
		if (EnumType && val < EnumType->ElementNames.size())
			return EnumType->ElementNames[val].ToString();
		
		return std::to_string(*(uint8_t*)data); 
	}
	bool IsDefaultValue(void* val)
	{
		return *(uint8_t*)val == 0;
	}

	UEnum* EnumType = nullptr; // null if it is a normal byte, otherwise it is an enum type
};

class UObjectProperty : public UProperty
{
public:
	UObjectProperty(NameString name, UClass* base, ObjectFlags flags) : UProperty(std::move(name), base, flags) { ValueType = ExpressionValueType::ValueObject; }
	void Load(ObjectStream* stream) override;
	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;
	size_t Alignment() override { return sizeof(void*); }
	size_t ElementSize() override { return sizeof(void*); }
	bool Compare(void* v1, void* v2)
	{
		UObject* o1 = *(UObject**)v1;
		UObject* o2 = *(UObject**)v2;
		return o1 == o2;
	}
	std::string PrintValue(const void* data) override
	{
		UObject* obj = *(UObject**)data;
		if (obj)
			return obj->Class->Name.ToString() + '\'' + obj->package->GetExportName(obj->exportIndex) + '\'';
		else
			return "None";
	}
	bool IsDefaultValue(void* val)
	{
		return *(UObject**)val == nullptr;
	}

	UClass* ObjectClass = nullptr;
};

class UFixedArrayProperty : public UProperty
{
public:
	using UProperty::UProperty;
	void Load(ObjectStream* stream) override;
	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	size_t Alignment() override { return Inner->Alignment(); }
	size_t ElementSize() override { return Inner->Size() * Count; }

	void Construct(void* data) override
	{
		uint8_t* p = static_cast<uint8_t*>(data);
		size_t s = Inner->Size();
		int totalcount = Count * ArrayDimension;
		for (int i = 0; i < totalcount; i++)
		{
			Inner->Construct(p);
			p += s;
		}
	}

	void CopyConstruct(void* data, void* src) override
	{
		uint8_t* p = static_cast<uint8_t*>(data);
		uint8_t* sp = static_cast<uint8_t*>(src);
		size_t s = Inner->Size();
		int totalcount = Count * ArrayDimension;
		for (int i = 0; i < totalcount; i++)
		{
			Inner->CopyConstruct(p, sp);
			p += s;
			sp += s;
		}
	}

	void Destruct(void* data) override
	{
		uint8_t* p = static_cast<uint8_t*>(data);
		size_t s = Inner->Size();
		int totalcount = Count * ArrayDimension;
		for (int i = 0; i < totalcount; i++)
		{
			Inner->Destruct(p);
			p += s;
		}
	}

	std::string PrintValue(const void* data) override { return "fixed array"; }

	UProperty* Inner = nullptr;
	int Count = 0;
};

class UArrayProperty : public UProperty
{
public:
	using UProperty::UProperty;
	void Load(ObjectStream* stream) override;
	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	size_t Alignment() override { return sizeof(void*); }
	size_t ElementSize() override { return sizeof(std::vector<void*>); }

	void GetExportText(std::string& buf, const std::string& whitespace, UObject* obj, UObject* default, int i)
	{
		if (i >= ArrayDimension)
			Exception::Throw("UArrayProperty::GetExportText index out of bounds");

		size_t elementSize = ElementSize();
		int offset = i * (int)elementSize;

		std::vector<void*>* objarray = static_cast<std::vector<void*>*>(obj->GetProperty(Name)) + offset;
		std::vector<void*>* defarray = (default) ? static_cast<std::vector<void*>*>(default->GetProperty(Name)) + offset : nullptr;

		for (int k = 0; k < objarray->size(); k++)
		{
			void* objval = objarray->at(k);
			void* defval = (defarray && k < defarray->size()) ? defarray->at(k) : nullptr;

			Inner->GetExportText(buf, whitespace, objval, defval, i);
		}
	}

	void Construct(void* data) override
	{
		auto vec = static_cast<std::vector<void*>*>(data);
		for (int i = 0; i < ArrayDimension; i++)
			new(vec + i) std::vector<void*>();
	}

	void CopyConstruct(void* data, void* src) override
	{
		auto vec = static_cast<std::vector<void*>*>(data);
		auto srcvec = static_cast<std::vector<void*>*>(src);

		for (int i = 0; i < ArrayDimension; i++)
		{
			new(vec + i) std::vector<void*>();

			size_t s = (Inner->Size() + 7) / 8;
			for (auto& sp : srcvec[i])
			{
				int64_t* d = new int64_t[s];
				Inner->CopyConstruct(d, sp);
				vec[i].push_back(d);
			}
		}
	}

	void Destruct(void* data) override
	{
		auto vec = static_cast<std::vector<void*>*>(data);
		for (int i = 0; i < ArrayDimension; i++)
		{
			for (void* d : vec[i])
			{
				Inner->Destruct(d);
				delete[](int64_t*)d;
			}
			vec[i].~vector();
		}
	}

	std::string PrintValue(const void* data) override { return "array"; }

	UProperty* Inner = nullptr;
};

class UMapProperty : public UProperty
{
public:
	using UProperty::UProperty;
	void Load(ObjectStream* stream) override;
	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	size_t Alignment() override { return sizeof(void*); }
	size_t ElementSize() override { return sizeof(std::map<void*,void*>); }

	void Construct(void* data) override
	{
		auto map = static_cast<std::map<void*, void*>*>(data);
		for (int i = 0; i < ArrayDimension; i++)
			new(map + i) std::map<void*, void*>();
	}

	void CopyConstruct(void* data, void* src) override
	{
		auto map = static_cast<std::map<void*, void*>*>(data);
		auto srcmap = static_cast<std::map<void*, void*>*>(src);
		for (int i = 0; i < ArrayDimension; i++)
		{
			new(map + i) std::map<void*, void*>();

			size_t sk = (Key->Size() + 7) / 8;
			size_t sv = (Value->Size() + 7) / 8;
			for (auto& sp : srcmap[i])
			{
				int64_t* dk = new int64_t[sk];
				int64_t* dv = new int64_t[sv];
				Key->CopyConstruct(dk, sp.first);
				Key->CopyConstruct(dv, sp.second);
				map[i][dk] = dv;
			}
		}
	}

	void Destruct(void* data) override
	{
		auto map = static_cast<std::map<void*, void*>*>(data);
		for (int i = 0; i < ArrayDimension; i++)
		{
			for (auto& it : map[i])
			{
				Key->Destruct(it.first);
				Key->Destruct(it.second);
				delete[](int64_t*)it.first;
				delete[](int64_t*)it.second;
			}
			map[i].~map();
		}
	}

	std::string PrintValue(const void* data) override { return "map"; }

	UProperty* Key = nullptr;
	UProperty* Value = nullptr;
};

class UClassProperty : public UObjectProperty
{
public:
	using UObjectProperty::UObjectProperty;
	void Load(ObjectStream* stream) override;

	std::string PrintValue(const void* data) override
	{
		UObject* obj = *(UObject**)data;
		if (obj)
			return "Class'" + obj->package->GetPackageName().ToString() + '.' + obj->Name.ToString() + '\'';
		else
			return "None";
	}

	UClass* MetaClass = nullptr;
};

class UStructProperty : public UProperty
{
public:
	UStructProperty(NameString name, UClass* base, ObjectFlags flags) : UProperty(std::move(name), base, flags) { ValueType = ExpressionValueType::ValueStruct; }

	void Load(ObjectStream* stream) override;
	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	size_t Alignment() override { return sizeof(void*); }
	size_t ElementSize() override { return Struct ? Struct->StructSize : 0; }

	void GetExportText(std::string& buf, const std::string& whitespace, UObject* obj, UObject* default, int i)
	{
		if (!Struct)
		{
			buf += whitespace + Name.ToString() + '=' + "null struct";
			return;
		}

		if (i >= ArrayDimension)
			Exception::Throw("UStructProperty::GetExportText index out of bounds");

		uint8_t* objval = static_cast<uint8_t*>(obj->PropertyData.Ptr(this)) + (i * ElementSize());
		uint8_t* defval = nullptr;
		try
		{
			if (default)
				defval = static_cast<uint8_t*>(default->GetProperty(Name)) + (i * ElementSize());
		}
		catch (...)
		{
		}

		std::string structbuf = "(";
		std::string innerbuf;

		for (UField* field = Struct->Children; field != nullptr; field = field->Next) 
		{
			UProperty* fieldprop = dynamic_cast<UProperty*>(field);

			if (fieldprop == nullptr)
				continue;

			size_t offset = fieldprop->DataOffset.DataOffset;

			for (int k = 0; k < fieldprop->ArrayDimension; k++)
			{
				uint8_t* objsubval = objval + offset;
				uint8_t* defsubval = (defval) ? defval + offset : nullptr;
				fieldprop->GetExportText(innerbuf, "", objsubval, defsubval, k);

				if (innerbuf.length() > 0)
				{
					structbuf += innerbuf + ',';
					innerbuf.clear();
				}
			}
		}

		if (structbuf.length() > 1)
		{
			structbuf.pop_back();
			buf += whitespace + Name.ToString() + "=" + structbuf + ")\r\n";
		}
	}

	std::string PrintValue(const void* data) override
	{
		if (Struct)
		{
			std::string print;
			uint8_t* d = (uint8_t*)data;
			for (UField* field = Struct->Children; field != nullptr; field = field->Next)
			{
				UProperty* fieldprop = dynamic_cast<UProperty*>(field);
				if (fieldprop)
				{
					print += print.empty() ? " " : ", ";
					print += fieldprop->Name.ToString();
					print += "=";
					print += fieldprop->PrintValue(d + fieldprop->DataOffset.DataOffset);
				}
			}
			return "(" + print + ")";
		}
		else
		{
			return "null struct";
		}
	}

	UStruct* Struct = nullptr;
};

class UIntProperty : public UProperty
{
public:
	UIntProperty(NameString name, UClass* base, ObjectFlags flags) : UProperty(std::move(name), base, flags) { ValueType = ExpressionValueType::ValueInt; }
	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;
	std::string PrintValue(const void* data) override { return std::to_string(*(int32_t*)data); }
	bool IsDefaultValue(void* val)
	{
		return *(int*)val == 0;
	}
};

class UBoolProperty : public UProperty
{
public:
	UBoolProperty(NameString name, UClass* base, ObjectFlags flags) : UProperty(std::move(name), base, flags) { ValueType = ExpressionValueType::ValueBool; }
	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	bool Compare(void* v1, void* v2)
	{
		bool b1 = GetBool(v1);
		bool b2 = GetBool(v2);
		return b1 == b2;
	}
	void Construct(void* data) override
	{
		SetBool(data, false);
	}

	void CopyConstruct(void* data, void* src) override
	{
		SetBool(data, GetBool(src));
	}

	bool GetBool(const void* data) const
	{
		uint32_t v = *reinterpret_cast<const uint32_t*>(data);
		return (v & DataOffset.BitfieldMask) != 0;
	}

	void SetBool(void* data, bool value)
	{
		uint32_t& v = *reinterpret_cast<uint32_t*>(data);
		if (value)
			v = v | DataOffset.BitfieldMask;
		else
			v = v & ~DataOffset.BitfieldMask;
	}

	bool IsDefaultValue(void* val)
	{
		return GetBool(val) == false;
	}

	std::string PrintValue(const void* data) override { return GetBool(data) ? "True" : "False"; }
};

class UFloatProperty : public UProperty
{
public:
	UFloatProperty(NameString name, UClass* base, ObjectFlags flags) : UProperty(std::move(name), base, flags) { ValueType = ExpressionValueType::ValueFloat; }
	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;
	std::string PrintValue(const void* data) override { return std::to_string(*(float*)data); }
	bool IsDefaultValue(void* val)
	{
		return *(float*)val == 0.0f;
	}
};

class UNameProperty : public UProperty
{
public:
	UNameProperty(NameString name, UClass* base, ObjectFlags flags) : UProperty(std::move(name), base, flags) { ValueType = ExpressionValueType::ValueName; }

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	size_t Alignment() override { return sizeof(void*); }
	size_t ElementSize() override { return sizeof(NameString); }
	
	bool Compare(void* v1, void* v2)
	{
		NameString* n1 = static_cast<NameString*>(v1);
		NameString* n2 = static_cast<NameString*>(v2);
		return *n1 == *n2;
	}

	void Construct(void* data) override
	{
		auto str = static_cast<NameString*>(data);
		for (int i = 0; i < ArrayDimension; i++)
			new(str + i) NameString();
	}

	void CopyConstruct(void* data, void* src) override
	{
		auto str = static_cast<NameString*>(data);
		auto srcstr = static_cast<NameString*>(src);
		for (int i = 0; i < ArrayDimension; i++)
			new(str + i) NameString(srcstr[i]);
	}

	void Destruct(void* data) override
	{
		auto str = static_cast<NameString*>(data);
		for (int i = 0; i < ArrayDimension; i++)
			str[i].~NameString();
	}

	std::string PrintValue(const void* data) override { return ((NameString*)data)->ToString(); }

	bool IsDefaultValue(void* val)
	{
		return static_cast<NameString*>(val)->IsNone();
	}
};

class UStrProperty : public UProperty
{
public:
	UStrProperty(NameString name, UClass* base, ObjectFlags flags) : UProperty(std::move(name), base, flags) { ValueType = ExpressionValueType::ValueString; }

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;
	void LoadStructMemberValue(void* data, ObjectStream* stream) override;

	size_t Alignment() override { return sizeof(void*); }
	size_t ElementSize() override { return sizeof(std::string); }

	void Construct(void* data) override
	{
		auto str = static_cast<std::string*>(data);
		for (int i = 0; i < ArrayDimension; i++)
			new(str + i) std::string();
	}

	void CopyConstruct(void* data, void* src) override
	{
		auto str = static_cast<std::string*>(data);
		auto srcstr = static_cast<std::string*>(src);
		for (int i = 0; i < ArrayDimension; i++)
			new(str + i) std::string(srcstr[i]);
	}

	void Destruct(void* data) override
	{
		auto str = static_cast<std::string*>(data);
		for (int i = 0; i < ArrayDimension; i++)
			str[i].~basic_string();
	}

	std::string PrintValue(const void* data) override { return '"' + *(std::string*)data + '"'; }
	bool IsDefaultValue(void* val)
	{
		return ((std::string*)val)->length() == 0;
	}
};

class UStringProperty : public UProperty
{
public:
	UStringProperty(NameString name, UClass* base, ObjectFlags flags) : UProperty(std::move(name), base, flags) { ValueType = ExpressionValueType::ValueString; }

	void LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header) override;

	size_t Alignment() override { return sizeof(void*); }
	size_t ElementSize() override { return sizeof(std::string); }

	void Construct(void* data) override
	{
		auto str = static_cast<std::string*>(data);
		for (int i = 0; i < ArrayDimension; i++)
			new(str + i) std::string();
	}

	void CopyConstruct(void* data, void* src) override
	{
		auto str = static_cast<std::string*>(data);
		auto srcstr = static_cast<std::string*>(src);
		for (int i = 0; i < ArrayDimension; i++)
			new(str + i) std::string(srcstr[i]);
	}

	void Destruct(void* data) override
	{
		auto str = static_cast<std::string*>(data);
		for (int i = 0; i < ArrayDimension; i++)
			str[i].~basic_string();
	}

	std::string PrintValue(const void* data) override { return '"' + *(std::string*)data + '"'; }
	bool IsDefaultValue(void* val)
	{
		return ((std::string*)val)->length() == 0;
	}
};
