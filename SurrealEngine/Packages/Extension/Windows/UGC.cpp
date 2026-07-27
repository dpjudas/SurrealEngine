
#include "Precomp.h"
#include "UGC.h"
#include "Engine.h"
#include "Render/RenderSubsystem.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Packages/Engine/Resources/UFont.h"
#include "Packages/Engine/Resources/UPalette.h"

// Hardcoded |p colors
// Perhaps corresponding to CSS color names?
// TODO: Figure out the rest
static Color s_PColors[] = {
	Color{  0,   0,   0, 255}, // p0 = ???
	Color{255, 255, 255, 255}, // p1 = ??? (White maybe?)
	Color{255, 255, 255, 255}, // p2 = ???
	Color{255, 255, 255, 255}, // p3 = ???
	Color{255, 255,   0, 255}, // p4 = Yellow
	Color{  0,   0, 139, 255}, // p5 = Dark Blue
	Color{255, 255, 255, 255}, // p6 = ???
	Color{  0, 255, 255, 255}  // p7 = Cyan
	// Are there more colors???
};

void UGC::ClearZ()
{
	// Only used by ActorDisplayWindow.DrawWindow
	engine->render->Device->ClearZ();
}

void UGC::CopyGC(UObject* Copy)
{
	// Not used directly by scripts
	LogUnimplemented("GC.CopyGC");
}

void UGC::DrawActor(UObject* Actor, std::optional<bool> bClearZ, std::optional<bool> bConstrain, std::optional<bool> bUnlit, std::optional<float> DrawScale, std::optional<float> ScaleGlow, std::optional<UObject*> Skin)
{
	// Only used by ActorDisplayWindow.DrawWindow, AugmentationDisplayWindow.DrawWindow
	LogUnimplemented("GC.DrawActor");
}

void UGC::DrawText(float DestX, float DestY, float destWidth, float destHeight, const std::string& textStr)
{
	if (!bDrawEnabled())
		return;

	UFont* font = normalFont();
	if (font)
	{
		float x = offsetX + DestX;
		float y = offsetY + DestY;
		uint32_t polyflags = EffectiveTextPolyFlags();

		auto valign = (EVAlign)VAlign();
		if (valign == EVAlign::Top)
		{
			DrawText(font, x, y, destWidth, textStr, TextColor(), polyflags);
		}
		else if (valign == EVAlign::Center || valign == EVAlign::Full)
		{
			Sizef extents = DrawText(font, x, y, destWidth, textStr, TextColor(), polyflags, true);
			DrawText(font, x, y + (destHeight - extents.height) * 0.5f, destWidth, textStr, TextColor(), polyflags);
		}
		else if (valign == EVAlign::Bottom)
		{
			Sizef extents = DrawText(font, x, y, destWidth, textStr, TextColor(), polyflags, true);
			DrawText(font, x, y + destHeight - extents.height, destWidth, textStr, TextColor(), polyflags);
		}
	}
}

