
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "GameWindow.h"
#include "UObject/USubsystem.h"
#include "VM/ScriptCall.h"
#include "Engine.h"

RenderSubsystem::RenderSubsystem(RenderDevice* renderdevice) : Device(renderdevice)
{
}

void RenderSubsystem::DrawGame(float levelTimeElapsed)
{
	LevelTimeElapsed = levelTimeElapsed;
	AutoUV += levelTimeElapsed * 64.0f;
	AmbientGlowTime = std::fmod(AmbientGlowTime + 0.8f * levelTimeElapsed, 1.0f);
	AmbientGlowAmount = 0.20f + 0.20f * std::sin(radians(AmbientGlowTime * 360.0f));

	Stats.Frames = 0;
	Stats.Surfaces = 0;
	Stats.Actors = 0;

	vec3 flashScale = 0.5f;
	vec3 flashFog = vec3(0.0f, 0.0f, 0.0f);

	UPlayerPawn* player = UObject::TryCast<UPlayerPawn>(engine->CameraActor);
	if (player)
	{
		flashScale = player->FlashScale();
		flashFog = player->FlashFog();
	}

	Device->Brightness = engine->client->Brightness;

	VRSubsystem* vr = engine->vr.get();
	bool drewVREyes = false;
	if (vr && vr->IsActive() && vr->WaitFrame())
	{
		vr->BeginFrame();

		VRSubsystem::EyeView eyeViews[VRSubsystem::EyeCount];
		if (vr->LocateViews(eyeViews))
		{
			// While the console/menu has taken over the whole frame, render it once (not per-eye) onto an
			// offscreen canvas and draw that as a single world-space plane per eye instead of the normal
			// flat-2D-onto-the-eye-viewport path - see DrawUICanvas()/DrawVRMenuPlane() for why.
			bool menuOpen = engine->console->bNoDrawWorld();
			if (menuOpen)
			{
				if (!WasVRMenuOpen)
					CaptureVRMenuPlaneAnchor(eyeViews);
				DrawUICanvas();
			}
			WasVRMenuOpen = menuOpen;

			for (int eye = 0; eye < VRSubsystem::EyeCount; eye++)
			{
				CurrentVREye = &eyeViews[eye];
				Device->BeginEyeFrame(eye);
				if (menuOpen)
					DrawVRMenuEyeFrame(vec4(flashScale, 1.0f), vec4(flashFog, 1.0f), eye == 0);
				else
					DrawGameFrame(vec4(flashScale, 1.0f), vec4(flashFog, 1.0f), eye == 0);
				Device->EndEyeFrame(eye);
			}
			CurrentVREye = nullptr;
			drewVREyes = true;
		}

		vr->EndFrame();
	}

	if (!drewVREyes)
	{
		// Either VR is off, or it is on but the runtime had no head pose to give us this frame (tracking
		// lost, or the session isn't visible). Fall back to drawing the desktop frame rather than leaving
		// the window frozen on whatever it last showed.
		WasVRMenuOpen = false;
		DrawGameFrame(vec4(flashScale, 1.0f), vec4(flashFog, 1.0f), true);
	}
}

void RenderSubsystem::DrawGameFrame(vec4 flashScale, vec4 flashFog, bool presentToDesktop)
{
	Device->Lock(flashScale, flashFog, vec4(0.0f), nullptr, nullptr);

	ResetCanvas();
	PreRender();

	if (engine->LaunchInfo.engineVersion <= 219 || engine->console->bNoDrawWorld() == false)
	{
		DrawScene();
		RenderOverlays();
		if (engine->LaunchInfo.IsDeusEx())
			PostRenderFlash();
		Device->EndFlash();
	}

	PostRender();

	Device->Unlock(presentToDesktop);
}

void RenderSubsystem::DrawEditorViewport()
{
	Device->Brightness = engine->client->Brightness;
	DrawScene();
}

void RenderSubsystem::DrawVideoFrame(FTextureInfo* frame, FTextureInfo* background)
{
	vec3 flashScale = 0.5f;
	vec3 flashFog = vec3(0.0f, 0.0f, 0.0f);
	Device->Brightness = 0.4f;// engine->client->Brightness;
	Device->Lock(vec4(flashScale, 1.0f), vec4(flashFog, 1.0f), vec4(0.0f), nullptr, nullptr);
	ResetCanvas();
	Device->SetSceneNode(&Canvas.Frame);

	float sizeX = (float)(int)(engine->viewport->ViewportWidth() / (float)Canvas.uiscale);
	float sizeY = (float)(int)(engine->viewport->ViewportHeight() / (float)Canvas.uiscale);

	Rectf clipBox = Rectf::xywh(0.0f, 0.0f, sizeX, sizeY);
	Rectf dest = clipBox;

	if (frame)
	{
		Rectf src = Rectf::xywh(0.0f, 0.0f, (float)frame->USize, (float)frame->VSize);
		DrawTile(*frame, dest, src, clipBox, 1.0f, vec4(1.0f), vec4(0.0f), PF_TwoSided);
	}

	if (background)
	{
		Rectf src = Rectf::xywh(0.0f, 0.0f, (float)background->USize, (float)background->VSize);
		DrawTile(*background, dest, src, clipBox, 1.0f, vec4(1.0f), vec4(0.0f), PF_TwoSided | PF_Highlighted);
	}

	Device->EndFlash();
	Device->Unlock(true);
}

