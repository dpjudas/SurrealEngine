
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "VisibleMesh.h"
#include "RenderDevice/RenderDevice.h"
#include "UObject/USubsystem.h"
#include "UObject/UActor.h"
#include "GameWindow.h"
#include "VM/ScriptCall.h"
#include "Engine.h"
#include "LauncherSettings.h"
#include "VR/VRPlayerInput.h"
#include "Math/coords.h"

namespace
{
	// The player's own first-person weapon - the one the game draws through Canvas.DrawActor each frame and
	// positions relative to the camera. Only this actor gets moved onto the VR hand; any other actor a script
	// happens to draw through the same native is left exactly where it placed it.
	bool IsPlayerViewWeapon(UActor* actor)
	{
		if (!actor || !engine->viewport)
			return false;
		UPlayerPawn* pawn = UObject::TryCast<UPlayerPawn>(engine->viewport->Actor());
		return pawn && pawn->Weapon() == actor;
	}
}

void RenderSubsystem::ResetCanvas()
{
	// While rendering a VR eye, the canvas has to be sized to the eye's render target (the headset's
	// recommended eye resolution), not the desktop window - the two can have very different sizes/aspects.
	int width, height;
	if (DrawingVRMenuCanvas)
	{
		// The offscreen canvas the VR menu plane gets rendered to (see DrawUICanvas()), not an eye or
		// the desktop window.
		width = RenderDevice::VRMenuCanvasWidth;
		height = RenderDevice::VRMenuCanvasHeight;
	}
	else if (CurrentVREye && engine->vr)
	{
		width = engine->vr->GetRecommendedEyeWidth();
		height = engine->vr->GetRecommendedEyeHeight();
	}
	else
	{
		width = engine->viewport->ViewportWidth();
		height = engine->viewport->ViewportHeight();
	}

	// Scale the UI so it matches what you saw on a 1024x768 CRT monitor for Unreal and other older games.
	// Assume 1280x960 for UT and newer.
	int vertResolution = engine->LaunchInfo.engineVersion < 400 ? 768 : 960;
	Canvas.uiscale = std::max((height + vertResolution / 2) / vertResolution, 1);

	// The HUD tablet lays out in a smaller virtual space than the derived scale would give, which makes
	// each element take up a larger share of the (fixed physical size) tablet - the derived scale is tuned
	// to keep desktop UI a constant apparent size across resolutions, but on a wrist panel that leaves the
	// elements too small to read. A single knob: +1 shrinks the virtual space by one step (bigger
	// elements), at the cost of a more cramped layout. Only the HUD, not the pause menu.
	const int VRHudUiScaleBoost = 3;
	if (DrawingVRHudCanvas)
		Canvas.uiscale += VRHudUiScaleBoost;

	FSceneNode frame;
	Canvas.Frame.XB = 0;
	Canvas.Frame.YB = 0;
	Canvas.Frame.X = width;
	Canvas.Frame.Y = height;
	Canvas.Frame.FX = (float)width;
	Canvas.Frame.FY = (float)height;
	Canvas.Frame.FX2 = Canvas.Frame.FX * 0.5f;
	Canvas.Frame.FY2 = Canvas.Frame.FY * 0.5f;
	Canvas.Frame.ObjectToWorld = mat4::identity();
	Canvas.Frame.WorldToView = mat4::identity();
	Canvas.Frame.FovAngle = engine->CameraFovAngle;
	float Aspect = Canvas.Frame.FY / Canvas.Frame.FX;
	float RProjZ = (float)std::tan(radians(Canvas.Frame.FovAngle) * 0.5f);
	float RFX2 = 2.0f * RProjZ / Canvas.Frame.FX;
	float RFY2 = 2.0f * RProjZ * Aspect / Canvas.Frame.FY;
	Canvas.Frame.Projection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);

	int sizeX = (int)(width / (float)Canvas.uiscale);
	int sizeY = (int)(height / (float)Canvas.uiscale);
	engine->canvas->CurX() = 0.0f;
	engine->canvas->CurY() = 0.0f;
	if (engine->LaunchInfo.engineVersion > 219)
	{
		engine->console->FrameX() = (float)sizeX;
		engine->console->FrameY() = (float)sizeY;
	}
	engine->canvas->ClipX() = (float)sizeX;
	engine->canvas->ClipY() = (float)sizeY;
	engine->canvas->SizeX() = sizeX;
	engine->canvas->SizeY() = sizeY;
	//engine->viewport->bShowWindowsMouse() = true; // bShowWindowsMouse is set to true by WindowConsole if mouse cursor should be visible
	//engine->viewport->bWindowsMouseAvailable() = true; // if true then RenderUWindow updates mouse pos from (WindowsMouseX,WindowsMouseY), otherwise it uses KeyEvent(IK_MouseX, delta) + KeyEvent(IK_MouseY, delta). Maybe used for windowed mode?
	//engine->viewport->WindowsMouseX() = 10.0f;
	//engine->viewport->WindowsMouseY() = 200.0f;
	CallEvent(engine->canvas, EventName::Reset);
}

