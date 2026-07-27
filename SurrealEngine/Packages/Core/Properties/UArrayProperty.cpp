
#include "Precomp.h"
#include "UArrayProperty.h"

UArrayProperty::UArrayProperty(NameString name, UClass* base, ObjectFlags flags) : UProperty(std::move(name), base, flags)
{
	ValueType = ExpressionValueType::ValueArray;
}

void UArrayProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	Inner = stream->ReadObject<UProperty>();
}

void UArrayProperty::Save(PackageStreamWriter* stream)
{
	UProperty::Save(stream);
	stream->WriteObject(Inner);
}

void UArrayProperty::LoadValue(void* data, ObjectStream* stream, const PropertyHeader& header)
{
	ThrowIfTypeMismatch(header, UPT_Array);

	int arraySize = stream->ReadIndex();
	ScriptArray& vec = static_cast<ScriptArray*>(data)[header.arrayIndex];
	vec.Resize(arraySize);
	for (int i = 0; i < arraySize; i++)
	{
		Inner->LoadStructMemberValue(vec.GetItem(i), stream);
	}
}

void UArrayProperty::LoadStructMemberValue(void* data, ObjectStream* stream)
{
	int arraySize = stream->ReadIndex();
	ScriptArray& vec = *static_cast<ScriptArray*>(data);
	vec.Resize(arraySize);
	for (int i = 0; i < arraySize; i++)
	{
		Inner->LoadStructMemberValue(vec.GetItem(i), stream);
	}
}

void UArrayProperty::SaveHeader(void* data, PropertyHeader& header)
{
	header.type = UPT_Array;
}

void UArrayProperty::SaveValue(void* data, PackageStreamWriter* stream)
{
	ScriptArray& vec = *static_cast<ScriptArray*>(data);
	stream->WriteIndex((int)vec.GetSize());
	for (size_t i = 0, count = vec.GetSize(); i < count; i++)
	{
		Inner->SaveValue(vec.GetItem(i), stream);
	}
}

size_t UArrayProperty::ElementAlignment()
{
	return alignof(ScriptArray);
}

size_t UArrayProperty::ElementSize()
{
	return sizeof(ScriptArray);
}

void UArrayProperty::ConstructElement(void* data)
{
	new ((char*)data) ScriptArray(Inner);
}

void UArrayProperty::CopyConstructElement(void* data, const void* src)
{
	ConstructElement(data);
	CopyElement(data, src);
}

void UArrayProperty::DestructElement(void* data)
{
	auto vec = static_cast<ScriptArray*>(data);
	vec->~ScriptArray();
}

void UArrayProperty::CopyElement(void* data, const void* src)
{
	auto& vec = *static_cast<ScriptArray*>(data);
	auto& srcvec = *static_cast<const ScriptArray*>(src);
	vec = srcvec;
}

bool UArrayProperty::CompareElement(const void* a, const void* b)
{
	auto& avec = *static_cast<const ScriptArray*>(a);
	auto& bvec = *static_cast<const ScriptArray*>(b);
	return avec == bvec;
}

bool UArrayProperty::CompareLessElement(const void* a, const void* b)
{
	auto& avec = *static_cast<const ScriptArray*>(a);
	auto& bvec = *static_cast<const ScriptArray*>(b);
	return avec.GetSize() < bvec.GetSize();
}

void UArrayProperty::GetExportText(std::string& buf, const std::string& whitespace, UObject* obj, UObject* defobj, int i)
{
	if (i >= ArrayDimension)
		Exception::Throw("UArrayProperty::GetExportText index out of bounds");

	size_t elementPitch = ElementPitch();
	int offset = i * (int)elementPitch;

	ScriptArray* objarray = static_cast<ScriptArray*>(obj->GetProperty(Name)) + offset;
	ScriptArray* defarray = (defobj) ? static_cast<ScriptArray*>(defobj->GetProperty(Name)) + offset : nullptr;

	for (int k = 0; k < objarray->GetSize(); k++)
	{
		void* objval = objarray->GetItem(k);
		void* defval = (defarray && k < defarray->GetSize()) ? defarray->GetItem(k) : nullptr;

		Inner->GetExportText(buf, whitespace, objval, defval, i);
	}
}

std::string UArrayProperty::PrintValue(const void* data)
{
	return "array";
}

GCAllocation* UArrayProperty::MarkPropertyElement(GCAllocation* marklist, void* data)
{
	ScriptArray* array = static_cast<ScriptArray*>(data);
	size_t count = array->GetSize();
	for (size_t i = 0; i < count; i++)
	{
		marklist = Inner->MarkPropertyElement(marklist, array->GetItem(i));
	}
	return marklist;
}