void RenderSubsystem::UpdateTexture(UTexture* tex)
{
	if (tex && tex->FrameCounter != TextureFrameCounter)
	{
		tex->Update(LevelTimeElapsed);
		tex->FrameCounter = TextureFrameCounter;
	}
}

void RenderSubsystem::UpdateTextureInfo(FTextureInfo& info, BspSurface& surface, UTexture* texture, float ZoneUPanSpeed, float ZoneVPanSpeed)
{
	UpdateTextureInfo(info, texture);

	info.Pan.x = -(float)surface.PanU;
	info.Pan.y = -(float)surface.PanV;
	if (surface.PolyFlags & PF_AutoUPan) info.Pan.x -= AutoUV * ZoneUPanSpeed;
	if (surface.PolyFlags & PF_AutoVPan) info.Pan.y -= AutoUV * ZoneVPanSpeed;
}

void RenderSubsystem::UpdateTextureInfo(FTextureInfo& info, const Poly& poly, UTexture* texture, float ZoneUPanSpeed, float ZoneVPanSpeed)
{
	UpdateTextureInfo(info, texture);

	info.Pan.x = -(float)poly.PanU;
	info.Pan.y = -(float)poly.PanV;
	if (poly.PolyFlags & PF_AutoUPan) info.Pan.x -= AutoUV * ZoneUPanSpeed;
	if (poly.PolyFlags & PF_AutoVPan) info.Pan.y -= AutoUV * ZoneVPanSpeed;
}

void RenderSubsystem::UpdateTextureInfo(FTextureInfo& info, UTexture* texture)
{
	info.Texture = texture;
	info.CacheID = (uint64_t)(ptrdiff_t)texture;

	if (!info.Texture)
		return;

	info.UScale = texture->DrawScale();
	info.VScale = texture->DrawScale();
	info.Format = texture->UsedFormat;
	info.Mips = texture->UsedMipmaps.data();
	info.NumMips = (int)texture->UsedMipmaps.size();
	info.USize = texture->USize();
	info.VSize = texture->VSize();
	if (texture->Palette())
		info.Palette = (FColor*)texture->Palette()->Colors.data();

	info.bRealtimeChanged = texture->TextureModified;
	if (texture->TextureModified)
		texture->TextureModified = false;
}

void RenderSubsystem::DrawUICanvas()
{
	DrawingVRMenuCanvas = true;
	Device->BeginUICanvasFrame();

	Device->Lock(vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), nullptr, nullptr);
	ResetCanvas();
	PreRender();
	PostRender();
	Device->Unlock(false);

	Device->EndUICanvasFrame();
	DrawingVRMenuCanvas = false;
}