void RenderSubsystem::PreRender()
{
	Device->SetSceneNode(&Canvas.Frame);
	CallEvent(engine->console, EventName::PreRender, { ExpressionValue::ObjectValue(engine->canvas) });
	if (engine->viewport->Actor())
		CallEvent(engine->viewport->Actor(), EventName::PreRender, { ExpressionValue::ObjectValue(engine->canvas) });
}

void RenderSubsystem::RenderOverlays()
{
	Device->SetSceneNode(&Canvas.Frame);
	if (engine->viewport->Actor())
	{
		if (engine->LaunchInfo.engineVersion > 219)
		{
			CallEvent(engine->viewport->Actor(), EventName::RenderOverlays, { ExpressionValue::ObjectValue(engine->canvas) });
		}
		else
		{
			UWeapon* weapon = engine->viewport->Actor()->Weapon();
			if (weapon)
			{
				CallEvent(weapon, "InvCalcView", {});
				DrawActor(weapon, false, false);
			}
		}
	}
}

void RenderSubsystem::PostRender()
{
	Device->SetSceneNode(&Canvas.Frame);
	if (engine->viewport->Actor())
		CallEvent(engine->viewport->Actor(), EventName::PostRender, { ExpressionValue::ObjectValue(engine->canvas) });
	CallEvent(engine->console, EventName::PostRender, { ExpressionValue::ObjectValue(engine->canvas) });
	DrawTimedemoStats();
	
	if (ShowCollisionDebug)
		DrawCollisionDebug();
}

void RenderSubsystem::PostRenderFlash()
{
	Device->SetSceneNode(&Canvas.Frame);
	if (engine->viewport->Actor())
		CallEvent(engine->viewport->Actor(), "PostRenderFlash", {ExpressionValue::ObjectValue(engine->canvas)});
}

