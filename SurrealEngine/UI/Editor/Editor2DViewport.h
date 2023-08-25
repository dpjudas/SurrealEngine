
#pragma once

#include "EditorViewport.h"
#include "Math/rotator.h"
#include "Math/coords.h"

class BspNode;

class Editor2DViewport : public EditorViewport
{
public:
	Editor2DViewport(const Coords& coords, Widget* parent);
	~Editor2DViewport();

protected:
	void OnPaint(Canvas* canvas) override;
	void OnMouseMove(const Point& pos) override;
	void OnMouseDown(const Point& pos, int key) override;
	void OnMouseDoubleclick(const Point& pos, int key) override;
	void OnMouseUp(const Point& pos, int key) override;
	void OnRawMouseMove(int dx, int dy) override;
	void OnKeyDown(EInputKey key) override;
	void OnKeyUp(EInputKey key) override;

private:
	void DrawLevel(Canvas* canvas);
	void DrawNode(Canvas* canvas, BspNode* node);
	void DrawNodeSurface(Canvas* canvas, BspNode* node);
	void DrawGrid(Canvas* canvas);
	void MoveCamera(float x, float y);

	bool MouseIsMoving = false;
	vec2 Location = vec2(0.0f);
	double Zoom = 0.01;
	Coords ViewCoords = Coords::Identity();
};