void RenderSubsystem::CaptureVRMenuPlaneAnchor(const VRSubsystem::EyeView eyeViews[2])
{
	// eyeViews[].Position/Forward are relative to the camera's own (possibly moving/turning) local frame,
	// in SurrealEngine's world axis convention (X forward, Y right, Z up) - see VRSubsystem.h. Average both
	// eyes for a head-centered anchor, then rotate into world space the same way UnrealScript's well-known
	// GetAxes() does it (Coords::Rotation(rotator).XAxis/YAxis/ZAxis *is* the world-space forward/right/up
	// for that rotator - confirmed via NObject::GetAxes), applied here as a linear combination instead of
	// removed like DrawScene()'s worldToView does, since we want the opposite direction: local vector ->
	// world vector, not world point -> local point.
	//
	// Deliberately use a LEVELED (yaw-only) camera rotation for this, not the camera's actual pitch/roll -
	// a scripted cinematic camera (e.g. the intro flythrough) can be steeply pitched/rolled to frame
	// architecture, which isn't a stand-in for how a person holds their head. This same leveled rotation is
	// also what gets frozen as the reference frame the menu is rendered against (see
	// VRMenuFrozenCameraLocation/Rotation) - freezing the camera's *actual* tilt there too would leave that
	// tilt baked into the view (and hence how "up" reads on screen) for as long as the menu stays open,
	// which is what made the plane look subtly rotated instead of upright.
	Rotator levelRotation = engine->CameraRotation;
	levelRotation.Pitch = 0;
	levelRotation.Roll = 0;
	VRMenuFrozenCameraLocation = engine->CameraLocation;
	VRMenuFrozenCameraRotation = levelRotation;
	Coords levelCameraRotation = Coords::Rotation(levelRotation);
	auto localToWorldDir = [&levelCameraRotation](const vec3& v)
	{
		return levelCameraRotation.XAxis * v.x + levelCameraRotation.YAxis * v.y + levelCameraRotation.ZAxis * v.z;
	};

	vec3 localPos = (eyeViews[0].Position + eyeViews[1].Position) * 0.5f;
	vec3 localForward = normalize(eyeViews[0].Forward + eyeViews[1].Forward);

	vec3 worldGazeDir = normalize(localToWorldDir(localForward));
	vec3 worldEyePos = engine->CameraLocation + localToWorldDir(localPos);
	// engine->CameraLocation already represents eye level for the current camera (that's what every other
	// VR view in this renderer is built from - see BuildVREyeViewProjection). Adding the headset's own
	// height-above-its-calibration-point on top of that (via localPos.z above) double-counts height and is
	// what made the plane spawn noticeably too high; keep the horizontal (X/Y) head offset but not this one.
	worldEyePos.z = engine->CameraLocation.z;

	// Flatten pitch/roll out of the gaze direction too (in case the player's own head is tilted) and keep
	// the plane perfectly level/upright, anchored at eye height.
	vec3 worldForward = vec3(worldGazeDir.x, worldGazeDir.y, 0.0f);
	if (length(worldForward) < 0.01f) // looking almost straight up/down - fall back to camera facing
		worldForward = vec3(levelCameraRotation.XAxis.x, levelCameraRotation.XAxis.y, 0.0f);
	worldForward = normalize(worldForward);

	const vec3 worldUpAxis(0.0f, 0.0f, 1.0f);
	vec3 worldRight = normalize(cross(worldUpAxis, worldForward));

	const float planeDistance = 1.5f * MetersToUnrealUnits;
	const float planeWidth = 1.6f * MetersToUnrealUnits;
	// Derived from the canvas rather than picked independently, so the menu can't end up stretched if
	// the canvas resolution is ever changed.
	const float planeHeight = planeWidth * (float)RenderDevice::VRMenuCanvasHeight / (float)RenderDevice::VRMenuCanvasWidth;

	vec3 center = worldEyePos + worldForward * planeDistance;
	vec3 halfRight = worldRight * (planeWidth * 0.5f);
	vec3 halfUp = worldUpAxis * (planeHeight * 0.5f);

	// Order matches VRMenuPlaneUVs (0,0)/(1,0)/(1,1)/(0,1): top-left, top-right, bottom-right, bottom-left.
	VRMenuPlaneCorners[0] = center - halfRight + halfUp;
	VRMenuPlaneCorners[1] = center + halfRight + halfUp;
	VRMenuPlaneCorners[2] = center + halfRight - halfUp;
	VRMenuPlaneCorners[3] = center - halfRight - halfUp;
}

void RenderSubsystem::DrawVRMenuPlane()
{
	if (!CurrentVREye)
		return;

	mat4 worldToView, providedProjection;
	Coords head;
	BuildVREyeViewProjection(worldToView, providedProjection, head, VRMenuFrozenCameraLocation, VRMenuFrozenCameraRotation);

	FSceneNode frame;
	frame.XB = 0;
	frame.YB = 0;
	frame.X = engine->vr->GetRecommendedEyeWidth();
	frame.Y = engine->vr->GetRecommendedEyeHeight();
	frame.FX = (float)frame.X;
	frame.FY = (float)frame.Y;
	frame.FX2 = frame.FX * 0.5f;
	frame.FY2 = frame.FY * 0.5f;
	frame.ObjectToWorld = mat4::identity();
	frame.WorldToView = worldToView;
	frame.FovAngle = engine->CameraFovAngle;
	frame.UseProvidedProjection = true;
	frame.Projection = providedProjection;

	Device->DrawVRMenuPlane(&frame, VRMenuPlaneCorners, VRMenuPlaneUVs);
}

void RenderSubsystem::DrawVRMenuEyeFrame(vec4 flashScale, vec4 flashFog, bool presentToDesktop)
{
	Device->Lock(flashScale, flashFog, vec4(0.0f, 0.0f, 0.0f, 1.0f), nullptr, nullptr);
	DrawVRMenuPlane();
	Device->Unlock(presentToDesktop);
}

void RenderSubsystem::OnMapLoaded()
{
	Device->Flush(true);

	Light.FogBalls.clear();
	Light.lmtextures.clear();
	Light.fogtextures.clear();

	std::set<UActor*> lightset;
	for (UActor* light : engine->Level->Model->Lights)
	{
		if (light)
			lightset.insert(light);
	}

	for (UActor* light : lightset)
	{
		if (light->VolumeRadius() != 0)
			Light.FogBalls.push_back(light);
	}
}
