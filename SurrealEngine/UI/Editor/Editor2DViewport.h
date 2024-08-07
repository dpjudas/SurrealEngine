
#pragma once

#include "EditorViewport.h"
#include "Math/rotator.h"
#include "Math/coords.h"

class Editor2DViewport : public EditorViewport
{
public:
	Editor2DViewport(const Coords& coords, Widget* parent);
	~Editor2DViewport();

protected:
	void OnPaint(Canvas* canvas) override;
	void OnMouseMove(const Point& pos) override;
	bool OnMouseDown(const Point& pos, InputKey key) override;
	bool OnMouseDoubleclick(const Point& pos, InputKey key) override;
	bool OnMouseUp(const Point& pos, InputKey key) override;
	bool OnMouseWheel(const Point& pos, InputKey key) override;
	void OnRawMouseMove(int dx, int dy) override;
	void OnKeyDown(InputKey key) override;
	void OnKeyUp(InputKey key) override;

private:
	void DrawLevel(Canvas* canvas);
	void DrawGrid(Canvas* canvas);
	void MoveCamera(float x, float y);

	bool MouseIsMoving = false;
	vec2 Location = vec2(0.0f);
	double Zoom = 0.05;
	Coords ViewCoords = Coords::Identity();
};
