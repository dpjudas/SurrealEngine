
#include "Precomp.h"
#include "RenderSubsystem.h"
#include "RenderDevice/RenderDevice.h"
#include "UObject/USubsystem.h"
#include "GameWindow.h"
#include "VM/ScriptCall.h"
#include "Engine.h"

void RenderSubsystem::ResetCanvas()
{
	Canvas.uiscale = std::max((engine->ViewportHeight + 540) / 1080, 1);

	FSceneNode frame;
	Canvas.Frame.XB = 0;
	Canvas.Frame.YB = 0;
	Canvas.Frame.X = engine->ViewportWidth;
	Canvas.Frame.Y = engine->ViewportHeight;
	Canvas.Frame.FX = (float)engine->ViewportWidth;
	Canvas.Frame.FY = (float)engine->ViewportHeight;
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

	int sizeX = (int)(engine->ViewportWidth / (float)Canvas.uiscale);
	int sizeY = (int)(engine->ViewportHeight / (float)Canvas.uiscale);
	engine->canvas->CurX() = 0.0f;
	engine->canvas->CurY() = 0.0f;
	engine->console->FrameX() = (float)sizeX;
	engine->console->FrameY() = (float)sizeY;
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
	CallEvent(engine->viewport->Actor(), EventName::PreRender, { ExpressionValue::ObjectValue(engine->canvas) });
}

void RenderSubsystem::RenderOverlays()
{
	Device->SetSceneNode(&Canvas.Frame);
	CallEvent(engine->viewport->Actor(), EventName::RenderOverlays, { ExpressionValue::ObjectValue(engine->canvas) });
}

void RenderSubsystem::PostRender()
{
	Device->SetSceneNode(&Canvas.Frame);
	CallEvent(engine->viewport->Actor(), EventName::PostRender, { ExpressionValue::ObjectValue(engine->canvas) });
	CallEvent(engine->console, EventName::PostRender, { ExpressionValue::ObjectValue(engine->canvas) });
	DrawTimedemoStats();
	
	if (ShowCollisionDebug)
		DrawCollisionDebug();
}

void RenderSubsystem::DrawActor(UActor* actor, bool WireFrame, bool ClearZ)
{
	actor->bHidden() = false;

	Device->SetSceneNode(&Scene.Frame);
	if (ClearZ)
		Device->ClearZ(&Scene.Frame);
	DrawMesh(&Scene.Frame, actor, WireFrame);
	Device->SetSceneNode(&Canvas.Frame);

	actor->bHidden() = true;
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
	frame.ObjectToWorld = Coords::ViewToRenderDev().ToMatrix() * mat4::scale(1.0f, 1.0f, -1.0f);
	frame.WorldToView = mat4::identity();
	frame.FovAngle = engine->CameraFovAngle;
	float Aspect = frame.FY / frame.FX;
	float RProjZ = (float)std::tan(radians(frame.FovAngle) * 0.5f);
	float RFX2 = 2.0f * RProjZ / frame.FX;
	float RFY2 = 2.0f * RProjZ * Aspect / frame.FY;
	frame.Projection = mat4::frustum(-RProjZ, RProjZ, -Aspect * RProjZ, Aspect * RProjZ, 1.0f, 32768.0f, handedness::left, clipzrange::zero_positive_w);
	Device->SetSceneNode(&frame);

	if (ClearZ)
		Device->ClearZ(&frame);

	actor->bHidden() = false;
	DrawMesh(&frame, actor, WireFrame);
	actor->bHidden() = true;

	Device->SetSceneNode(&Canvas.Frame);
}

