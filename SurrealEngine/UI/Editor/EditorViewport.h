
#pragma once

#include "UI/Core/Widget.h"
#include "Math/rotator.h"

class EditorViewport : public Widget
{
public:
	EditorViewport(Widget* parent);
	~EditorViewport();

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
	void MoveCamera(float x, float y, float z);

	bool MouseIsMoving = false;
	bool MouseIsPanning = false;
	vec3 Location = vec3(0.0f);
	Rotator Rotation = Rotator(0, 0, 0);
};
