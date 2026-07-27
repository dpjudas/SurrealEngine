#pragma once

#include "Packages/Core/UObject.h"
#include "Math/quaternion.h"

struct RefBone
{
	NameString Name;
	uint32_t Flags = 0;
	uint32_t ParentIndex = 0;
};

struct AnimTrack
{
	uint32_t Flags = 0;
	Array<quaternion> KeyQuat;
	Array<vec3> KeyPos;
	Array<float> KeyTime;
};

struct AnimMove
{
	vec3 RootSpeed3D;
	float TrackTime = 0.0f;
	uint32_t StartBone = 0;
	uint32_t Flags = 0;
	Array<uint32_t> BoneIndices;
	Array<AnimTrack> AnimTracks;
	AnimTrack RootTrack;
};

class UAnimation : public UObject
{
public:
	using UObject::UObject;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<RefBone> RefBones;
	Array<AnimMove> Moves;
};
