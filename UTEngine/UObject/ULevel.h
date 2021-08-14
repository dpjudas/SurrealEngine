#pragma once

#include "UMesh.h"
#include "Math/bbox.h"
#include <list>

class UTexture;
class UActor;
class UPawn;
class UBrush;

enum EBspNodeFlags
{
	NF_NotCsg = 1,
	NF_ShootThrough = 2,
	NF_NotVisBlocking = 4
};

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
};

class UModel : public UPrimitive
{
public:
	using UPrimitive::UPrimitive;
	void Load(ObjectStream* stream) override;

	std::vector<vec3> Vectors;
	std::vector<vec3> Points;
	std::vector<BspNode> Nodes;
	std::vector<BspSurface> Surfaces;
	std::vector<BspVert> Vertices;

	int32_t NumSharedSides;

	std::vector<ZoneProperties> Zones;

	int Polys;

	std::vector<LightMapIndex> LightMap;
	std::vector<uint8_t> LightBits;

	std::vector<BBox> Bounds;
	std::vector<int32_t> LeafHulls;
	std::vector<ConvexVolumeLeaf> Leaves;

	std::vector<UActor*> Lights;

	int32_t RootOutside;
	int32_t Linked;

	std::map<int, UTexture*> lmtextures;
	std::map<int, std::pair<int, UTexture*>> fogtextures;
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

	std::vector<UActor*> Actors;

	std::string Protocol;
	std::string Host;
	int Port = 0;
	std::string Map;
	std::vector<std::string> Options;
	std::string Portal;
};

class TraceFilter
{
public:
	TraceFilter() = default;
	TraceFilter(UActor* tracingActor, bool traceActors, bool traceWorld) : TracingActor(tracingActor), TraceActors(traceActors), TraceWorld(traceWorld) { }

	UActor* TracingActor = nullptr;
	bool TraceActors = false;
	bool TraceWorld = false;
};

class SweepHit
{
public:
	float Fraction = 1.0;
	vec3 Normal = vec3(0.0);
	UActor* Actor = nullptr;
};

class SphereSweep
{
public:
	SphereSweep() = default;
	SphereSweep(const dvec3& from, const dvec3& to, double radius, TraceFilter filter)
	{
		From = from;
		To = to;
		Radius = radius;
		Filter = filter;
		From4 = dvec4(From, 1.0);
		To4 = dvec4(To, 1.0);
	}

	dvec3 From = dvec3(0.0);
	dvec3 To = dvec3(0.0);
	double Radius = 0.0;
	TraceFilter Filter;
	SweepHit Hit;

	dvec4 From4 = dvec4(0.0, 0.0, 0.0, 1.0);
	dvec4 To4 = dvec4(0.0, 0.0, 0.0, 1.0);
};

class CylinderSweep
{
public:
	CylinderSweep() = default;
	CylinderSweep(const vec3& from, const vec3& to, float height, float radius, UActor* tracingActor, bool traceActors, bool traceWorld)
	{
		From = dvec3(from.x, from.y, from.z);
		To = dvec3(to.x, to.y, to.z);
		Height = height;
		Radius = radius;
		Filter = TraceFilter(tracingActor, traceActors, traceWorld);
	}

	dvec3 From = dvec3(0.0);
	dvec3 To = dvec3(0.0);
	double Height = 0.0;
	double Radius = 0.0;
	TraceFilter Filter;
	SweepHit Hit;
};

class ULevel : public ULevelBase
{
public:
	using ULevelBase::ULevelBase;
	void Load(ObjectStream* stream) override;

	void Tick(float elapsed);

	std::vector<LevelReachSpec> ReachSpecs;
	UModel* Model = nullptr;

	std::vector<std::list<UActor*>> CollisionActors;

	void AddToCollision(UActor* actor);
	void RemoveFromCollision(UActor* actor);

	bool TraceAnyHit(vec3 from, vec3 to, UActor* tracingActor, bool traceActors, bool traceWorld);
	SweepHit Sweep(const vec3& from, const vec3& to, float height, float radius, UActor* tracingActor, bool traceActors, bool traceWorld);

private:
	void AddToCollision(UActor* actor, const vec3& location, const vec3& extents, BspNode* node);
	void RemoveFromCollision(UActor* actor, const vec3& location, const vec3& extents, BspNode* node);
	int NodeAABBOverlap(const vec3& center, const vec3& extents, BspNode* node);
	double ActorRayIntersect(const dvec3& from, const dvec3& to, UActor* actor);
	double ActorSphereIntersect(const dvec3& from, const dvec3& to, double radius, UActor* actor);
	double RaySphereIntersect(const dvec3& rayOrigin, const dvec3& rayDir, const dvec3& sphereCenter, double sphereRadius);

	bool TraceAnyHit(const dvec4& from, const dvec4& to, UActor* tracingActor, bool traceActors, bool traceWorld, BspNode* node);
	void Sweep(SphereSweep* sphere, BspNode* node);

	double NodeRayIntersect(const dvec4& from, const dvec4& to, BspNode* node);
	double NodeSphereIntersect(const dvec4& from, const dvec4& to, double radius, BspNode* node);

	double TriangleRayIntersect(const dvec4& from, const dvec4& to, const dvec3* points);
	double TriangleSphereIntersect(const dvec4& from, const dvec4& to, double radius, const dvec3* points);

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
	std::vector<vec3> Vertices;
	uint32_t PolyFlags;
	UBrush* Actor;
	UTexture* Texture;
	std::string ItemName;
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

	std::vector<Poly> Polys;
};

class UBspNodes : public UObject
{
public:
	using UObject::UObject;
	void Load(ObjectStream* stream) override;

	std::vector<BspNode> Nodes;
	std::vector<ZoneProperties> Zones;
};

class UBspSurfs : public UObject
{
public:
	using UObject::UObject;
	void Load(ObjectStream* stream) override;

	std::vector<BspSurface> Surfaces;
};

class UVectors : public UObject
{
public:
	using UObject::UObject;
	void Load(ObjectStream* stream) override;

	std::vector<vec3> Vectors;
};

class UVerts : public UObject
{
public:
	using UObject::UObject;
	void Load(ObjectStream* stream) override;

	std::vector<BspVert> Vertices;
	int32_t NumSharedSides;
};
