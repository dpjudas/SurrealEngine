
#include "Precomp.h"
#include "UAnimation.h"
#include "Engine.h"

void UAnimation::Load(ObjectStream* stream)
{
	UObject::Load(stream);

	if (engine->LaunchInfo.IsHarryPotter1() || engine->LaunchInfo.IsHarryPotter2())
		return; // To do: format changed. Figure out how. Maybe the arrays became skippable?

	int NumRefBones = stream->ReadIndex();
	for (int i = 0; i < NumRefBones; i++)
	{
		RefBone refbone;
		refbone.Name = stream->ReadName();
		refbone.Flags = stream->ReadUInt32();
		refbone.ParentIndex = stream->ReadUInt32();
		RefBones.push_back(refbone);
	}

	int NumMoves = stream->ReadIndex();
	for (int i = 0; i < NumMoves; i++)
	{
		AnimMove move;
		move.RootSpeed3D.x = stream->ReadFloat();
		move.RootSpeed3D.y = stream->ReadFloat();
		move.RootSpeed3D.z = stream->ReadFloat();
		move.TrackTime = stream->ReadFloat();
		move.StartBone = stream->ReadUInt32();
		move.Flags = stream->ReadUInt32();

		int NumBoneIndices = stream->ReadIndex();
		for (int j = 0; j < NumBoneIndices; j++)
			move.BoneIndices.push_back(stream->ReadUInt32());

		int NumAnimTracks = stream->ReadIndex();
		for (int j = 0; j < NumAnimTracks; j++)
		{
			AnimTrack track;
			track.Flags = stream->ReadUInt32();

			int NumKeyQuat = stream->ReadIndex();
			for (int k = 0; k < NumKeyQuat; k++)
			{
				quaternion q;
				q.x = stream->ReadFloat();
				q.y = stream->ReadFloat();
				q.z = stream->ReadFloat();
				q.w = stream->ReadFloat();
				track.KeyQuat.push_back(q);
			}

			int NumKeyPos = stream->ReadIndex();
			for (int k = 0; k < NumKeyPos; k++)
			{
				vec3 pos;
				pos.x = stream->ReadFloat();
				pos.y = stream->ReadFloat();
				pos.z = stream->ReadFloat();
				track.KeyPos.push_back(pos);
			}

			int NumKeyTime = stream->ReadIndex();
			for (int k = 0; k < NumKeyTime; k++)
				track.KeyTime.push_back(stream->ReadFloat());

			move.AnimTracks.push_back(track);
		}

		move.RootTrack.Flags = stream->ReadUInt32();

		int NumKeyQuat = stream->ReadIndex();
		for (int k = 0; k < NumKeyQuat; k++)
		{
			quaternion q;
			q.x = stream->ReadFloat();
			q.y = stream->ReadFloat();
			q.z = stream->ReadFloat();
			q.w = stream->ReadFloat();
			move.RootTrack.KeyQuat.push_back(q);
		}

		int NumKeyPos = stream->ReadIndex();
		for (int k = 0; k < NumKeyPos; k++)
		{
			vec3 pos;
			pos.x = stream->ReadFloat();
			pos.y = stream->ReadFloat();
			pos.z = stream->ReadFloat();
			move.RootTrack.KeyPos.push_back(pos);
		}

		int NumKeyTime = stream->ReadIndex();
		for (int k = 0; k < NumKeyTime; k++)
			move.RootTrack.KeyTime.push_back(stream->ReadFloat());

		Moves.push_back(move);
	}
}

void UAnimation::Save(PackageStreamWriter* stream)
{
	UObject::Save(stream);

	stream->WriteIndex((int)RefBones.size());
	for (const RefBone& refbone : RefBones)
	{
		stream->WriteName(refbone.Name);
		stream->WriteUInt32(refbone.Flags);
		stream->WriteUInt32(refbone.ParentIndex);
	}

	stream->WriteIndex((int)Moves.size());
	for (const AnimMove& move : Moves)
	{
		stream->WriteFloat(move.RootSpeed3D.x);
		stream->WriteFloat(move.RootSpeed3D.y);
		stream->WriteFloat(move.RootSpeed3D.z);
		stream->WriteFloat(move.TrackTime);
		stream->WriteUInt32(move.StartBone);
		stream->WriteUInt32(move.Flags);

		stream->WriteIndex((int)move.BoneIndices.size());
		for (uint32_t index : move.BoneIndices)
			stream->WriteUInt32(index);

		stream->WriteIndex((int)move.AnimTracks.size());
		for (const AnimTrack& track : move.AnimTracks)
		{
			stream->WriteUInt32(track.Flags);

			stream->WriteIndex((int)track.KeyQuat.size());
			for (const quaternion& q : track.KeyQuat)
			{
				stream->WriteFloat(q.x);
				stream->WriteFloat(q.y);
				stream->WriteFloat(q.z);
				stream->WriteFloat(q.w);
			}

			stream->WriteIndex((int)track.KeyPos.size());
			for (const vec3& pos : track.KeyPos)
			{
				stream->WriteFloat(pos.x);
				stream->WriteFloat(pos.y);
				stream->WriteFloat(pos.z);
			}

			stream->WriteIndex((int)track.KeyTime.size());
			for (float v : track.KeyTime)
				stream->WriteFloat(v);
		}

		stream->WriteUInt32(move.RootTrack.Flags);

		stream->WriteIndex((int)move.RootTrack.KeyQuat.size());
		for (const quaternion& q : move.RootTrack.KeyQuat)
		{
			stream->WriteFloat(q.x);
			stream->WriteFloat(q.y);
			stream->WriteFloat(q.z);
			stream->WriteFloat(q.w);
		}

		stream->WriteIndex((int)move.RootTrack.KeyPos.size());
		for (const vec3& pos : move.RootTrack.KeyPos)
		{
			stream->WriteFloat(pos.x);
			stream->WriteFloat(pos.y);
			stream->WriteFloat(pos.z);
		}

		stream->WriteIndex((int)move.RootTrack.KeyTime.size());
		for (float v : move.RootTrack.KeyTime)
			stream->WriteFloat(v);

		Moves.push_back(move);
	}
}
