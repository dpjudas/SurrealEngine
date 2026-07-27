
#include "Precomp.h"
#include "UPrimitive.h"

void UPrimitive::Load(ObjectStream* stream)
{
	UObject::Load(stream);

	BoundingBox.min.x = stream->ReadFloat();
	BoundingBox.min.y = stream->ReadFloat();
	BoundingBox.min.z = stream->ReadFloat();
	BoundingBox.max.x = stream->ReadFloat();
	BoundingBox.max.y = stream->ReadFloat();
	BoundingBox.max.z = stream->ReadFloat();
	BoundingBox.IsValid = stream->ReadUInt8() != 0;

	BoundingSphere.x = stream->ReadFloat();
	BoundingSphere.y = stream->ReadFloat();
	BoundingSphere.z = stream->ReadFloat();
	BoundingSphere.w = (stream->GetVersion() > 61) ? stream->ReadFloat() : 0.0f;
}

void UPrimitive::Save(PackageStreamWriter* stream)
{
	UObject::Save(stream);

	stream->WriteFloat(BoundingBox.min.x);
	stream->WriteFloat(BoundingBox.min.y);
	stream->WriteFloat(BoundingBox.min.z);
	stream->WriteFloat(BoundingBox.max.x);
	stream->WriteFloat(BoundingBox.max.y);
	stream->WriteFloat(BoundingBox.max.z);
	stream->WriteUInt8(BoundingBox.IsValid ? 1 : 0);

	stream->WriteFloat(BoundingSphere.x);
	stream->WriteFloat(BoundingSphere.y);
	stream->WriteFloat(BoundingSphere.z);
	if (stream->GetVersion() > 61)
		stream->WriteFloat(BoundingSphere.w);
}
