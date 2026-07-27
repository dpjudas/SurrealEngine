
#include "Precomp.h"
#include "UField.h"

void UField::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	BaseField = stream->ReadObject<UField>();
	Next = stream->ReadObject<UField>();
}

void UField::Save(PackageStreamWriter* stream)
{
	UObject::Save(stream);
	stream->WriteObject(BaseField);
	stream->WriteObject(Next);
}
