
#include "Precomp.h"
#include "Editor2DViewport.h"
#include "Engine.h"
#include "UObject/ULevel.h"
#include <zwidget/core/colorf.h>

#include "Package/PackageManager.h"
#include "Package/IniFile.h"
#include "Package/IniProperty.h"

Editor2DViewport::Editor2DViewport(const Coords& coords, Widget* parent) : EditorViewport(parent), ViewCoords(coords)
{
	ReadLineColors();
}

Editor2DViewport::~Editor2DViewport()
{
}

void Editor2DViewport::OnPaint(Canvas* canvas)
{
	Colorf background = Colorf::fromRgba8(40, 40, 40);
	canvas->fillRect(Rect::xywh(0.0, 0.0, GetWidth(), GetHeight()), background);
	DrawGrid(canvas);
	if (engine && engine->Level)
	{
		DrawLevel(canvas);
	}
}

void Editor2DViewport::DrawLevel(Canvas* canvas)
{
	for (UActor* actor : engine->Level->Actors)
	{
		UBrush* brush = UActor::TryCast<UBrush>(actor);
		if (brush)
		{
			UModel* model = brush->Brush();
			if (model)
			{
				mat4 objectToWorld = mat4::translate(actor->Location()) * Coords::Rotation(actor->Rotation()).ToMatrix() * mat4::translate(-actor->PrePivot()) * mat4::scale(actor->DrawScale());

				for (const Poly& poly : model->Polys->Polys)
				{
					Point center(GetWidth() * 0.5, GetHeight() * 0.5);

					int numverts = (int)poly.Vertices.size();

					vec3 n = vec3(0.0f, 0.0f, -1.0f);
					if (numverts > 2)
					{
						const vec3 p0 = ViewCoords * (objectToWorld * vec4(poly.Vertices[0], 1.0f)).xyz();
						const vec3 p1 = ViewCoords * (objectToWorld * vec4(poly.Vertices[1], 1.0f)).xyz();
						const vec3 p2 = ViewCoords * (objectToWorld * vec4(poly.Vertices[2], 1.0f)).xyz();
						n = cross(p0 - p1, p2 - p1);
					}

					// Colorf linecolor = n.z > 0.0f ? Colorf::fromRgba8(150, 150, 150) : Colorf::fromRgba8(100, 100, 100);
					auto linecolor = GetActorLineColor(actor);

					for (int j = 0; j < numverts; j++)
					{
						int k = j + 1;
						if (k == numverts)
							k = 0;

						const vec3 p0 = ViewCoords * (objectToWorld * vec4(poly.Vertices[j], 1.0f)).xyz();
						const vec3 p1 = ViewCoords * (objectToWorld * vec4(poly.Vertices[k], 1.0f)).xyz();

						canvas->line(Point(p0.x - Location.x, p0.y - Location.y) * Zoom + center, Point(p1.x - Location.x, p1.y - Location.y) * Zoom + center, linecolor);
					}
				}
			}
		}
	}
}

void Editor2DViewport::DrawGrid(Canvas* canvas)
{
	Colorf linecolor = Colorf::fromRgba8(75, 75, 75);
	Point center(GetWidth() * 0.5, GetHeight() * 0.5);

	for (double y = -32767.0; y < 32767.0; y += 1000.0)
	{
		canvas->line(Point(-32767.0 - Location.x, y - Location.y) * Zoom + center, Point(32767.0 - Location.x, y - Location.y) * Zoom + center, linecolor);
	}
	for (double x = -32767.0; x < 32767.0; x += 1000.0)
	{
		canvas->line(Point(x - Location.x, -32767.0 - Location.y) * Zoom + center, Point(x - Location.x, 32767.0 - Location.y) * Zoom + center, linecolor);
	}
}

bool Editor2DViewport::OnMouseWheel(const Point& pos, InputKey key)
{
	if (key == InputKey::MouseWheelUp)
	{
		Zoom = std::min(Zoom + 0.01, 2.0);
		Update();
	}
	else if (key == InputKey::MouseWheelDown)
	{
		Zoom = std::max(Zoom - 0.01, 0.01);
		Update();
	}
	return true;
}

void Editor2DViewport::OnMouseMove(const Point& pos)
{
}

bool Editor2DViewport::OnMouseDown(const Point& pos, InputKey key)
{
	SetFocus();
	if (key == InputKey::RightMouse)
	{
		LockCursor();
		MouseIsMoving = true;
	}
	return true;
}

bool Editor2DViewport::OnMouseDoubleclick(const Point& pos, InputKey key)
{
	return true;
}