void UGC::DrawBorders(float DestX, float DestY, float destWidth, float destHeight, float leftMargin, float rightMargin, float TopMargin, float BottomMargin, UObject** borders, std::optional<bool> bStretchHorizontally, std::optional<bool> bStretchVertically)
{
	if (!bDrawEnabled())
		return;

	if (leftMargin != 0.0f || rightMargin != 0.0f || TopMargin != 0.0f || BottomMargin != 0.0f || bStretchHorizontally || bStretchVertically)
	{
		// margins are always zero from script. bStretchHorizontally and bStretchVertically are never specified.
		LogUnimplemented("GC.DrawBorders");
	}

	UTexture* tl = UObject::Cast<UTexture>(borders[0]);
	UTexture* tr = UObject::Cast<UTexture>(borders[1]);
	UTexture* bl = UObject::Cast<UTexture>(borders[2]);
	UTexture* left = UObject::Cast<UTexture>(borders[4]);
	UTexture* br = UObject::Cast<UTexture>(borders[3]);
	UTexture* right = UObject::Cast<UTexture>(borders[5]);
	UTexture* top = UObject::Cast<UTexture>(borders[6]);
	UTexture* bottom = UObject::Cast<UTexture>(borders[7]);
	UTexture* center = UObject::Cast<UTexture>(borders[8]);

	DestX += offsetX;
	DestY += offsetY;

	float tlX = 0.0f, trX = 0.0f;
	float tlY = 0.0f, trY = 0.0f;
	float blX = destWidth, brX = destWidth;
	float blY = destHeight, brY = destHeight;

	if (auto tex = center) // To do: maybe draw this only in the inner box?
	{
		float swidth = (float)tex->USize();
		float sheight = (float)tex->VSize();
		Rectf dest = Rectf::xywh(DestX, DestY, destWidth, destHeight);
		DrawTile(tex, ScaleRect(dest), Rectf::xywh(0.0f, 0.0f, swidth, sheight), tileColor(), EffectivePolyFlags());
	}
	if (auto tex = tl) // top left corner
	{
		float swidth = (float)tex->USize();
		float sheight = (float)tex->VSize();
		Rectf dest = Rectf::xywh(DestX, DestY, swidth, sheight);
		DrawTile(tex, ScaleRect(dest), Rectf::xywh(0.0f, 0.0f, swidth, sheight), tileColor(), EffectivePolyFlags());
		tlX = dest.right;
		tlY = dest.bottom;
	}
	if (auto tex = tr) // top right corner
	{
		float swidth = (float)tex->USize();
		float sheight = (float)tex->VSize();
		Rectf dest = Rectf::xywh(DestX + destWidth - swidth, DestY, swidth, sheight);
		DrawTile(tex, ScaleRect(dest), Rectf::xywh(0.0f, 0.0f, swidth, sheight), tileColor(), EffectivePolyFlags());
		trX = dest.left;
		trY = dest.bottom;
	}
	if (auto tex = bl) // bottom left corner
	{
		float swidth = (float)tex->USize();
		float sheight = (float)tex->VSize();
		Rectf dest = Rectf::xywh(DestX, DestY + destHeight - sheight, swidth, sheight);
		DrawTile(tex, ScaleRect(dest), Rectf::xywh(0.0f, 0.0f, swidth, sheight), tileColor(), EffectivePolyFlags());
		blX = dest.right;
		blY = dest.top;
	}
	if (auto tex = br) // bottom right corner
	{
		float swidth = (float)tex->USize();
		float sheight = (float)tex->VSize();
		Rectf dest = Rectf::xywh(DestX + destWidth - swidth, DestY + destHeight - sheight, swidth, sheight);
		DrawTile(tex, ScaleRect(dest), Rectf::xywh(0.0f, 0.0f, swidth, sheight), tileColor(), EffectivePolyFlags());
		brX = dest.left;
		brY = dest.top;
	}
	if (auto tex = left) // left side
	{
		float swidth = (float)tex->USize();
		float sheight = (float)tex->VSize();
		Rectf dest = Rectf::xywh(DestX, tlY, swidth, blY - tlY);
		DrawTile(tex, ScaleRect(dest), Rectf::xywh(0.0f, 0.0f, swidth, sheight), tileColor(), EffectivePolyFlags());
	}
	if (auto tex = right) // right side
	{
		float swidth = (float)tex->USize();
		float sheight = (float)tex->VSize();
		Rectf dest = Rectf::xywh(DestX + destWidth - swidth, trY, swidth, brY - trY);
		DrawTile(tex, ScaleRect(dest), Rectf::xywh(0.0f, 0.0f, swidth, sheight), tileColor(), EffectivePolyFlags());
	}
	if (auto tex = top) // top side
	{
		float swidth = (float)tex->USize();
		float sheight = (float)tex->VSize();
		Rectf dest = Rectf::xywh(tlX, DestY, trX - tlX, sheight);
		DrawTile(tex, ScaleRect(dest), Rectf::xywh(0.0f, 0.0f, swidth, sheight), tileColor(), EffectivePolyFlags());
	}
	if (auto tex = bottom) // bottom side
	{
		float swidth = (float)tex->USize();
		float sheight = (float)tex->VSize();
		Rectf dest = Rectf::xywh(tlX, DestY + destHeight - sheight, brX - blX, sheight);
		DrawTile(tex, ScaleRect(dest), Rectf::xywh(0.0f, 0.0f, swidth, sheight), tileColor(), EffectivePolyFlags());
	}
}

