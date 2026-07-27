
#include "Precomp.h"
#include "UVectors.h"

void UVectors::Load(ObjectStream* stream)
{
	UObject::Load(stream);
	int count = stream->ReadInt32();
	int maxcount = stream->ReadInt32();
	for (int i = 0; i < count; i++)
	{
		vec3 v;
		v.x = stream->ReadFloat();
		v.y = stream->ReadFloat();
		v.z = stream->ReadFloat();
		Vectors.push_back(v);
	}
}

void UVectors::Save(PackageStreamWriter* stream)
{
	UObject::Save(stream);
	int count = (int)Vectors.size();
	int maxcount = count;
	stream->WriteInt32(count);
	stream->WriteInt32(maxcount);
	for (const vec3& v : Vectors)
	{
		stream->WriteFloat(v.x);
		stream->WriteFloat(v.y);
		stream->WriteFloat(v.z);
	}
}