void RenderSubsystem::DrawActor(UActor* actor, bool WireFrame, bool ClearZ)
{
	Device->SetSceneNode(&MainFrame.Frame);
	if (ClearZ)
		Device->ClearZ();

	// Phase 4: hang the first-person weapon off the weapon hand. The game's CalcDrawOffset has already
	// placed it relative to the camera (welding it to the head); while a VR eye is being drawn, override
	// that with the hand's pose so the gun is where the hand is. The mesh is 3D-projected into the eye
	// either way (RenderScene gives MainFrame the eye projection), so only Location/Rotation/DrawScale
	// change here. Restored after the draw so nothing downstream sees the overridden transform, and so the
	// game's script keeps owning it on the desktop and on the next frame.
	bool weaponOverridden = false;
	vec3 savedLocation;
	Rotator savedRotation;
	float savedDrawScale = 0.0f;
	if (CurrentVREye && engine->vr && engine->vr->IsActive() && engine->vrHands && IsPlayerViewWeapon(actor))
	{
		const VRHands::HandPose& hand = engine->vrHands->GetHand(VRPlayerInput::WeaponHandIndex());
		if (hand.Valid)
		{
			const auto& vr = LauncherSettings::Get().VR;
			const float degToRot = 65536.0f / 360.0f;
			const float cmToUU = 0.01f * MetersToUnrealUnits;

			// Align the gun to the AIM pose, not the grip pose. The grip pose runs along the controller's
			// handle (tilted well off the direction the player is pointing), so a gun laid on it points where
			// the physical controller lies rather than where the hand model's pointer stub aims - the two
			// disagreed by the grip's rake angle. The aim pose is the same ray the drawn hand ball's forward
			// stub uses (RenderSubsystem::DrawVRHands) and the same one the menu laser casts along, so the
			// muzzle now lines up with the pointer line the player sees.
			const vec3 handForward = hand.Forward;
			const vec3 handRight = hand.Right;
			const vec3 handUp = hand.Up;

			// The tuning rotation is applied in the aim pose's own frame - pitch tilts the muzzle, yaw swings
			// it, roll banks it - regardless of how the wrist is turned, by expressing the offset's axes in
			// that basis (the same local-to-world mapping VRHands uses for the camera basis).
			Rotator off(
				(int)std::lround(vr.WeaponPitchOffsetDegrees * degToRot),
				(int)std::lround(vr.WeaponYawOffsetDegrees * degToRot),
				(int)std::lround(vr.WeaponRollOffsetDegrees * degToRot));
			Coords offCoords = Coords::Rotation(off);
			auto handLocalToWorld = [&](const vec3& v)
			{
				return handForward * v.x + handRight * v.y + handUp * v.z;
			};
			vec3 forward = handLocalToWorld(offCoords.XAxis);
			vec3 up = handLocalToWorld(offCoords.ZAxis);

			vec3 position = hand.Position
				+ handForward * (vr.WeaponForwardOffsetCm * cmToUU)
				+ handRight * (vr.WeaponRightOffsetCm * cmToUU)
				+ handUp * (vr.WeaponUpOffsetCm * cmToUU);

			savedLocation = actor->Location();
			savedRotation = actor->Rotation();
			savedDrawScale = actor->DrawScale();
			actor->Location() = position;
			actor->Rotation() = RotatorFromForwardUp(forward, up);
			// First-person view meshes are modelled tiny - they read at the right size on the desktop only
			// because CalcDrawOffset parks them centimetres from the camera. Held at arm's length in VR they
			// shrink to a toy, so scale the mesh up (WeaponScalePercent, a single in-headset tuning knob) to
			// bring it back to a plausible held size.
			actor->DrawScale() = savedDrawScale * (vr.WeaponScalePercent * 0.01f);
			weaponOverridden = true;
		}
	}

	actor->bHidden() = false;
	VisibleMesh vismesh;
	if (vismesh.DrawMesh(&MainFrame, actor, WireFrame, false))
		vismesh.DrawMesh(&MainFrame, actor, WireFrame, true);
	actor->bHidden() = true;

	if (weaponOverridden)
	{
		actor->Location() = savedLocation;
		actor->Rotation() = savedRotation;
		actor->DrawScale() = savedDrawScale;
	}

	Device->SetSceneNode(&Canvas.Frame);
}

void RenderSubsystem::DrawClippedActor(UActor* actor, bool WireFrame, int X, int Y, int XB, int YB, bool ClearZ)
{
	FSceneNode frame;
	frame.XB = XB * Canvas.uiscale;
	frame.YB = YB * Canvas.uiscale;
	frame.X = X * Canvas.uiscale;
	frame.Y = Y * Canvas.uiscale;
	frame.FX = (float)X * Canvas.uiscale;
	frame.FY = (float)Y * Canvas.uiscale;
	frame.FX2 = frame.FX * 0.5f;
	frame.FY2 = frame.FY * 0.5f;
	frame.ObjectToWorld = Coords::ViewToRenderDev().ToMatrix();
	frame.WorldToView = mat4::identity();
	frame.FovAngle = engine->CameraFovAngle;
	float Aspect = frame.FY / frame.FX;
	float RProjZ = (float)std::tan(radians(frame.FovAngle) * 0.5f);
	float RFX2 = 2.0f * RProjZ / frame.FX;
	float RFY2 = 2.0f * RProjZ * Aspect / frame.FY;
	frame.Projection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);
	Device->SetSceneNode(&frame);

	if (ClearZ)
		Device->ClearZ();

	actor->bHidden() = false;
	VisibleMesh vismesh;
	if (vismesh.DrawMesh(&MainFrame, actor, WireFrame, false))
		vismesh.DrawMesh(&MainFrame, actor, WireFrame, true);
	actor->bHidden() = true;

	Device->SetSceneNode(&Canvas.Frame);
}