void UGC::DrawBox(float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, float boxThickness, UObject* tX)
{
	if (!bDrawEnabled())
		return;

	UTexture* tex = UObject::Cast<UTexture>(tX);
	if (tex)
	{
		float swidth = (float)tex->USize();
		float sheight = (float)tex->VSize();
		Rectf src = Rectf::xywh(0.0f, 0.0f, swidth, sheight);
		uint32_t polyflags = EffectivePolyFlags();
		Color color = tileColor();

		Rectf top = Rectf::xywh(offsetX + DestX, offsetY + DestY, destWidth, boxThickness);
		Rectf bottom = Rectf::xywh(offsetX + DestX, offsetY + DestY + destHeight - boxThickness, destWidth, boxThickness);
		Rectf left = Rectf::xywh(offsetX + DestX, offsetY + DestY, boxThickness, destHeight);
		Rectf right = Rectf::xywh(offsetX + DestX + destWidth - boxThickness, offsetY + DestY, boxThickness, destHeight);

		DrawTile(tex, ScaleRect(top), src, color, polyflags);
		DrawTile(tex, ScaleRect(bottom), src, color, polyflags);
		DrawTile(tex, ScaleRect(left), src, color, polyflags);
		DrawTile(tex, ScaleRect(right), src, color, polyflags);
	}
}

void UGC::DrawIcon(float DestX, float DestY, UObject* tX)
{
	if (!bDrawEnabled())
		return;

	UTexture* tex = UObject::Cast<UTexture>(tX);
	if (tex)
	{
		float swidth = (float)tex->USize();
		float sheight = (float)tex->VSize();
		Rectf dest = Rectf::xywh(offsetX + DestX, offsetY + DestY, swidth, sheight);
		Rectf src = Rectf::xywh(0.0f, 0.0f, swidth, sheight);
		DrawTile(tex, ScaleRect(dest), src, tileColor(), EffectivePolyFlags());
	}
}

void UGC::DrawPattern(float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, UObject* tX)
{
	if (!bDrawEnabled())
		return;

	UTexture* tex = UObject::Cast<UTexture>(tX);
	if (tex)
	{
		Rectf dest = Rectf::xywh(offsetX + DestX, offsetY + DestY, destWidth, destHeight);
		Rectf src = Rectf::xywh(OrgX, OrgY, destWidth, destHeight);
		DrawTile(tex, ScaleRect(dest), src, tileColor(), EffectivePolyFlags());
	}
}

void UGC::DrawStretchedTexture(float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, float srcWidth, float srcHeight, UObject* tX)
{
	if (!bDrawEnabled())
		return;

	UTexture* tex = UObject::Cast<UTexture>(tX);
	if (tex)
	{
		Rectf dest = Rectf::xywh(offsetX + DestX, offsetY + DestY, destWidth, destHeight);
		Rectf src = Rectf::xywh(srcX, srcY, srcWidth, srcHeight);
		DrawTile(tex, ScaleRect(dest), src, tileColor(), EffectivePolyFlags());
	}
}

void UGC::DrawTexture(float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, UObject* tX)
{
	if (!bDrawEnabled())
		return;

	UTexture* tex = UObject::Cast<UTexture>(tX);
	if (tex)
	{
		Rectf dest = Rectf::xywh(offsetX + DestX, offsetY + DestY, destWidth, destHeight);
		Rectf src = Rectf::xywh(srcX, srcY, destWidth, destHeight);
		DrawTile(tex, ScaleRect(dest), src, tileColor(), EffectivePolyFlags());
	}
}

Rectf UGC::ScaleRect(const Rectf& box)
{
	float scale = UWindow::GetVirtualScale();
	return Rectf(box.left * scale, box.top * scale, box.right * scale, box.bottom * scale);
}

