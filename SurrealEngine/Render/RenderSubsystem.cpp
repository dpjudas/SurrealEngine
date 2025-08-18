
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
	FrameCounter++;
	LevelTimeElapsed = levelTimeElapsed;
	AutoUV += levelTimeElapsed * 64.0f;
	AmbientGlowTime = std::fmod(AmbientGlowTime + 0.8f * levelTimeElapsed, 1.0f);
	AmbientGlowAmount = 0.20f + 0.20f * std::sin(radians(AmbientGlowTime * 360.0f));

	Light.FogFrameCounter++;

	vec3 flashScale = 0.5f;
	vec3 flashFog = vec3(1.0f, 0.0f, 0.0f);

	UPlayerPawn* player = UObject::TryCast<UPlayerPawn>(engine->CameraActor);
	if (player)
	{
		flashScale = player->FlashScale();
		flashFog = player->FlashFog();
	}

	Device->Brightness = engine->client->Brightness;
	Device->Lock(vec4(flashScale, 1.0f), vec4(flashFog, 1.0f), vec4(0.0f), nullptr, nullptr);

	ResetCanvas();
	PreRender();

	if (engine->console->bNoDrawWorld() == false)
	{
		DrawScene();
		RenderOverlays();
		Device->EndFlash();
	}

	PostRender();

	Device->Unlock(true);
}

void RenderSubsystem::DrawEditorViewport()
{
	Device->Brightness = engine->client->Brightness;
	DrawScene();
}

void RenderSubsystem::UpdateTexture(UTexture* tex)
{
	if (tex && tex->FrameCounter != FrameCounter)
	{
		tex->Update(LevelTimeElapsed);
		tex->FrameCounter = FrameCounter;
	}
}

void RenderSubsystem::UpdateTextureInfo(FTextureInfo& info, BspSurface& surface, UTexture* texture, float ZoneUPanSpeed, float ZoneVPanSpeed)
{
	info.CacheID = (uint64_t)(ptrdiff_t)texture;
	info.bRealtimeChanged = texture->TextureModified;
	info.UScale = texture->DrawScale();
	info.VScale = texture->DrawScale();
	info.Pan.x = -(float)surface.PanU;
	info.Pan.y = -(float)surface.PanV;
	info.Texture = texture;
	info.Format = texture->ActualFormat;
	info.Mips = texture->Mipmaps.data();
	info.NumMips = (int)texture->Mipmaps.size();
	info.USize = texture->USize();
	info.VSize = texture->VSize();
	if (texture->Palette())
		info.Palette = (FColor*)texture->Palette()->Colors.data();

	if (texture->TextureModified)
		texture->TextureModified = false;

	if (surface.PolyFlags & PF_AutoUPan) info.Pan.x -= AutoUV * ZoneUPanSpeed;
	if (surface.PolyFlags & PF_AutoVPan) info.Pan.y -= AutoUV * ZoneVPanSpeed;
}

void RenderSubsystem::UpdateTextureInfo(FTextureInfo& info, const Poly& poly, UTexture* texture, float ZoneUPanSpeed, float ZoneVPanSpeed)
{
	info.CacheID = (uint64_t)(ptrdiff_t)texture;
	info.bRealtimeChanged = texture->TextureModified;
	info.UScale = texture->DrawScale();
	info.VScale = texture->DrawScale();
	info.Pan.x = -(float)poly.PanU;
	info.Pan.y = -(float)poly.PanV;
	info.Texture = texture;
	info.Format = texture->ActualFormat;
	info.Mips = texture->Mipmaps.data();
	info.NumMips = (int)texture->Mipmaps.size();
	info.USize = texture->USize();
	info.VSize = texture->VSize();
	if (texture->Palette())
		info.Palette = (FColor*)texture->Palette()->Colors.data();

	if (texture->TextureModified)
		texture->TextureModified = false;

	if (poly.PolyFlags & PF_AutoUPan) info.Pan.x -= AutoUV * ZoneUPanSpeed;
	if (poly.PolyFlags & PF_AutoVPan) info.Pan.y -= AutoUV * ZoneVPanSpeed;
}

void RenderSubsystem::OnMapLoaded()
{
	Device->Flush(true);

	Light.Lights.clear();
	Light.lmtextures.clear();
	Light.fogtextures.clear();

	std::set<UActor*> lightset;
	for (UActor* light : engine->Level->Model->Lights)
		lightset.insert(light);
	for (UActor* light : lightset)
		Light.Lights.push_back(light);
}