void RenderSubsystem::DrawTile(UTexture* Tex, float x, float y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags)
{
	if (!Tex)
		return;
	UpdateTexture(Tex);
	Tex = Tex->GetAnimTexture();
	UpdateTexture(Tex);

	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)Tex;
	texinfo.Texture = Tex;
	texinfo.Format = texinfo.Texture->UsedFormat;
	texinfo.Mips = Tex->UsedMipmaps.data();
	texinfo.NumMips = (int)Tex->UsedMipmaps.size();
	texinfo.USize = Tex->USize();
	texinfo.VSize = Tex->VSize();
	if (Tex->Palette())
		texinfo.Palette = (FColor*)Tex->Palette()->Colors.data();

	if (Tex->bMasked())
		flags |= PF_Masked;

	Device->DrawTile(&Canvas.Frame, texinfo, x * Canvas.uiscale, y * Canvas.uiscale, XL * Canvas.uiscale, YL * Canvas.uiscale, U, V, UL, VL, Z, color, fog, flags);
}

void RenderSubsystem::DrawTileClipped(UTexture* Tex, float orgX, float orgY, float curX, float curY, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags, float clipX, float clipY)
{
	if (!Tex)
		return;
	UpdateTexture(Tex);
	Tex = Tex->GetAnimTexture();
	UpdateTexture(Tex);

	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)Tex;
	texinfo.Texture = Tex;
	texinfo.Format = texinfo.Texture->UsedFormat;
	texinfo.Mips = Tex->UsedMipmaps.data();
	texinfo.NumMips = (int)Tex->UsedMipmaps.size();
	texinfo.USize = Tex->USize();
	texinfo.VSize = Tex->VSize();
	if (Tex->Palette())
		texinfo.Palette = (FColor*)Tex->Palette()->Colors.data();

	if (Tex->bMasked())
		flags |= PF_Masked;

	Rectf clipBox = Rectf::xywh(orgX, orgY, clipX, clipY);
	Rectf dest = Rectf::xywh(orgX + curX, orgY + curY, XL, YL);
	Rectf src = Rectf::xywh(U, V, UL, VL);
	DrawTile(texinfo, dest, src, clipBox, Z, color, fog, flags);
}

Array<std::string> RenderSubsystem::FindTextBlocks(const std::string& text)
{
	// Split text into words, whitespace or newline
	Array<std::string> textBlocks;
	size_t pos = 0;
	while (pos < text.size())
	{
		if (text[pos] == '\n')
		{
			textBlocks.push_back("\n");
			pos++;
		}
		else if (text[pos] == ' ')
		{
			size_t end = std::min(text.find_first_not_of(' ', pos + 1), text.size());
			textBlocks.push_back(text.substr(pos, end - pos));
			pos = end;
		}
		else
		{
			size_t end = std::min(text.find_first_of(" \n", pos + 1), text.size());
			textBlocks.push_back(text.substr(pos, end - pos));
			pos = end;
		}
	}
	return textBlocks;
}

void RenderSubsystem::DrawTextBlockRange(float x, float y, const Array<std::string>& textBlocks, size_t start, size_t end, UFont* font, vec4 color, uint32_t polyflags, float spaceX)
{
	for (size_t i = start; i < end; i++)
	{
		for (char c : textBlocks[i])
		{
			FontGlyph glyph = font->GetGlyph(c);

			if (!glyph.Texture)
				continue;

			FTextureInfo texinfo;
			texinfo.CacheID = (uint64_t)(ptrdiff_t)glyph.Texture;
			texinfo.Texture = glyph.Texture;
			texinfo.Format = texinfo.Texture->UsedFormat;
			texinfo.Mips = glyph.Texture->UsedMipmaps.data();
			texinfo.NumMips = (int)glyph.Texture->UsedMipmaps.size();
			texinfo.USize = glyph.Texture->USize();
			texinfo.VSize = glyph.Texture->VSize();
			if (glyph.Texture->Palette())
				texinfo.Palette = (FColor*)glyph.Texture->Palette()->Colors.data();

			int width = glyph.USize;
			int height = glyph.VSize;
			float StartU = (float)glyph.StartU;
			float StartV = (float)glyph.StartV;
			float USize = (float)glyph.USize;
			float VSize = (float)glyph.VSize;

			Device->DrawTile(&Canvas.Frame, texinfo, x * Canvas.uiscale, y * Canvas.uiscale, (float)width * Canvas.uiscale, (float)height * Canvas.uiscale, StartU, StartV, USize, VSize, 1.0f, color, vec4(0.0f), polyflags);

			x += width + spaceX;
		}
	}
}