uint32_t UGC::EffectivePolyFlags()
{
	uint32_t polyflags = PolyFlags();
	if (bMasked())
		polyflags |= PF_Masked;
	if (bModulated())
		polyflags |= PF_Modulated;
	if (!bSmoothed())
		polyflags |= PF_NoSmooth;
	if (bTranslucent())
		polyflags |= PF_Translucent;
	return polyflags;
}

uint32_t UGC::EffectiveTextPolyFlags()
{
	uint32_t polyflags = textPolyFlags() | PF_NoSmooth;
	if (bTextTranslucent())
		polyflags |= PF_Translucent;
	else
		polyflags |= PF_Masked;
	return polyflags;
}

void UGC::EnableDrawing(bool newDrawEnabled)
{
	// Only set by ActorDisplayWindow.DrawWindow and always set to true.
	bDrawEnabled() = newDrawEnabled;
}

void UGC::EnableMasking(bool bNewMasking)
{
	bMasked() = bNewMasking;
}

void UGC::EnableModulation(bool bNewModulation)
{
	bModulated() = bNewModulation;
}

void UGC::EnableSmoothing(bool bNewSmoothing)
{
	bSmoothed() = bNewSmoothing;
}

void UGC::EnableSpecialText(bool bNewSpecialText)
{
	SpecialTextEnabled = bNewSpecialText;
}

void UGC::EnableTranslucency(bool bNewTranslucency)
{
	bTranslucent() = bNewTranslucency;
}

void UGC::EnableTranslucentText(bool bNewTranslucency)
{
	bTextTranslucent() = bNewTranslucency;
}

void UGC::EnableWordWrap(bool bNewWordWrap)
{
	bWordWrap() = bNewWordWrap;
}

void UGC::GetAlignments(uint8_t& outHAlign, uint8_t& outVAlign)
{
	outHAlign = HAlign();
	outVAlign = VAlign();
}

float UGC::GetFontHeight(std::optional<bool> bIncludeSpace)
{
	// Not used directly by scripts
	if (!normalFont())
		return 0.0f;
	FontGlyph glyph = normalFont()->GetGlyph('X');
	return (float)glyph.VSize;
}

void UGC::GetFonts(UObject*& outNormalFont, UObject*& outBoldFont)
{
	outNormalFont = normalFont();
	outBoldFont = boldFont();
}

uint8_t UGC::GetHorizontalAlignment()
{
	return HAlign();
}

uint8_t UGC::GetStyle()
{
	return Style();
}

void UGC::GetTextColor(Color& outTextColor)
{
	outTextColor = TextColor();
}

void UGC::GetTextExtent(float destWidth, float& xExtent, float& yExtent, const std::string& textStr)
{
	UFont* font = normalFont();
	if (font)
	{
		Sizef extents = DrawText(font, 0.0f, 0.0f, destWidth, textStr, TextColor(), 0, true);
		xExtent = extents.width;
		yExtent = extents.height;
	}
	else
	{
		xExtent = 0.0f;
		yExtent = 0.0f;
	}
}

float UGC::GetTextVSpacing()
{
	return textVSpacing();
}

void UGC::GetTileColor(Color& outTileColor)
{
	outTileColor = tileColor();
}

uint8_t UGC::GetVerticalAlignment()
{
	return VAlign();
}

void UGC::Intersect(float ClipX, float ClipY, float clipWidth, float clipHeight)
{
	// Not called directly by script. Seems we only clip by window then?
	LogUnimplemented("GC.Intersect");
}

bool UGC::IsDrawingEnabled()
{
	return bDrawEnabled();
}

bool UGC::IsMaskingEnabled()
{
	return bMasked();
}

bool UGC::IsModulationEnabled()
{
	return bModulated();
}

bool UGC::IsSmoothingEnabled()
{
	return bSmoothed();
}

bool UGC::IsSpecialTextEnabled()
{
	return SpecialTextEnabled;
}

bool UGC::IsTranslucencyEnabled()
{
	return bTranslucent();
}

bool UGC::IsTranslucentTextEnabled()
{
	return bTextTranslucent();
}

bool UGC::IsWordWrapEnabled()
{
	return bWordWrap();
}

void UGC::PopGC(std::optional<int> gcNum)
{
	// Not used directly by scripts
	LogUnimplemented("GC.PopGC");
}

