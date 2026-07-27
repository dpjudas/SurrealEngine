
#include "Precomp.h"
#include "ULevelBase.h"
#include "Packages/Engine/Actors/UActor.h"

void ULevelBase::Load(ObjectStream* stream)
{
	UObject::Load(stream);

	int32_t count = stream->ReadInt32();
	int32_t maxcount = stream->ReadInt32();
	for (int32_t i = 0; i < count; i++)
	{
		auto actor = stream->ReadObject<UActor>();
		if (actor)
			actor->Index = (int)Actors.size();
		Actors.push_back(actor);
	}

	Protocol = stream->ReadString();
	Host = stream->ReadString();
	Map = stream->ReadString();
	Portal = stream->ReadString();

	int optcount = stream->ReadIndex();
	for (int i = 0; i < optcount; i++)
		Options.push_back(stream->ReadString());

	Port = stream->ReadInt32();
	Unknown = stream->ReadUInt32();
}

void ULevelBase::Save(PackageStreamWriter* stream)
{
	UObject::Save(stream);

	int32_t count = (int32_t)Actors.size();
	int32_t maxcount = count;
	stream->WriteInt32(count);
	stream->WriteInt32(maxcount);
	for (UActor* actor : Actors)
		stream->WriteObject(actor);

	stream->WriteString(Protocol);
	stream->WriteString(Host);
	stream->WriteString(Map);
	stream->WriteString(Portal);

	stream->WriteIndex((int)Options.size());
	for (const auto& option : Options)
		stream->WriteString(option);

	stream->WriteInt32(Port);
	stream->WriteInt32(Unknown);
}