void RenderSubsystem::DrawText(UFont* font, vec4 color, float orgX, float orgY, float& curX, float& curY, float& curXL, float& curYL, bool newlineAtEnd, const std::string& text, uint32_t polyflags, bool center, float spaceX, float spaceY, float clipX, float clipY, bool noDraw)
{
	float totalWidth = 0.0f;
	float totalHeight = 0.0f;

	Array<std::string> textBlocks = FindTextBlocks(text);
	size_t lineBegin = 0;
	float lineWidth = 0.0f;
	float lineHeight = 0.0f;
	for (size_t pos = 0; pos < textBlocks.size(); pos++)
	{
		if (textBlocks[pos].front() == '\n')
		{
			if (pos != lineBegin)
			{
				float centerX = 0;
				if (center)
					centerX = std::round((clipX - lineWidth) * 0.5f);
				if (!noDraw)
					DrawTextBlockRange(orgX + curX + centerX, orgY + curY, textBlocks, lineBegin, pos, font, color, polyflags, spaceX);
				curY += lineHeight;
				totalHeight += lineHeight;
				totalWidth = std::max(totalWidth, lineWidth);
			}

			curX = 0;
			lineBegin = pos + 1;
			lineWidth = 0.0f;
			lineHeight = 0.0f;
		}
		else
		{
			vec2 blockSize = GetTextSize(font, textBlocks[pos], spaceX, spaceY);
			if (lineWidth + blockSize.x > clipX)
			{
				float centerX = 0;
				if (center)
					centerX = std::round((clipX - lineWidth) * 0.5f);
				if (!noDraw)
					DrawTextBlockRange(orgX + curX + centerX, orgY + curY, textBlocks, lineBegin, pos, font, color, polyflags, spaceX);

				curX = 0;
				curY += lineHeight;
				totalHeight += lineHeight;
				totalWidth = std::max(totalWidth, lineWidth);

				if (textBlocks[pos].front() == ' ')
				{
					// Ignore whitespace at the beginning of a word wrapped line
					lineBegin = pos + 1;
					lineWidth = 0.0f;
					lineHeight = 0.0f;
				}
				else
				{
					lineBegin = pos;
					lineWidth = blockSize.x;
					lineHeight = blockSize.y;
				}
			}
			else
			{
				lineWidth += blockSize.x;
				lineHeight = std::max(lineHeight, blockSize.y);
			}
		}
	}

	if (lineBegin < textBlocks.size())
	{
		float centerX = 0;
		if (center)
			centerX = std::round((clipX - lineWidth) * 0.5f);
		if (!noDraw)
			DrawTextBlockRange(orgX + curX + centerX, orgY + curY, textBlocks, lineBegin, textBlocks.size(), font, color, polyflags, spaceX);
		curX += centerX + lineWidth;
		curY += lineHeight;
		totalHeight += lineHeight;
		totalWidth = std::max(totalWidth, lineWidth);
	}

	curXL = std::max(curXL, totalWidth);
	curYL = std::max(curYL, totalHeight);

	if (newlineAtEnd)
	{
		curX = 0;
		curY += curYL;
		curXL = 0;
		curYL = 0;
	}
}

