
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "GameWindow.h"
#include "UObject/USubsystem.h"
#include "VM/ScriptCall.h"
#include "Engine.h"
#include "LauncherSettings.h"
#include "VR/VRPlayerInput.h"

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
				// Anchor the plane once, on the transition into the menu, and hold it until the menu closes.
				// Tracked by "have we captured an anchor for this menu session" rather than "was the menu
				// open last frame" specifically so a one-frame VR dropout (tracking lost, session briefly
				// not visible) doesn't re-anchor the plane to the new gaze mid-menu - the dropped frame draws
				// the desktop fallback below without touching this, so the frozen anchor survives it.
				if (!VRMenuAnchorCaptured)
				{
					CaptureVRMenuPlaneAnchor(eyeViews);
					VRMenuAnchorCaptured = true;
				}
				UpdateVRMenuLaser();
				DrawUICanvas(false); // opaque black - the menu blacks the world out
			}
			else
			{
				VRMenuAnchorCaptured = false; // next menu-open recaptures against wherever the player then looks
				VRMenuLaserValid = false;
				// The gameplay HUD (health, ammo, crosshair, messages) is the same flat 2D layer the menu
				// is - console->PostRender paints all of it - so it gets the same treatment: rendered once
				// to the offscreen canvas here, then drawn as a world-space plane per eye by DrawVRHudPlane
				// (from inside DrawGameFrame), instead of stretched flat across each eye where it sits
				// outside the lenses' readable area. DrawGameFrame skips PreRender/PostRender while a VR eye
				// is current precisely because this call already ran them for the frame.
				DrawUICanvas(true); // transparent background so the tablet isn't a black slab over the world
			}

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
		// the window frozen on whatever it last showed. Deliberately does NOT touch VRMenuAnchorCaptured:
		// a dropped frame mid-menu must keep the frozen anchor so the plane doesn't jump when VR resumes.
		DrawGameFrame(vec4(flashScale, 1.0f), vec4(flashFog, 1.0f), true);
	}
}

