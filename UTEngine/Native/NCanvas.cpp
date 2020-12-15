
#include "Precomp.h"
#include "NCanvas.h"
#include "VM/NativeFunc.h"

void NCanvas::RegisterFunctions()
{
	RegisterVMNativeFunc_3("Canvas", "DrawActor", &NCanvas::DrawActor, 467);
	RegisterVMNativeFunc_7("Canvas", "DrawClippedActor", &NCanvas::DrawClippedActor, 471);
	RegisterVMNativeFunc_9("Canvas", "DrawPortal", &NCanvas::DrawPortal, 480);
	RegisterVMNativeFunc_2("Canvas", "DrawText", &NCanvas::DrawText, 465);
	RegisterVMNativeFunc_2("Canvas", "DrawTextClipped", &NCanvas::DrawTextClipped, 469);
	RegisterVMNativeFunc_7("Canvas", "DrawTile", &NCanvas::DrawTile, 466);
	RegisterVMNativeFunc_7("Canvas", "DrawTileClipped", &NCanvas::DrawTileClipped, 468);
	RegisterVMNativeFunc_3("Canvas", "StrLen", &NCanvas::StrLen, 464);
	RegisterVMNativeFunc_3("Canvas", "TextSize", &NCanvas::TextSize, 470);
}

void NCanvas::DrawActor(UObject* Self, UObject* A, bool WireFrame, bool* ClearZ)
{
	throw std::runtime_error("Canvas.DrawActor not implemented");
}

void NCanvas::DrawClippedActor(UObject* Self, UObject* A, bool WireFrame, int X, int Y, int XB, int YB, bool* ClearZ)
{
	throw std::runtime_error("Canvas.DrawClippedActor not implemented");
}

void NCanvas::DrawPortal(UObject* Self, int X, int Y, int Width, int Height, UObject* CamActor, const vec3& CamLocation, const Rotator& CamRotation, int* FOV, bool* ClearZ)
{
	throw std::runtime_error("Canvas.DrawPortal not implemented");
}

void NCanvas::DrawText(UObject* Self, const std::string& Text, bool* CR)
{
	throw std::runtime_error("Canvas.DrawText not implemented");
}

void NCanvas::DrawTextClipped(UObject* Self, const std::string& Text, bool* bCheckHotKey)
{
	throw std::runtime_error("Canvas.DrawTextClipped not implemented");
}

void NCanvas::DrawTile(UObject* Self, UObject* Tex, float XL, float YL, float U, float V, float UL, float VL)
{
	throw std::runtime_error("Canvas.DrawTile not implemented");
}

void NCanvas::DrawTileClipped(UObject* Self, UObject* Tex, float XL, float YL, float U, float V, float UL, float VL)
{
	throw std::runtime_error("Canvas.DrawTileClipped not implemented");
}

void NCanvas::StrLen(UObject* Self, const std::string& String, float& XL, float& YL)
{
	throw std::runtime_error("Canvas.StrLen not implemented");
}

void NCanvas::TextSize(UObject* Self, const std::string& String, float& XL, float& YL)
{
	throw std::runtime_error("Canvas.TextSize not implemented");
}