void RenderSubsystem::DrawTextClipped(UFont* font, vec4 color, float orgX, float orgY, float curX, float curY, const std::string& text, uint32_t polyflags, bool checkHotKey, float clipX, float clipY, bool center)
{
	FontGlyph uglyph = font->GetGlyph('_');
	int uwidth = uglyph.USize;
	int uheight = uglyph.VSize;
	float uStartU = (float)uglyph.StartU;
	float uStartV = (float)uglyph.StartV;
	float uUSize = (float)uglyph.USize;
	float uVSize = (float)uglyph.VSize;

	Rectf clipBox = Rectf::xywh(orgX, orgY, clipX, clipY);

	float centerX = 0;
	if (center)
		centerX = std::round((clipX - GetTextSize(font, text).x) * 0.5f);

	bool foundAmpersand = false;
	int maxY = 0;
	for (char c : text)
	{
		if (checkHotKey && c == '&' && !foundAmpersand)
		{
			foundAmpersand = true;
		}
		else if (foundAmpersand && c != '&')
		{
			foundAmpersand = false;

			FontGlyph glyph = font->GetGlyph(c);
			if (curX + glyph.USize > (int)clipX)
				break;

			FTextureInfo texinfo;
			texinfo.CacheID = (uint64_t)(ptrdiff_t)glyph.Texture;
			texinfo.Texture = glyph.Texture;
			texinfo.Format = texinfo.Texture->UsedFormat;
			texinfo.Mips = glyph.Texture->UsedMipmaps.data();
			texinfo.NumMips = (int)glyph.Texture->UsedMipmaps.size();
			texinfo.USize = glyph.Texture->USize();
			texinfo.VSize = glyph.Texture->VSize();
			if (glyph.Texture->Palette())
				texinfo.Palette = (FColor*)glyph.Texture->Palette()->Colors.data();

			Rectf dest = Rectf::xywh(orgX + curX + centerX, orgY + curY, (float)glyph.USize, (float)glyph.VSize);
			Rectf src = Rectf::xywh((float)glyph.StartU, (float)glyph.StartV, (float)glyph.USize, (float)glyph.VSize);
			DrawTile(texinfo, dest, src, clipBox, 1.0f, color, vec4(0.0f), polyflags);

			texinfo.CacheID = (uint64_t)(ptrdiff_t)uglyph.Texture;
			texinfo.Texture = uglyph.Texture;

			dest = Rectf::xywh(orgX + curX + (glyph.USize - uwidth) / 2, orgY + curY, (float)uwidth, (float)uheight);
			src = Rectf::xywh(uStartU, uStartV, uUSize, uVSize);
			DrawTile(texinfo, dest, src, clipBox, 1.0f, color, vec4(0.0f), polyflags);

			curX += glyph.USize;
			maxY = std::max(maxY, glyph.VSize);
		}
		else
		{
			foundAmpersand = false;

			FontGlyph glyph = font->GetGlyph(c);
			if (curX + glyph.USize > (int)clipX)
				break;

			FTextureInfo texinfo;
			texinfo.CacheID = (uint64_t)(ptrdiff_t)glyph.Texture;
			texinfo.Texture = glyph.Texture;
			texinfo.Format = texinfo.Texture->UsedFormat;
			texinfo.Mips = glyph.Texture->UsedMipmaps.data();
			texinfo.NumMips = (int)glyph.Texture->UsedMipmaps.size();
			texinfo.USize = glyph.Texture->USize();
			texinfo.VSize = glyph.Texture->VSize();
			if (glyph.Texture->Palette())
				texinfo.Palette = (FColor*)glyph.Texture->Palette()->Colors.data();

			Rectf dest = Rectf::xywh(orgX + curX + centerX, orgY + curY, (float)glyph.USize, (float)glyph.VSize);
			Rectf src = Rectf::xywh((float)glyph.StartU, (float)glyph.StartV, (float)glyph.USize, (float)glyph.VSize);
			DrawTile(texinfo, dest, src, clipBox, 1.0f, color, vec4(0.0f), PF_Highlighted | PF_NoSmooth | PF_Masked);

			curX += glyph.USize;
			maxY = std::max(maxY, glyph.VSize);
		}
	}
}

void RenderSubsystem::DrawTile(FTextureInfo& texinfo, const Rectf& dest, const Rectf& src, const Rectf& clipBox, float Z, vec4 color, vec4 fog, uint32_t flags)
{
	if (dest.left > dest.right || dest.top > dest.bottom)
		return;

	if (dest.left >= clipBox.left && dest.top >= clipBox.top && dest.right <= clipBox.right && dest.bottom <= clipBox.bottom)
	{
		Device->DrawTile(&Canvas.Frame, texinfo, dest.left * Canvas.uiscale, dest.top * Canvas.uiscale, (dest.right - dest.left) * Canvas.uiscale, (dest.bottom - dest.top) * Canvas.uiscale, src.left, src.top, src.right - src.left, src.bottom - src.top, Z, color, fog, flags);
	}
	else
	{
		Rectf d = dest;
		Rectf s = src;

		float scaleX = (s.right - s.left) / (d.right - d.left);
		float scaleY = (s.bottom - s.top) / (d.bottom - d.top);

		if (d.left < clipBox.left)
		{
			s.left += scaleX * (clipBox.left - d.left);
			d.left = clipBox.left;
		}
		if (d.right > clipBox.right)
		{
			s.right += scaleX * (clipBox.right - d.right);
			d.right = clipBox.right;
		}
		if (d.top < clipBox.top)
		{
			s.top += scaleY * (clipBox.top - d.top);
			d.top = clipBox.top;
		}
		if (d.bottom > clipBox.bottom)
		{
			s.bottom += scaleY * (clipBox.bottom - d.bottom);
			d.bottom = clipBox.bottom;
		}

		if (d.left < d.right && d.top < d.bottom)
			Device->DrawTile(&Canvas.Frame, texinfo, d.left * Canvas.uiscale, d.top * Canvas.uiscale, (d.right - d.left) * Canvas.uiscale, (d.bottom - d.top) * Canvas.uiscale, s.left, s.top, s.right - s.left, s.bottom - s.top, Z, color, fog, flags);
	}
}

