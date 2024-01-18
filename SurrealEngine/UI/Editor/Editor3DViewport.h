
#pragma once

#include "EditorViewport.h"
#include "Math/rotator.h"

class Editor3DViewport : public EditorViewport
{
public:
	Editor3DViewport(Widget* parent);
	~Editor3DViewport();

protected:
	void OnPaint(Canvas* canvas) override;
	void OnMouseMove(const Point& pos) override;
	bool OnMouseDown(const Point& pos, InputKey key) override;
	bool OnMouseDoubleclick(const Point& pos, InputKey key) override;
	bool OnMouseUp(const Point& pos, InputKey key) override;
	void OnRawMouseMove(int dx, int dy) override;
	void OnKeyDown(InputKey key) override;
	void OnKeyUp(InputKey key) override;

private:
	void MoveCamera(float x, float y, float z);

	bool MouseIsMoving = false;
	bool MouseIsPanning = false;
	vec3 Location = vec3(0.0f);
	Rotator Rotation = Rotator(0, 0, 0);
};
