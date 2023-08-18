
#pragma once

#include "EditorViewport.h"
#include "Math/rotator.h"

class Editor2DViewport : public EditorViewport
{
public:
	Editor2DViewport(Widget* parent);
	~Editor2DViewport();

protected:
	void OnPaint(Canvas* canvas) override;
	void OnMouseMove(const Point& pos) override;
	void OnMouseDown(const Point& pos, int key) override;
	void OnMouseDoubleclick(const Point& pos, int key) override;
	void OnMouseUp(const Point& pos, int key) override;
	void OnRawMouseMove(int dx, int dy) override;
	void OnKeyDown(int key) override;
	void OnKeyUp(int key) override;

private:
	void MoveCamera(float x, float y);

	bool MouseIsMoving = false;
	vec2 Location = vec2(0.0f);
	float Zoom = 1.5f;
};