void RenderSubsystem::DrawTile(UTexture* Tex, float x, float y, float XL, float YL, float U, float V, float UL, float VL, float Z, vec4 color, vec4 fog, uint32_t flags)
{
	UpdateTexture(Tex);
	Tex = Tex->GetAnimTexture();
	UpdateTexture(Tex);

	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)Tex;
	texinfo.Texture = Tex;
	texinfo.Format = texinfo.Texture->ActualFormat;
	texinfo.Mips = Tex->Mipmaps.data();
	texinfo.NumMips = (int)Tex->Mipmaps.size();
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
	UpdateTexture(Tex);
	Tex = Tex->GetAnimTexture();
	UpdateTexture(Tex);

	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)Tex;
	texinfo.Texture = Tex;
	texinfo.Format = texinfo.Texture->ActualFormat;
	texinfo.Mips = Tex->Mipmaps.data();
	texinfo.NumMips = (int)Tex->Mipmaps.size();
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

void RenderSubsystem::DrawText(UFont* font, vec4 color, float orgX, float orgY, float& curX, float& curY, float& curYL, bool newlineAtEnd, const std::string& text, uint32_t flags, bool center, float spaceX, float spaceY)
{
	float centerX = 0;
	if (center)
		centerX = std::round((engine->canvas->SizeX() - GetTextSize(font, text).x) * 0.5f);

	for (char c : text)
	{
		// To do: word wrap
		// To do: SpaceX and SpaceY also affects DrawText

		if (c == '\n')
		{
			curX = 0;
			curY += curYL;
			curYL = 0;
		}
		else
		{
			FontGlyph glyph = font->GetGlyph(c);

			FTextureInfo texinfo;
			texinfo.CacheID = (uint64_t)(ptrdiff_t)glyph.Texture;
			texinfo.Texture = glyph.Texture;
			texinfo.Format = texinfo.Texture->ActualFormat;
			texinfo.Mips = glyph.Texture->Mipmaps.data();
			texinfo.NumMips = (int)glyph.Texture->Mipmaps.size();
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

			Device->DrawTile(&Canvas.Frame, texinfo, (orgX + curX + centerX) * Canvas.uiscale, (float)(orgY + curY) * Canvas.uiscale, (float)width * Canvas.uiscale, (float)height * Canvas.uiscale, StartU, StartV, USize, VSize, 1.0f, color, vec4(0.0f), PF_Highlighted | PF_NoSmooth | PF_Masked);

			curX += width + spaceX;
			curYL = std::max(curYL, (float)glyph.VSize + spaceY);
		}
	}

	curY += curYL;

	if (newlineAtEnd)
	{
		curX = 0;
		curY += curYL;
		curYL = 0;
	}
}

void RenderSubsystem::DrawTextClipped(UFont* font, vec4 color, float orgX, float orgY, float curX, float curY, const std::string& text, uint32_t flags, bool checkHotKey, float clipX, float clipY, bool center)
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
			texinfo.Format = texinfo.Texture->ActualFormat;
			texinfo.Mips = glyph.Texture->Mipmaps.data();
			texinfo.NumMips = (int)glyph.Texture->Mipmaps.size();
			texinfo.USize = glyph.Texture->USize();
			texinfo.VSize = glyph.Texture->VSize();
			if (glyph.Texture->Palette())
				texinfo.Palette = (FColor*)glyph.Texture->Palette()->Colors.data();

			Rectf dest = Rectf::xywh(orgX + curX + centerX, orgY + curY, (float)glyph.USize, (float)glyph.VSize);
			Rectf src = Rectf::xywh((float)glyph.StartU, (float)glyph.StartV, (float)glyph.USize, (float)glyph.VSize);
			DrawTile(texinfo, dest, src, clipBox, 1.0f, color, vec4(0.0f), PF_Highlighted | PF_NoSmooth | PF_Masked);

			texinfo.CacheID = (uint64_t)(ptrdiff_t)uglyph.Texture;
			texinfo.Texture = uglyph.Texture;

			dest = Rectf::xywh(orgX + curX + (glyph.USize - uwidth) / 2, orgY + curY, (float)uwidth, (float)uheight);
			src = Rectf::xywh(uStartU, uStartV, uUSize, uVSize);
			DrawTile(texinfo, dest, src, clipBox, 1.0f, color, vec4(0.0f), PF_Highlighted | PF_NoSmooth | PF_Masked);

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
			texinfo.Format = texinfo.Texture->ActualFormat;
			texinfo.Mips = glyph.Texture->Mipmaps.data();
			texinfo.NumMips = (int)glyph.Texture->Mipmaps.size();
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