void RenderSubsystem::Draw2DLine(vec4 Color, uint32_t LineFlags, vec3 P1, vec3 P2)
{
	auto uiscale = static_cast<float>(Canvas.uiscale);
	Device->Draw2DLine(&Canvas.Frame, Color, LineFlags, vec3(P1.xy() * uiscale, P1.z), vec3(P2.xy() * uiscale, P2.z));
}

void RenderSubsystem::Draw3DLine(vec4 Color, uint32_t LineFlags, vec3 P1, vec3 P2)
{
	Device->Draw3DLine(&Canvas.Frame, Color, LineFlags, P1, P2);
}

void RenderSubsystem::DrawTile(FTextureInfo& Info, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 Color, vec4 Fog, uint32_t PolyFlags)
{
	Device->DrawTile(&Canvas.Frame, Info, X, Y, XL, YL, U, V, UL, VL, Z, Color, Fog, PolyFlags);
}

vec2 RenderSubsystem::GetTextSize(UFont* font, const std::string& text, float spaceX, float spaceY)
{
	float x = 0.0f;
	float y = 0.0f;
	for (char c : text)
	{
		FontGlyph glyph = font->GetGlyph(c);
		x += (float)glyph.USize + spaceX;
		y = std::max(y, (float)glyph.VSize + spaceY);
	}
	return { x, y };
}

void RenderSubsystem::DrawTimedemoStats()
{
	Canvas.framesDrawn++;
	if (Canvas.startFPSTime == 0 || engine->lastTime - Canvas.startFPSTime >= 1'000'000)
	{
		Canvas.fps = Canvas.framesDrawn;
		Canvas.startFPSTime = engine->lastTime;
		Canvas.framesDrawn = 0;
	}

	if (ShowTimedemoStats)
	{
		Array<std::string> lines;
		lines.push_back(std::to_string(Canvas.fps) + " FPS");
		lines.push_back(std::to_string(engine->Level->Actors.size()) + " actors");
		lines.push_back(std::to_string(GC::GetStats().numObjects) + " GC objects");
		lines.push_back(std::to_string(GC::GetStats().memoryUsage / (1024 * 1024)) + " mb memory used");
		lines.push_back(std::to_string(Stats.Frames) + " visible frames");
		lines.push_back(std::to_string(Stats.Surfaces) + " visible surfaces");
		lines.push_back(std::to_string(Stats.Actors) + " visible actors");

		UFont* font = engine->canvas->SmallFont();
		if (font)
		{
			float curY = 180;
			for (const std::string& text : lines)
			{
				float curX = engine->viewport->ViewportWidth() / (float)Canvas.uiscale - GetTextSize(font, text).x - 16;
				float curXL = 0.0f;
				float curYL = 0.0f;
				DrawText(font, vec4(1.0f), 0.0f, 0.0f, curX, curY, curXL, curYL, false, text, PF_NoSmooth | PF_Masked, false);
				curY += curYL;
			}

			/*
			Array<std::string> leftlines;
			engine->audiodev->AddStats(leftlines);
			curY = 64;
			for (const std::string& text : leftlines)
			{
				float curX = 16.0f;
				float curXL = 0.0f;
				float curYL = 0.0f;
				DrawText(font, vec4(1.0f), 0.0f, 0.0f, curX, curY, curXL, curYL, false, text, PF_NoSmooth | PF_Masked, false);
				curY += curYL;
			}
			*/
		}
	}

	if (ShowRenderStats)
	{
		Array<std::string> lines;
		lines.push_back(std::to_string(Canvas.fps) + " FPS");
		lines.push_back(std::to_string(engine->Level->Actors.size()) + " actors");
		lines.push_back(std::to_string(GC::GetStats().numObjects) + " GC objects");
		lines.push_back(std::to_string(GC::GetStats().memoryUsage / (1024 * 1024)) + " mb memory used");

		/*size_t numCollisionActors = 0;
		for (auto& it : engine->Level->Hash.CollisionActors)
			numCollisionActors += it.second.size();
		lines.push_back(std::to_string(numCollisionActors) + " collision actors");*/

		/*lines.push_back(std::to_string(Scene.OpaqueNodes.size() + Scene.TranslucentNodes.size()) + " visible surfaces");
		lines.push_back(std::to_string(Scene.Actors.size()) + " visible actors");
		lines.push_back(std::to_string(Scene.Coronas.size()) + " visible coronas");

		lines.push_back(std::to_string(Scene.Clipper.numDrawSpans) + " spans");
		lines.push_back(std::to_string(Scene.Clipper.numSurfs) + " checked surfaces");
		lines.push_back(std::to_string(Scene.Clipper.numTris) + " checked triangles");*/

		UFont* font = engine->canvas->MedFont();
		if (font)
		{
			float curY = 180;
			for (const std::string& text : lines)
			{
				float curX = engine->viewport->ViewportWidth() / (float)Canvas.uiscale - GetTextSize(font, text).x - 16;
				float curXL = 0.0f;
				float curYL = 0.0f;
				DrawText(font, vec4(1.0f), 0.0f, 0.0f, curX, curY, curXL, curYL, false, text, PF_NoSmooth | PF_Masked, false);
				curY += curYL;
			}
		}
	}
}

