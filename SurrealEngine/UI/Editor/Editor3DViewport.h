
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
	void UpdateMovement();
	void MoveCamera(float x, float y, float z);
	float CalcTimeElapsed();

	bool MouseIsMoving = false;
	bool MouseIsPanning = false;
	vec3 Location = vec3(0.0f);
	Rotator Rotation = Rotator(0, 0, 0);

	vec3 LastLocation = vec3(0.0f);
	Rotator LastRotation = Rotator(0, 0, 0);

	bool moveLeft = false;
	bool moveRight = false;
	bool moveUp = false;
	bool moveDown = false;
	bool moveForwards = false;
	bool moveBackwards = false;

	uint64_t lastTime = 0;
};