int UGC::PushGC()
{
	// Not used directly by scripts
	LogUnimplemented("GC.PushGC");
	return 0;
}

void UGC::SetAlignments(uint8_t newHAlign, uint8_t newVAlign)
{
	HAlign() = newHAlign;
	VAlign() = newVAlign;
}

void UGC::SetBaselineData(std::optional<float> newBaselineOffset, std::optional<float> newUnderlineHeight)
{
	if (newBaselineOffset)
		baselineOffset() = *newBaselineOffset;
	if (newUnderlineHeight)
		underlineHeight() = *newUnderlineHeight;
}

void UGC::SetBoldFont(UObject* newBoldFont)
{
	boldFont() = UObject::Cast<UFont>(newBoldFont);
}

void UGC::SetFont(UObject* NewFont)
{
	normalFont() = UObject::Cast<UFont>(NewFont);
	boldFont() = UObject::Cast<UFont>(NewFont);
}

void UGC::SetFonts(UObject* newNormalFont, UObject* newBoldFont)
{
	normalFont() = UObject::Cast<UFont>(newNormalFont);
	boldFont() = UObject::Cast<UFont>(newBoldFont);
}

void UGC::SetHorizontalAlignment(uint8_t newHAlign)
{
	HAlign() = newHAlign;
}

void UGC::SetNormalFont(UObject* newNormalFont)
{
	normalFont() = UObject::Cast<UFont>(newNormalFont);
}

void UGC::SetStyle(EDrawStyle NewStyle)
{
	Style() = (uint8_t)NewStyle;

	// Is this what it is doing? So stupid to have the same states 3 times!
	switch (NewStyle)
	{
	case EDrawStyle::None:
		bDrawEnabled() = false;
		PolyFlags() = 0;
		textPolyFlags() = 0;
		break;
	case EDrawStyle::Normal:
		PolyFlags() = 0;
		textPolyFlags() = 0;
		bDrawEnabled() = true;
		bMasked() = false;
		bTranslucent() = false;
		bModulated() = false;
		bTextTranslucent() = false;
		break;
	case EDrawStyle::Masked:
		PolyFlags() = 0;
		textPolyFlags() = 0;
		bDrawEnabled() = true;
		bMasked() = true;
		bTranslucent() = false;
		bModulated() = false;
		bTextTranslucent() = false;
		break;
	case EDrawStyle::Translucent:
		PolyFlags() = 0;
		textPolyFlags() = 0;
		bDrawEnabled() = true;
		bMasked() = false;
		bTranslucent() = true;
		bModulated() = false;
		bTextTranslucent() = true;
		break;
	case EDrawStyle::Modulated:
		PolyFlags() = 0;
		textPolyFlags() = PF_Modulated;
		bDrawEnabled() = true;
		bMasked() = false;
		bTranslucent() = false;
		bModulated() = true;
		bTextTranslucent() = false;
		break;
	}
}

void UGC::SetTextColor(const Color& newTextColor)
{
	TextColor() = newTextColor;
}

void UGC::SetTextVSpacing(float newVSpacing)
{
	textVSpacing() = newVSpacing;
}

void UGC::SetTileColor(const Color& newTileColor)
{
	tileColor() = newTileColor;
}

void UGC::SetVerticalAlignment(uint8_t newVAlign)
{
	VAlign() = newVAlign;
}