void RenderSubsystem::DrawCollisionDebug()
{
	Array<std::string> lines;
	if (engine->PlayerBspNode)
	{
		BspNode* node = engine->PlayerBspNode;
		vec3& normal = engine->PlayerHitNormal;
		vec3& location = engine->PlayerHitLocation;
		BspSurface* surf = (node->Surf >= 0) ? &engine->Level->Model->Surfaces[node->Surf] : nullptr;

		lines.push_back("BspNode CollisionBound: " + std::to_string(node->CollisionBound));
		lines.push_back("BspNode Surface: " + std::to_string(node->Surf));

		if (surf && surf->Material)
			lines.push_back("BspNode Texture: " + surf->Material->Name.ToString());

		lines.push_back("BspNode Plane: (" +
			std::to_string(node->PlaneX) + ", " +
			std::to_string(node->PlaneY) + ", " +
			std::to_string(node->PlaneZ) + ", " +
			std::to_string(node->PlaneW) + ")"
		);

		BBox box = node->GetCollisionBox(engine->Level->Model);
		lines.push_back("BspNode Bound Min: (" +
			std::to_string(box.min.x) + ", " +
			std::to_string(box.min.y) + ", " +
			std::to_string(box.min.z) + ")"
		);

		lines.push_back("BspNode Bound Max: (" +
			std::to_string(box.max.x) + ", " +
			std::to_string(box.max.y) + ", " +
			std::to_string(box.max.z) + ")"
		);

		lines.push_back("HitNormal: (" +
			std::to_string(normal.x) + ", " +
			std::to_string(normal.y) + ", " +
			std::to_string(normal.z) + ")"
		);

		lines.push_back("HitLocation: (" +
			std::to_string(location.x) + ", " +
			std::to_string(location.y) + ", " +
			std::to_string(location.z) + ")"
		);
	}

	UFont* font = engine->canvas->MedFont();
	if (font)
	{
		float curY = 180;
		for (const std::string& text : lines)
		{
			float curX = engine->viewport->ViewportWidth() / (float)Canvas.uiscale - GetTextSize(font, text).x - 16;
			float curXL = 0.0f;
			float curYL = 0.0f;
			DrawText(font, vec4(1.0f), 0.0f, 0.0f, curX, curY, curXL, curYL, false, text, PF_NoSmooth | PF_Masked, false);
			curY += curYL;
		}
	}
}