bool Editor2DViewport::OnMouseUp(const Point& pos, InputKey key)
{
	if (key == InputKey::RightMouse && MouseIsMoving)
	{
		UnlockCursor();
		MouseIsMoving = false;
	}
	return true;
}

void Editor2DViewport::OnKeyDown(InputKey key)
{
	if (key == InputKey::A)
	{
		MoveCamera(-100.0f, 0.0f);
	}
	else if (key == InputKey::D)
	{
		MoveCamera(100.0f, 0.0f);
	}
	else if (key == InputKey::W)
	{
		MoveCamera(0.0f, -100.0f);
	}
	else if (key == InputKey::S)
	{
		MoveCamera(0.0f, 100.0f);
	}
}

void Editor2DViewport::OnKeyUp(InputKey key)
{
}

void Editor2DViewport::OnRawMouseMove(int dx, int dy)
{
	if (MouseIsMoving)
	{
		MoveCamera(-dx * 20.0f, -dy * 20.0f);
	}
}

void Editor2DViewport::MoveCamera(float x, float y)
{
	Location.x += x;
	Location.y += y;
	Update();
}

Colorf Editor2DViewport::GetActorLineColor(UActor* actor) const
{
	auto drawType = actor->DrawType();

	if (drawType == DT_Brush)
	{
		// If the brush is a mover, then it should be colored as such
		if (auto mover = UObject::TryCast<UMover>(actor); mover)
			return m_MoverColor;

		// Otherwise, it will get the color of the CSG operation it has
		auto brush = UObject::TryCast<UBrush>(actor);

		if (brush)
		{
			auto csgOper = brush->CsgOper();

			switch (csgOper)
			{
				case CSG_Active:
					return m_BrushWireColor;
				case CSG_Add:
					return m_BrushAddColor;
				case CSG_Sub:
					return m_BrushSubColor;
				default:
					return {0, 0, 0, 0};
			}
		}
	}
	else if (drawType == DT_Mesh)
		return m_ActorWireColor;

	return {0, 0, 0, 0};
}

void Editor2DViewport::ReadLineColors()
{
	// Editor colors reside in System ini file.
	auto systemini = engine->packages->GetIniFile("System");

	// More can be added as needed
	Color brushColor = IniPropertyConverter<Color>::FromIniFile(*systemini, "Editor.EditorEngine", "C_BrushWire", Color());
	Color brushAddColor = IniPropertyConverter<Color>::FromIniFile(*systemini, "Editor.EditorEngine", "C_AddWire", Color());
	Color brushSubColor = IniPropertyConverter<Color>::FromIniFile(*systemini, "Editor.EditorEngine", "C_SubtractWire", Color());
	Color brushActorColor = IniPropertyConverter<Color>::FromIniFile(*systemini, "Editor.EditorEngine", "C_ActorWire", Color());
	Color brushSemiSolidColor = IniPropertyConverter<Color>::FromIniFile(*systemini, "Editor.EditorEngine", "C_SemiSolidWire", Color());
	Color brushNonSolidColor = IniPropertyConverter<Color>::FromIniFile(*systemini, "Editor.EditorEngine", "C_NonSolidWire", Color());
	Color brushMoverColor = IniPropertyConverter<Color>::FromIniFile(*systemini, "Editor.EditorEngine", "C_Mover", Color());

	// Manually convert Color to Colorf
	m_ActorWireColor = Colorf::fromRgba8(brushActorColor.R, brushActorColor.G, brushActorColor.B, brushActorColor.A);
	m_BrushWireColor = Colorf::fromRgba8(brushColor.R, brushColor.G, brushColor.B, brushColor.A);
	m_BrushAddColor = Colorf::fromRgba8(brushAddColor.R, brushAddColor.G, brushAddColor.B, brushAddColor.A);
	m_BrushSubColor = Colorf::fromRgba8(brushSubColor.R, brushSubColor.G, brushSubColor.B, brushSubColor.A);
	m_SemiSolidWireColor = Colorf::fromRgba8(brushSemiSolidColor.R, brushSemiSolidColor.G, brushSemiSolidColor.B, brushSemiSolidColor.A);
	m_NonSolidWireColor = Colorf::fromRgba8(brushNonSolidColor.R, brushNonSolidColor.G, brushNonSolidColor.B, brushNonSolidColor.A);
	m_MoverColor = Colorf::fromRgba8(brushMoverColor.R, brushMoverColor.G, brushMoverColor.B, brushMoverColor.A);
}