void UGC::DrawTile(UTexture* tex, const Rectf& dest, const Rectf& src, const Color& c, uint32_t flags)
{
	vec4 color(c.R / 255.0f, c.G / 255.0f, c.B / 255.0f, 1.0f/*c.A / 255.0f*/);
	float Z = 1.0f;
	vec4 fog(0.0f);

	FTextureInfo texinfo;
	texinfo.CacheID = (uint64_t)(ptrdiff_t)tex;
	texinfo.Texture = tex;
	texinfo.Format = texinfo.Texture->UsedFormat;
	texinfo.Mips = tex->UsedMipmaps.data();
	texinfo.NumMips = (int)tex->UsedMipmaps.size();
	texinfo.USize = tex->USize();
	texinfo.VSize = tex->VSize();
	if (tex->Palette())
		texinfo.Palette = (FColor*)tex->Palette()->Colors.data();

	if (dest.left >= dest.right || dest.top >= dest.bottom)
		return;

	if (dest.left >= clipBox.left && dest.top >= clipBox.top && dest.right <= clipBox.right && dest.bottom <= clipBox.bottom)
	{
		engine->render->DrawTile(
			texinfo,
			dest.left,
			dest.top,
			dest.right - dest.left,
			dest.bottom - dest.top,
			src.left,
			src.top,
			src.right - src.left,
			src.bottom - src.top,
			Z,
			color,
			fog,
			flags);
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
			engine->render->DrawTile(
				texinfo,
				d.left,
				d.top,
				d.right - d.left,
				d.bottom - d.top,
				s.left,
				s.top,
				s.right - s.left,
				s.bottom - s.top,
				Z, color, fog, flags);
	}
}