ivec2 RenderSubsystem::GetTextSize(UFont* font, const std::string& text)
{
	int x = 0;
	int y = 0;
	for (char c : text)
	{
		FontGlyph glyph = font->GetGlyph(c);
		x += glyph.USize;
		y = std::max(y, glyph.VSize);
	}
	return { x, y };
}

ivec2 RenderSubsystem::GetTextClippedSize(UFont* font, const std::string& text, float clipX)
{
	int x = 0;
	int y = 0;
	for (char c : text)
	{
		FontGlyph glyph = font->GetGlyph(c);
		if (x + glyph.USize > (int)clipX)
			break;
		x += glyph.USize;
		y = std::max(y, glyph.VSize);
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

		/*size_t numCollisionActors = 0;
		for (auto& it : engine->Level->Hash.CollisionActors)
			numCollisionActors += it.second.size();
		lines.push_back(std::to_string(numCollisionActors) + " collision actors");*/

		lines.push_back(std::to_string(Scene.OpaqueNodes.size() + Scene.TranslucentNodes.size()) + " visible surfaces");
		lines.push_back(std::to_string(Scene.Actors.size()) + " visible actors");
		lines.push_back(std::to_string(Scene.Coronas.size()) + " visible coronas");

		UFont* font = engine->canvas->SmallFont();
		if (font)
		{
			float curY = 180;
			for (const std::string& text : lines)
			{
				float curX = engine->ViewportWidth / (float)Canvas.uiscale - GetTextSize(font, text).x - 16;
				float curYL = 0.0f;
				DrawText(font, vec4(1.0f), 0.0f, 0.0f, curX, curY, curYL, false, text, PF_NoSmooth | PF_Masked, false);
				curY += curYL;
			}

			/*
			Array<std::string> leftlines;
			engine->audiodev->AddStats(leftlines);
			curY = 64;
			for (const std::string& text : leftlines)
			{
				float curX = 16.0f;
				float curYL = 0.0f;
				DrawText(font, vec4(1.0f), 0.0f, 0.0f, curX, curY, curYL, false, text, PF_NoSmooth | PF_Masked, false);
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

		/*size_t numCollisionActors = 0;
		for (auto& it : engine->Level->Hash.CollisionActors)
			numCollisionActors += it.second.size();
		lines.push_back(std::to_string(numCollisionActors) + " collision actors");*/

		lines.push_back(std::to_string(Scene.OpaqueNodes.size() + Scene.TranslucentNodes.size()) + " visible surfaces");
		lines.push_back(std::to_string(Scene.Actors.size()) + " visible actors");
		lines.push_back(std::to_string(Scene.Coronas.size()) + " visible coronas");

		lines.push_back(std::to_string(Scene.Clipper.numDrawSpans) + " spans");
		lines.push_back(std::to_string(Scene.Clipper.numSurfs) + " checked surfaces");
		lines.push_back(std::to_string(Scene.Clipper.numTris) + " checked triangles");

		UFont* font = engine->canvas->MedFont();
		if (font)
		{
			float curY = 180;
			for (const std::string& text : lines)
			{
				float curX = engine->ViewportWidth / (float)Canvas.uiscale - GetTextSize(font, text).x - 16;
				float curYL = 0.0f;
				DrawText(font, vec4(1.0f), 0.0f, 0.0f, curX, curY, curYL, false, text, PF_NoSmooth | PF_Masked, false);
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
			float curX = engine->ViewportWidth / (float)Canvas.uiscale - GetTextSize(font, text).x - 16;
			float curYL = 0.0f;
			DrawText(font, vec4(1.0f), 0.0f, 0.0f, curX, curY, curYL, false, text, PF_NoSmooth | PF_Masked, false);
			curY += curYL;
		}
	}
}