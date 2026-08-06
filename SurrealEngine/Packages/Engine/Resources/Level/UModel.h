#pragma once

#include "Packages/Engine/Resources/UPrimitive.h"

class UDecal;
class UTexture;
class UActor;
class UZoneInfo;
class UVectors;
class UBspNodes;
class UBspSurfs;
class UVerts;
class UObject;
class UPolys;
struct PointRegion;

enum EBspNodeFlags
{
	NF_NotCsg = 1,
	NF_ShootThrough = 2,
	NF_NotVisBlocking = 4
};

struct LevelDecal
{
	UDecal* Decal = nullptr;
	Array<vec3> Positions;
	Array<vec2> UVs;
};

class UModel;
class BspVert;
class BspNode
{
public:
	float PlaneX;
	float PlaneY;
	float PlaneZ;
	float PlaneW;
	uint64_t ZoneMask;
	uint8_t NodeFlags;
	int VertPool;
	int Surf;
	int Back;
	int Front;
	int Plane;
	int CollisionBound;
	int RenderBound;
	int Zone0;
	int Zone1;
	uint8_t NumVertices;
	int32_t Leaf0;
	int32_t Leaf1;

	BBox GetCollisionBox(UModel* model) const;

	UActor* ActorList = nullptr;
	Array<LevelDecal> Decals;
};

class BspSurface
{
public:
	UTexture* Material;
	uint32_t PolyFlags;
	int pBase;
	int vNormal;
	int vTextureU;
	int vTextureV;
	int LightMap;
	int BrushPoly;
	int16_t PanU;
	int16_t PanV;
	UActor* BrushActor = nullptr;

	vec3 Center;
	float Radius = 0.0f;
};

class BspVert
{
public:
	int Vertex;
	int Side;
};

class ZoneProperties
{
public:
	UActor* ZoneActor;
	uint64_t Connectivity;
	uint64_t Visibility;
};

class LightMapIndex
{
public:
	int32_t DataOffset;
	float PanX;
	float PanY;
	float PanZ;
	int UClamp;
	int VClamp;
	float UScale;
	float VScale;
	int LightActors;

	uint32_t LMCacheID = 0;
};

class ConvexVolumeLeaf
{
public:
	int Zone;
	int Permeating;
	int Volumetric;
	uint64_t VisibleZones;
};

enum PolyFlags
{
	PF_Invisible = 0x00000001,
	PF_Masked = 0x00000002,
	PF_Translucent = 0x00000004,
	PF_NotSolid = 0x00000008,
	PF_Environment = 0x00000010,
	PF_ForceViewZone = 0x00000010,
	PF_Semisolid = 0x00000020,
	PF_Modulated = 0x00000040,
	PF_FakeBackdrop = 0x00000080,
	PF_TwoSided = 0x00000100,
	PF_AutoUPan = 0x00000200,
	PF_AutoVPan = 0x00000400,
	PF_NoSmooth = 0x00000800,
	PF_BigWavy = 0x00001000,
	PF_SpecialPoly = 0x00001000,
	PF_SmallWavy = 0x00002000,
	PF_Flat = 0x00004000,
	PF_LowShadowDetail = 0x00008000,
	PF_NoMerge = 0x00010000,
	PF_CloudWavy = 0x00020000,
	PF_DirtyShadows = 0x00040000,
	PF_BrightCorners = 0x00080000,
	PF_SpecialLit = 0x00100000,
	PF_Gouraud = 0x00200000,
	PF_NoBoundRejection = 0x00200000,
	PF_Unlit = 0x00400000,
	PF_HighShadowDetail = 0x00800000,
	PF_Portal = 0x04000000,
	PF_Mirrored = 0x08000000,

	PF_Memorized = 0x01000000,
	PF_Selected = 0x02000000,
	PF_Highlighted = 0x10000000,
	PF_FlatShaded = 0x40000000,

	PF_EdProcessed = 0x40000000,
	PF_EdCut = 0x80000000,
	PF_RenderFog = 0x40000000,
	PF_Occlude = 0x80000000,
	PF_RenderHINT = 0x01000000,

	PF_NoOcclude = PF_Masked | PF_Translucent | PF_Invisible | PF_Modulated,
	PF_NoEdit = PF_Memorized | PF_Selected | PF_EdProcessed | PF_NoMerge | PF_EdCut,
	PF_NoImport = PF_NoEdit | PF_NoMerge | PF_Memorized | PF_Selected | PF_EdProcessed | PF_EdCut,
	PF_AddLast = PF_Semisolid | PF_NotSolid,
	PF_NoAddToBSP = PF_EdCut | PF_EdProcessed | PF_Selected | PF_Memorized,
	PF_NoShadows = PF_Unlit | PF_Invisible | PF_Environment | PF_FakeBackdrop,
	PF_Transient = PF_Highlighted,

	PF_SubpixelFont = 0xffffffff // For drawing fonts with subpixels
};

inline uint32_t ApplyPrecedenceRules(uint32_t PolyFlags)
{
	if (PolyFlags == PF_SubpixelFont)
		return PolyFlags;

	// Adjust PolyFlags according to Unreal's precedence rules.
	if (!(PolyFlags & (PF_Translucent | PF_Modulated)))
		PolyFlags |= PF_Occlude;
	else if (PolyFlags & PF_Translucent)
		PolyFlags &= ~PF_Masked;
	return PolyFlags;
}

enum LineFlags
{
	LINE_None = 0,
	LINE_DepthCued = 1
};

class UModel : public UPrimitive
{
public:
	using UPrimitive::UPrimitive;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	PointRegion FindRegion(const vec3& point, UZoneInfo* levelZoneInfo);

	Array<vec3> Vectors;
	Array<vec3> Points;
	Array<BspNode> Nodes;
	Array<BspSurface> Surfaces;
	Array<BspVert> Vertices;

	struct
	{
		UVectors* Vectors = nullptr;
		UVectors* Points = nullptr;
		UBspNodes* Nodes = nullptr;
		UBspSurfs* Surfaces = nullptr;
		UVerts* Verts = nullptr;
		UObject* Unknown1 = nullptr;
		UObject* Unknown2 = nullptr;
	} OldFormat;

	int32_t NumSharedSides;

	Array<ZoneProperties> Zones;

	UPolys* Polys = nullptr;

	Array<LightMapIndex> LightMap;
	Array<uint8_t> LightBits;

	Array<BBox> Bounds;
	Array<int32_t> LeafHulls;
	Array<ConvexVolumeLeaf> Leaves;

	Array<UActor*> Lights;

	int32_t RootOutside;
	int32_t Linked;
};
