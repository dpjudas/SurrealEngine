#pragma once

#include "UMesh.h"
#include "Math/bbox.h"
#include "Collision/TopLevel/CollisionSystem.h"
#include "Collision/TopLevel/CollisionHit.h"

class UTexture;
class UActor;
class UPawn;
class UBrush;
class UDecal;
class UZoneInfo;
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
	int BrushActor;
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
	// Regular in-game flags.
	PF_Invisible		= 0x00000001,	// Poly is invisible.
	PF_Masked			= 0x00000002,	// Poly should be drawn masked.
	PF_Translucent	 	= 0x00000004,	// Poly is transparent.
	PF_NotSolid			= 0x00000008,	// Poly is not solid, doesn't block.
	PF_Environment   	= 0x00000010,	// Poly should be drawn environment mapped.
	PF_ForceViewZone	= 0x00000010,	// Force current iViewZone in OccludeBSP (reuse Environment flag)
	PF_Semisolid	  	= 0x00000020,	// Poly is semi-solid = collision solid, Csg nonsolid.
	PF_Modulated 		= 0x00000040,	// Modulation transparency.
	PF_FakeBackdrop		= 0x00000080,	// Poly looks exactly like backdrop.
	PF_TwoSided			= 0x00000100,	// Poly is visible from both sides.
	PF_AutoUPan		 	= 0x00000200,	// Automatically pans in U direction.
	PF_AutoVPan 		= 0x00000400,	// Automatically pans in V direction.
	PF_NoSmooth			= 0x00000800,	// Don't smooth textures.
	PF_BigWavy 			= 0x00001000,	// Poly has a big wavy pattern in it.
	PF_SpecialPoly		= 0x00001000,	// Game-specific poly-level render control (reuse BigWavy flag)
	PF_SmallWavy		= 0x00002000,	// Small wavy pattern (for water/enviro reflection).
	PF_Flat				= 0x00004000,	// Flat surface.
	PF_LowShadowDetail	= 0x00008000,	// Low detaul shadows.
	PF_NoMerge			= 0x00010000,	// Don't merge poly's nodes before lighting when rendering.
	PF_CloudWavy		= 0x00020000,	// Polygon appears wavy like clouds.
	PF_DirtyShadows		= 0x00040000,	// Dirty shadows.
	PF_BrightCorners	= 0x00080000,	// Brighten convex corners.
	PF_SpecialLit		= 0x00100000,	// Only speciallit lights apply to this poly.
	PF_Gouraud			= 0x00200000,	// Gouraud shaded.
	PF_NoBoundRejection = 0x00200000,	// Disable bound rejection in OccludeBSP (reuse Gourard flag)
	PF_Unlit			= 0x00400000,	// Unlit.
	PF_HighShadowDetail	= 0x00800000,	// High detail shadows.
	PF_Portal			= 0x04000000,	// Portal between iZones.
	PF_Mirrored			= 0x08000000,	// Reflective surface.

	// Editor flags.
	PF_Memorized     	= 0x01000000,	// Editor: Poly is remembered.
	PF_Selected      	= 0x02000000,	// Editor: Poly is selected.
	PF_Highlighted      = 0x10000000,	// Editor: Poly is highlighted.   
	PF_FlatShaded		= 0x40000000,	// FPoly has been split by SplitPolyWithPlane.   

	// Internal.
	PF_EdProcessed 		= 0x40000000,	// FPoly was already processed in editorBuildUPolys.
	PF_EdCut       		= 0x80000000,	// FPoly has been split by SplitPolyWithPlane.  
	PF_RenderFog		= 0x40000000,	// Render with fogmapping.
	PF_Occlude			= 0x80000000,	// Occludes even if PF_NoOcclude.
	PF_RenderHINT       = 0x01000000,   // Rendering optimization hINT.

	// Combinations of flags.
	PF_NoOcclude		= PF_Masked | PF_Translucent | PF_Invisible | PF_Modulated,
	PF_NoEdit			= PF_Memorized | PF_Selected | PF_EdProcessed | PF_NoMerge | PF_EdCut,
	PF_NoImport			= PF_NoEdit | PF_NoMerge | PF_Memorized | PF_Selected | PF_EdProcessed | PF_EdCut,
	PF_AddLast			= PF_Semisolid | PF_NotSolid,
	PF_NoAddToBSP		= PF_EdCut | PF_EdProcessed | PF_Selected | PF_Memorized,
	PF_NoShadows		= PF_Unlit | PF_Invisible | PF_Environment | PF_FakeBackdrop,
	PF_Transient		= PF_Highlighted,

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

enum EReachSpecFlags
{
	R_WALK = 1,
	R_FLY = 2,
	R_SWIM = 4,
	R_JUMP = 8,
	R_DOOR = 16,
	R_SPECIAL = 32,
	R_PLAYERONLY = 64
};

class LevelReachSpec
{
public:
	int32_t distance;
	int32_t startActor;
	int32_t endActor;
	int32_t collisionRadius;
	int32_t collisionHeight;
	int32_t reachFlags;
	int8_t bPruned;
};

class ULevelBase : public UObject
{
public:
	using UObject::UObject;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<UActor*> Actors;

	std::string Protocol;
	std::string Host;
	int Port = 0;
	std::string Map;
	Array<std::string> Options;
	std::string Portal;
};

class ULevel : public ULevelBase
{
public:
	ULevel(NameString name, UClass* base, ObjectFlags flags);
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	void Tick(float elapsed, bool gamePaused);

	Array<LevelReachSpec> ReachSpecs;
	UModel* Model = nullptr;

	CollisionSystem Collision;
	std::map<std::string, std::string> TravelInfo;

private:
	void TickActor(float elapsed, UActor* actor);

	bool ticked = false;
};

class ULevelSummary : public UObject
{
public:
	using UObject::UObject;

	std::string& Author() { return Value<std::string>(PropOffsets_LevelSummary.Author); }
	std::string& IdealPlayerCount() { return Value<std::string>(PropOffsets_LevelSummary.IdealPlayerCount); }
	std::string& LevelEnterText() { return Value<std::string>(PropOffsets_LevelSummary.LevelEnterText); }
	int& RecommendedEnemies() { return Value<int>(PropOffsets_LevelSummary.RecommendedEnemies); }
	int& RecommendedTeammates() { return Value<int>(PropOffsets_LevelSummary.RecommendedTeammates); }
	std::string& Title() { return Value<std::string>(PropOffsets_LevelSummary.Title); }
};

struct Poly
{
	vec3 Base;
	vec3 Normal;
	vec3 TextureU;
	vec3 TextureV;
	Array<vec3> Vertices;
	uint32_t PolyFlags;
	UBrush* Actor;
	UTexture* Texture;
	NameString ItemName;
	int LinkIndex;
	int BrushPolyIndex;
	int16_t PanU;
	int16_t PanV;
};

class UPolys : public UObject
{
public:
	using UObject::UObject;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<Poly> Polys;
};

class UBspNodes : public UObject
{
public:
	using UObject::UObject;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<BspNode> Nodes;
	Array<ZoneProperties> Zones;
};

class UBspSurfs : public UObject
{
public:
	using UObject::UObject;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<BspSurface> Surfaces;
};

class UVectors : public UObject
{
public:
	using UObject::UObject;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<vec3> Vectors;
};

class UVerts : public UObject
{
public:
	using UObject::UObject;
	
	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	Array<BspVert> Vertices;
	int32_t NumSharedSides;
};