Sizef UGC::DrawText(UFont* font, float orgX, float orgY, float destWidth, const std::string& textStr, const Color& colormoo, uint32_t polyflags, bool noDraw)
{
	Color color = colormoo;
	color.A = 255; // grr

	if (!bWordWrap())
		destWidth = 100000.0f;


	// Remove the | and & escapes for now
	/*
	std::string text;
	text.reserve(textStr.size());
	for (char c : textStr)
	{
		if (c != '|' && c != '&')
			text += c;
	}
	*/

	auto halign = (EHAlign)HAlign();

	float totalWidth = 0.0f;
	float totalHeight = 0.0f;
	float curX = 0.0f;
	float curY = 0.0f;

	Array<TextBlock> textBlocks = FindTextBlocks(textStr, color);
	size_t lineBegin = 0;
	float lineWidth = 0.0f;
	float lineHeight = 0.0f;

	float emptyLineHeight = GetTextSize(font, " ").y;

	for (size_t pos = 0; pos < textBlocks.size(); pos++)
	{
		if (textBlocks[pos].text.front() == '\n')
		{
			if (pos != lineBegin)
			{
				float centerX = 0;
				if (halign == EHAlign::Center || halign == EHAlign::Full)
					centerX = std::round((destWidth - lineWidth) * 0.5f);
				else if (halign == EHAlign::Right)
					centerX = destWidth - lineWidth;

				if (!noDraw)
					DrawTextBlockRange(orgX + curX + centerX, orgY + curY, textBlocks, lineBegin, pos, font, polyflags);

				curY += lineHeight;
				totalHeight += lineHeight;
				totalWidth = std::max(totalWidth, lineWidth);
			}
			else
			{
				lineHeight = std::max(lineHeight, emptyLineHeight);
				curY += lineHeight;
				totalHeight += lineHeight;
			}

			curX = 0;
			lineBegin = pos + 1;
			lineWidth = 0.0f;
			lineHeight = 0.0f;
		}
		else
		{
			vec2 blockSize = GetTextSize(font, textBlocks[pos].text);
			if (lineWidth + blockSize.x > destWidth)
			{
				float centerX = 0;
				if (halign == EHAlign::Center || halign == EHAlign::Full)
					centerX = std::round((destWidth - lineWidth) * 0.5f);
				else if (halign == EHAlign::Right)
					centerX = destWidth - lineWidth;

				if (!noDraw)
					DrawTextBlockRange(orgX + curX + centerX, orgY + curY, textBlocks, lineBegin, pos, font, polyflags);

				curX = 0;
				curY += lineHeight;
				totalHeight += lineHeight;
				totalWidth = std::max(totalWidth, lineWidth);

				if (textBlocks[pos].text.front() == ' ')
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
		if (halign == EHAlign::Center || halign == EHAlign::Full)
			centerX = std::round((destWidth - lineWidth) * 0.5f);
		else if (halign == EHAlign::Right)
			centerX = destWidth - lineWidth;

		if (!noDraw)
			DrawTextBlockRange(orgX + curX + centerX, orgY + curY, textBlocks, lineBegin, textBlocks.size(), font, polyflags);

		curX += centerX + lineWidth;
		curY += lineHeight;
		totalHeight += lineHeight;
		totalWidth = std::max(totalWidth, lineWidth);
	}

	return Sizef(totalWidth + 1.0f, totalHeight); // Add 1.0f to avoid rounding issues
}

vec2 UGC::GetTextSize(UFont* font, const std::string& text)
{
	float x = 0.0f;
	float y = 0.0f;
	for (char c : text)
	{
		FontGlyph glyph = font->GetGlyph(c);
		x += (float)glyph.USize;
		y = std::max(y, (float)glyph.VSize);
	}
	return { x, y };
}

Array<TextBlock> UGC::FindTextBlocks(const std::string& text, const Color& color)
{
	// Split text into words, whitespace or newline
	Array<TextBlock> textBlocks;
	size_t pos = 0;

	// Find all |n tokens and convert them into '\n's
	std::string editedText = text;

	auto newLinePos = editedText.find("|n");

	while (newLinePos != std::string::npos)
	{
		editedText.replace(newLinePos, 2, "\n");
		newLinePos = editedText.find("|n");
	}

	while (pos < editedText.size())
	{
		if (editedText[pos] == '\r')
		{
			textBlocks.push_back({ "\r", Color{255,255,255, 255}, 0 });
			pos++;
		}
		if (editedText[pos] == '\n')
		{
			textBlocks.push_back({ "\n", Color{255,255,255, 255}, 0 });
			pos++;
		}
		else if (editedText[pos] == ' ')
		{
			// Arbitrary-length whitespace
			size_t end = std::min(editedText.find_first_not_of(' ', pos + 1), editedText.size());

			std::string whitespaceText = editedText.substr(pos, end - pos);

			textBlocks.push_back({ whitespaceText, color, 0 });
			pos = end;
		}
		else
		{
			size_t end = std::min(editedText.find_first_of(" \n", pos + 1), editedText.size());

			std::string foundText = editedText.substr(pos, end - pos);
			Color textColor = color;
			size_t accelPos = 0;

			if (foundText.starts_with("|p"))
			{
				try
				{
					int pColorIdx = std::stoi(foundText.substr(2, 1));
					textColor = s_PColors[pColorIdx];
					foundText = foundText.substr(3); // Strip |p and num out
				}
				catch (...)
				{
					foundText = foundText.substr(2); // Strip |p out
				}
			}

			auto accelFind = foundText.find("|&");

			if (accelFind != std::string::npos)
			{
				accelPos = accelFind;
				foundText = foundText.erase(accelFind, 2);
			}

			TextBlock block = {
				.text = foundText,
				.textColor = textColor,
				.accelPos = accelPos
			};

			textBlocks.push_back(block);
			pos = end;
		}
	}

	return textBlocks;
}

void UGC::DrawTextBlockRange(float x, float y, const Array<TextBlock>& textBlocks, size_t start, size_t end, UFont* font, uint32_t polyflags)
{
	for (size_t i = start; i < end; i++)
	{
		for (char c : textBlocks[i].text)
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

			Rectf dest = Rectf::xywh(x, y, (float)glyph.USize, (float)glyph.VSize);
			Rectf src = Rectf::xywh((float)glyph.StartU, (float)glyph.StartV, (float)glyph.USize, (float)glyph.VSize);

			DrawTile(glyph.Texture, ScaleRect(dest), src, textBlocks[i].textColor, polyflags);

			x += (float)glyph.USize;
		}
	}
}

void UGC::ResetClip(Rectf box)
{
	clipStack.clear();
	clipBox = box;
}

void UGC::PushClip(Rectf box)
{
	clipStack.push_back(clipBox);
	clipBox.left = std::max(clipBox.left, box.left);
	clipBox.top = std::max(clipBox.top, box.top);
	clipBox.right = std::min(clipBox.right, box.right);
	clipBox.bottom = std::min(clipBox.bottom, box.bottom);
	clipBox.right = std::max(clipBox.right, clipBox.left);
	clipBox.bottom = std::max(clipBox.bottom, clipBox.top);
}

void UGC::PopClip()
{
	clipBox = clipStack.back();
	clipStack.pop_back();
}