void RenderSubsystem::DrawGameFrame(vec4 flashScale, vec4 flashFog, bool presentToDesktop)
{
	Device->Lock(flashScale, flashFog, vec4(0.0f), nullptr, nullptr);

	ResetCanvas();
	// In VR the flat 2D HUD does not go onto the eye - it goes onto the wrist tablet (DrawUICanvas ran
	// PreRender/PostRender to the offscreen canvas already this frame, DrawVRHudPlane draws that canvas as
	// a plane below). Running them again here would paint the HUD flat across the eye on top of it. On the
	// desktop (CurrentVREye null, including the VR-inactive fallback) nothing changes.
	if (!CurrentVREye)
		PreRender();

	if (engine->LaunchInfo.engineVersion <= 219 || engine->console->bNoDrawWorld() == false)
	{
		DrawScene();
		DrawVRHands();
		RenderOverlays();
		if (engine->LaunchInfo.IsDeusEx())
			PostRenderFlash();
		Device->EndFlash();
	}

	if (CurrentVREye)
		DrawVRHudPlane();
	else
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

void RenderSubsystem::DrawUICanvas(bool transparentBackground)
{
	DrawingVRMenuCanvas = true;
	DrawingVRHudCanvas = transparentBackground; // the HUD tablet is the transparent one; the menu is not
	Device->BeginUICanvasFrame();

	// The clear's alpha is what the HUD tablet keys off: cleared to 0, everywhere the HUD script doesn't
	// paint stays transparent and the premultiplied-over plane draw (DrawVRHudPlane) lets the world show
	// through there. The pause menu clears to opaque black instead - it wants to black the world out.
	vec4 screenClear = vec4(0.0f, 0.0f, 0.0f, transparentBackground ? 0.0f : 1.0f);
	Device->Lock(vec4(0.5f, 0.5f, 0.5f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), screenClear, nullptr, nullptr);
	ResetCanvas();
	PreRender();
	PostRender();
	Device->Unlock(false);

	Device->EndUICanvasFrame();
	DrawingVRMenuCanvas = false;
	DrawingVRHudCanvas = false;
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

	// Same room-scale correction BuildVREyeView() applies, and for the same reason: this has to agree
	// with where the eye actually ends up, or the plane lands off to the side of the player's gaze.
	vec3 localPos = RemoveRoomScaleOffset((eyeViews[0].Position + eyeViews[1].Position) * 0.5f);
	vec3 localForward = normalize(eyeViews[0].Forward + eyeViews[1].Forward);

	// Where the head actually is and what it is actually looking at, in world space. This has to agree
	// with where BuildVREyeView() puts the eye, or the plane lands off to the side of the player's gaze
	// by however far the head is from the play space anchor.
	vec3 worldGazeDir = normalize(localToWorldDir(localForward));
	vec3 worldEyePos = engine->CameraLocation + localToWorldDir(localPos);

	// Centre the plane on the gaze itself, pitch included - anchoring it at a fixed height in front of a
	// flattened gaze put the menu above or below where the player was actually looking whenever they
	// opened it while not looking level.
	const vec3 worldUpAxis(0.0f, 0.0f, 1.0f);
	vec3 worldRight = cross(worldUpAxis, worldGazeDir);
	if (length(worldRight) < 0.01f) // looking almost straight up/down - roll is meaningless, use the camera's
		worldRight = levelCameraRotation.YAxis;
	worldRight = normalize(worldRight);
	// Square-on to the gaze, but with roll taken out by construction (worldRight is always horizontal),
	// so the menu never comes out tilted just because the player's head was.
	vec3 worldPlaneUp = cross(worldGazeDir, worldRight);

	const float planeDistance = 1.5f * MetersToUnrealUnits;
	const float planeWidth = 2.2f * MetersToUnrealUnits;
	// Derived from the canvas rather than picked independently, so the menu can't end up stretched if
	// the canvas resolution is ever changed.
	const float planeHeight = planeWidth * (float)RenderDevice::VRMenuCanvasHeight / (float)RenderDevice::VRMenuCanvasWidth;

	vec3 center = worldEyePos + worldGazeDir * planeDistance;
	vec3 halfRight = worldRight * (planeWidth * 0.5f);
	vec3 halfUp = worldPlaneUp * (planeHeight * 0.5f);

	// Order matches VRMenuPlaneUVs (0,0)/(1,0)/(1,1)/(0,1): top-left, top-right, bottom-right, bottom-left.
	VRMenuPlaneCorners[0] = center - halfRight + halfUp;
	VRMenuPlaneCorners[1] = center + halfRight + halfUp;
	VRMenuPlaneCorners[2] = center + halfRight - halfUp;
	VRMenuPlaneCorners[3] = center - halfRight - halfUp;
}

void RenderSubsystem::UpdateVRMenuLaser()
{
	VRMenuLaserValid = false;

	VRSubsystem* vr = engine->vr.get();
	if (!vr || !vr->IsActive())
		return;

	const VRSubsystem::ControllerState& controller = vr->GetController(VRPlayerInput::MenuPointerHandIndex());
	if (!controller.PoseValid)
		return; // pointer controller off or untracked - leave the cursor where it was, draw no beam

	// Put the controller pose into the same world space the menu plane lives in. The plane was frozen
	// against a leveled (yaw-only) camera reference when it opened, and the eyes are rendered against that
	// same frozen reference while the menu is up (DrawVRMenuPlane), so the laser has to use it too or it
	// would point at where the plane isn't. This is the same local-vector -> world-vector mapping
	// CaptureVRMenuPlaneAnchor uses, against the frozen rotation it stored.
	Coords frozenRotation = Coords::Rotation(VRMenuFrozenCameraRotation);
	auto localToWorldDir = [&frozenRotation](const vec3& v)
	{
		return frozenRotation.XAxis * v.x + frozenRotation.YAxis * v.y + frozenRotation.ZAxis * v.z;
	};

	vec3 rayOrigin = VRMenuFrozenCameraLocation + localToWorldDir(RemoveRoomScaleOffset(controller.Position));
	vec3 rayDir = localToWorldDir(controller.Forward);
	float rayDirLen = length(rayDir);
	if (rayDirLen < 0.0001f)
		return;
	rayDir = rayDir / rayDirLen;

	// The plane as an origin + two edge vectors, in the corner/UV order CaptureVRMenuPlaneAnchor lays out:
	// corner[0] is UV (0,0) (top-left), [1] is (1,0), [3] is (0,1). So uEdge runs left->right as u: 0->1
	// and vEdge runs top->bottom as v: 0->1, which is also how WindowsMouseX/Y are measured.
	vec3 planeOrigin = VRMenuPlaneCorners[0];
	vec3 uEdge = VRMenuPlaneCorners[1] - planeOrigin;
	vec3 vEdge = VRMenuPlaneCorners[3] - planeOrigin;
	vec3 normal = cross(uEdge, vEdge);
	float normalLen = length(normal);
	if (normalLen < 0.0001f)
		return;
	normal = normal / normalLen;

	float denom = dot(rayDir, normal);
	if (std::abs(denom) < 0.0001f)
		return; // pointing along the plane - no meaningful hit

	float t = dot(planeOrigin - rayOrigin, normal) / denom;
	if (t <= 0.0f)
		return; // the plane is behind the hand

	vec3 hit = rayOrigin + rayDir * t;
	vec3 relative = hit - planeOrigin;
	float u = dot(relative, uEdge) / dot(uEdge, uEdge);
	float v = dot(relative, vEdge) / dot(vEdge, vEdge);

	// Draw the beam all the way to wherever the hand actually points, even past the menu's edge, so it
	// reads as a real laser; the cursor below is clamped onto the menu so edge buttons stay reachable.
	VRMenuLaserStart = rayOrigin;
	VRMenuLaserEnd = hit;
	VRMenuLaserValid = true;

	float cursorU = std::clamp(u, 0.0f, 1.0f);
	float cursorV = std::clamp(v, 0.0f, 1.0f);

	// The script (UWindow) menus - Unreal's UMenu, UT - read the absolute cursor from WindowsMouseX/Y when
	// bWindowsMouseAvailable, the same channel the desktop mouse drives them through. The values are in the
	// menu's *frame* coordinates - the SizeX/SizeY the menu is laid out against - which ResetCanvas sets to
	// the render surface's pixel size divided by its UI scale. On the desktop that scale is ~1, so frame
	// coords equal raw pixels; but the VR menu canvas has a UI scale of 2, so feeding raw canvas pixels
	// (u * canvasWidth) overshot the cursor by exactly that scale. Derive the VR canvas UI scale the same
	// way ResetCanvas does (not Canvas.uiscale, which is a frame stale here since UpdateVRMenuLaser runs
	// before DrawUICanvas) and divide it back out.
	int vertResolution = engine->LaunchInfo.engineVersion < 400 ? 768 : 960;
	int menuUiScale = std::max((RenderDevice::VRMenuCanvasHeight + vertResolution / 2) / vertResolution, 1);
	float frameX = (float)RenderDevice::VRMenuCanvasWidth / (float)menuUiScale;
	float frameY = (float)RenderDevice::VRMenuCanvasHeight / (float)menuUiScale;

	if (engine->viewport)
	{
		engine->viewport->WindowsMouseX() = cursorU * frameX;
		engine->viewport->WindowsMouseY() = cursorV * frameY;
		engine->viewport->bWindowsMouseAvailable() = true;
	}

	// Games whose menu is instead the native URootWindow (Deus Ex and the like) never read WindowsMouseX/Y;
	// they track their own cursor directly in the window's virtual space (GetVirtualWidth/Height), which is
	// what ScaleRect then multiplies by GetVirtualScale to reach canvas pixels. dxRootWindow is null for the
	// script menus above, so this is a no-op there and can't disturb the path that works. Untested (this
	// machine's game uses the script path) - revisit if a native-window game ever runs in VR.
	if (engine->dxRootWindow)
		engine->dxRootWindow->SetRootCursorPos(cursorU * UWindow::GetVirtualWidth(), cursorV * UWindow::GetVirtualHeight());
}

void RenderSubsystem::DrawVRMenuPlane()
{
	if (!CurrentVREye)
		return;

	mat4 worldToView, providedProjection;
	Coords headRotation;
	vec3 headPosition;
	BuildVREyeView(worldToView, providedProjection, headRotation, headPosition, VRMenuFrozenCameraLocation, VRMenuFrozenCameraRotation);

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

	// The pointer laser, in the same world space and eye frame as the plane, so it lands at the
	// stereo-correct spot the player is aiming at. Not depth-cued (flags 0), so the thin beam always reads
	// crisply on top of the plane instead of z-fighting the surface it terminates on. Where the beam meets
	// the plane is the cursor - the menu highlights whatever is under it, so no separate marker is needed.
	if (VRMenuLaserValid)
		Device->Draw3DLine(&frame, vec4(0.2f, 0.8f, 1.0f, 1.0f), 0, VRMenuLaserStart, VRMenuLaserEnd);
}

void RenderSubsystem::DrawVRHudPlane()
{
	if (!CurrentVREye || !engine->vrHands)
		return;

	// Anchored to a controller so it rides along with that wrist, the way you'd glance at a watch or a
	// strapped-on tablet. Which wrist is a launcher setting (VR.HudHand) - it wants the hand the weapon
	// isn't in (phase 4).
	const auto& vrSettings = LauncherSettings::Get().VR;
	const int hudHand = VRPlayerInput::HudHandIndex();
	const VRHands::HandPose& hand = engine->vrHands->GetHand(hudHand);
	if (!hand.Valid)
		return; // off-hand controller off or untracked - no tablet to hang it on this frame

	const float tabletWidth = vrSettings.HudTabletWidthCm * 0.01f * MetersToUnrealUnits;
	// Derived from the canvas so the panel can't stretch if the canvas resolution changes - same as the menu.
	const float tabletHeight = tabletWidth * (float)RenderDevice::VRMenuCanvasHeight / (float)RenderDevice::VRMenuCanvasWidth;

	// Fixed to the controller, not billboarded at the head: the player aims it by turning their wrist, the
	// way you tilt a watch to read it, so they can park it out of the way and glance at it. The panel's
	// basis comes straight from the hand pose, and it's rotated a quarter turn about the panel normal from
	// the obvious layout because in the headset the obvious one sat sideways: the panel's own right edge
	// runs along the hand's forward axis (down the forearm), its top edge along the hand's -right, and its
	// normal stays along +hand.Up (out the back of the wrist). This is the same hand pose the drawn hand
	// ball uses, so panel and hand stay glued together.
	//
	// The winding keeps the front face on +hand.Up: uEdge = +right = +hand.Forward, vEdge = -planeUp =
	// +hand.Right, and cross(forward, right) = +up. If it reads mirrored or upside down in the headset,
	// negate both `right` and `planeUp` (a 180 flip) or swap which one is negated (the other 90 turn).
	const float upOffset = vrSettings.HudTabletWristOffsetCm * 0.01f * MetersToUnrealUnits; // float it off the back of the wrist, not through it
	// Well up the forearm toward the player, not out past the hand - in the headset the panel wanted to sit
	// back over the wrist, not floating off the fingertips.
	const float backOffset = vrSettings.HudTabletForearmOffsetCm * 0.01f * MetersToUnrealUnits;

	vec3 right = hand.Forward;    // panel right edge runs down the forearm
	vec3 planeUp = -hand.Right;   // panel top edge; the quarter turn that fixed the sideways look

	vec3 center = hand.Position + hand.Up * upOffset - hand.Forward * backOffset;

	vec3 halfRight = right * (tabletWidth * 0.5f);
	vec3 halfUp = planeUp * (tabletHeight * 0.5f);

	// Order matches VRMenuPlaneUVs (0,0)/(1,0)/(1,1)/(0,1): top-left, top-right, bottom-right, bottom-left.
	vec3 corners[4];
	corners[0] = center - halfRight + halfUp;
	corners[1] = center + halfRight + halfUp;
	corners[2] = center + halfRight - halfUp;
	corners[3] = center - halfRight - halfUp;

	// The live camera, not the menu's frozen one: the tablet rides the hand through a moving world, so it
	// has to be placed against where the eye actually is this frame - exactly what DrawVRHands does for the
	// hand balls, and the two must agree since the tablet hangs off one of those hands.
	mat4 worldToView, providedProjection;
	Coords headRotation;
	vec3 headPosition;
	BuildVREyeView(worldToView, providedProjection, headRotation, headPosition, engine->CameraLocation, engine->CameraRotation);

	// A member rather than a local: SetSceneNode keeps the pointer and Unlock() derefs it again later this
	// frame, so the frame has to outlive this call (same reason VRHandsFrame is a member).
	FSceneNode& frame = VRHudFrame;
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

	// PF_Highlighted: premultiplied alpha-over, so the transparent-cleared canvas (DrawUICanvas(true))
	// composites its HUD pixels onto the world and leaves the rest see-through, instead of the opaque
	// black slab the menu draws.
	Device->DrawVRMenuPlane(&frame, corners, VRMenuPlaneUVs, PF_Highlighted);
}

void RenderSubsystem::DrawVRHands()
{
	if (!CurrentVREye || !engine->vrHands)
		return;

	bool anyHandVisible = false;
	for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
		anyHandVisible = anyHandVisible || engine->vrHands->GetHand(hand).Valid;
	if (!anyHandVisible)
		return;

	// The live camera, unlike DrawVRMenuPlane's frozen one - and the same camera VRHands::Tick placed the
	// hands against earlier this frame, so the ball is drawn exactly where the collider is.
	mat4 worldToView, providedProjection;
	Coords headRotation;
	vec3 headPosition;
	BuildVREyeView(worldToView, providedProjection, headRotation, headPosition, engine->CameraLocation, engine->CameraRotation);

	FSceneNode& frame = VRHandsFrame;
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

	// Draw3DLine reads the scene node from whatever was last set rather than from the frame it is passed,
	// so this has to happen before the lines and after DrawScene() has set its own.
	Device->SetSceneNode(&frame);

	// An outline ball rather than a solid one: Draw3DLine is the only primitive here that needs neither a
	// texture nor a light, so three great circles give an unlit hand that is see-through for free - the
	// player can see the button they are reaching for straight through their own hand. LINE_DepthCued so
	// the hand goes behind the wall it is pushed into instead of hovering on top of the world.
	const int segments = 16;
	const float handRadius = VRHands::HandRadius(); // the drawn ball is exactly the collider (see VRHands)
	for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
	{
		const VRHands::HandPose& pose = engine->vrHands->GetHand(hand);
		if (!pose.Valid)
			continue;

		// Left and right are told apart by colour, since the balls are otherwise identical and the player
		// has no other cue as to which hand the game thinks is where.
		vec4 color = (hand == VRSubsystem::HandLeft) ? vec4(0.35f, 0.75f, 1.0f, 1.0f) : vec4(1.0f, 0.65f, 0.25f, 1.0f);

		// Draw the great circles in the controller's own frame, not world axes. A sphere is rotationally
		// symmetric, so world-aligned rings look identical whichever way the controller is turned - the ball
		// never appeared to rotate at all. Rings in the local frame turn with the controller, and the
		// forward spoke below makes which way it points unmistakable.
		const vec3 basis[3] = { pose.Forward, pose.Right, pose.Up };
		for (int axis = 0; axis < 3; axis++)
		{
			vec3 u = basis[(axis + 1) % 3];
			vec3 v = basis[(axis + 2) % 3];

			vec3 previous = pose.Position + u * handRadius;
			for (int i = 1; i <= segments; i++)
			{
				float angle = radians(360.0f * i / segments);
				vec3 next = pose.Position + (u * std::cos(angle) + v * std::sin(angle)) * handRadius;
				Device->Draw3DLine(&frame, color, LINE_DepthCued, previous, next);
				previous = next;
			}
		}

		// A stub poking out the front of the ball, so the controller's facing reads at a glance instead of
		// having to be inferred from the ring pattern.
		Device->Draw3DLine(&frame, color, LINE_DepthCued, pose.Position, pose.Position + pose.Forward * (handRadius * 1.6f));
	}
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
