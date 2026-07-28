
#include "Precomp.h"
#include "UActor.h"
#include "Packages/Engine/Actors/Brush/UMover.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Packages/Engine/Resources/Mesh/USkeletalMesh.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"

void UActor::UpdateBspInfo()
{
	// Figure out where the actor is visually located in the world
	BBox bbox;
	EDrawType dt = (EDrawType)DrawType();
	if (dt == DT_Mesh && Mesh())
	{
		UMesh* mesh = Mesh();
		Coords rotation = Coords::Rotation(Rotation());
		mat4 objectToWorld = mat4::translate(Location() + PrePivot()) * Coords::Rotation(Rotation()).ToMatrix() * mat4::scale(DrawScale());
		mat4 meshToWorld = objectToWorld * mesh->meshToObject;
		bbox = mesh->BoundingBox.transform(meshToWorld);
	}
	else if ((dt == DT_Sprite || dt == DT_SpriteAnimOnce) && (Texture()))
	{
		vec3 location = Location();
		auto texWidth = Texture()->UsedMipmaps[0].Width;
		auto texHeight = Texture()->UsedMipmaps[0].Height;
		// vec3 extents = vec3(100.0f); // To do: this is wrong. We need the size of a sprite
		vec3 extents = vec3(std::max(texWidth, texHeight) * 0.5f * DrawScale());
		bbox.min = location - extents;
		bbox.max = location + extents;
	}
	else if (dt == DT_Brush && Brush())
	{
		UModel* brush = Brush();
		if (UMover* mover = UObject::TryCast<UMover>(this))
		{
			mat4 objectToWorld = mat4::translate(Location()) * Coords::Rotation(Rotation()).ToMatrix() * mat4::scale(mover->MainScale().Scale) * mat4::translate(-PrePivot());
			bbox = brush->BoundingBox.transform(objectToWorld);
		}
		else
		{
			bbox.min = vec3(0.0f);
			bbox.max = vec3(0.0f);
		}
	}
	else
	{
		bbox.min = vec3(0.0f);
		bbox.max = vec3(0.0f);
	}

	// Is actor still in the bsp tree at the correct location?
	if (!BspInfo.Node || BspInfo.BoundingBox != bbox)
	{
		RemoveFromBspNode();

		BspInfo.BoundingBox = bbox;

		vec3 location = bbox.center();
		vec3 extents = bbox.extents();

		ULevel* level = XLevel();
		BspNode* node = level ? &level->Model->Nodes[0] : nullptr;
		while (node)
		{
			int side = NodeAABBOverlap(location, extents, node);
			if (side == 0 || (side < 0 && node->Front < 0) || (side > 0 && node->Back < 0))
			{
				AddToBspNode(node);
				break;
			}
			else if (side < 0)
			{
				node = &level->Model->Nodes[node->Front];
			}
			else
			{
				node = &level->Model->Nodes[node->Back];
			}
		}
	}
}

void UActor::AddToBspNode(BspNode* node)
{
	BspInfo.Node = node;

	if (node->ActorList)
	{
		node->ActorList->BspInfo.Prev = this;
		BspInfo.Next = node->ActorList;
	}

	node->ActorList = this;
}

void UActor::RemoveFromBspNode()
{
	if (BspInfo.Node)
	{
		if (BspInfo.Next)
		{
			BspInfo.Next->BspInfo.Prev = BspInfo.Prev;
		}
		if (BspInfo.Prev)
		{
			BspInfo.Prev->BspInfo.Next = BspInfo.Next;
		}
		if (BspInfo.Node->ActorList == this)
		{
			BspInfo.Node->ActorList = BspInfo.Next;
		}
		BspInfo.Node = nullptr;
		BspInfo.Prev = nullptr;
		BspInfo.Next = nullptr;
	}
}

// -1 = inside, 0 = intersects, 1 = outside
int UActor::NodeAABBOverlap(const vec3& center, const vec3& extents, BspNode* node)
{
	float e = extents.x * std::abs(node->PlaneX) + extents.y * std::abs(node->PlaneY) + extents.z * std::abs(node->PlaneZ);
	float s = center.x * node->PlaneX + center.y * node->PlaneY + center.z * node->PlaneZ - node->PlaneW;
	if (s - e > 0.0f)
		return -1;
	else if (s + e < 0.0f)
		return 1;
	else
		return 0;
}
