
#include "Precomp.h"
#include "UEnum.h"

void UEnum::Load(ObjectStream* stream)
{
	UField::Load(stream);
	int size = stream->ReadIndex();
	for (int i = 0; i < size; i++)
		ElementNames.push_back(stream->ReadName());
}

void UEnum::Save(PackageStreamWriter* stream)
{
	UField::Save(stream);
	stream->WriteIndex((int)ElementNames.size());
	for (auto& name : ElementNames)
		stream->WriteName(name);
}
