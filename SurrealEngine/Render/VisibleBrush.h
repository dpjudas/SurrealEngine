#pragma once

class VisibleFrame;
class UActor;
class UModel;
class UMover;
struct Poly;

class VisibleBrush
{
public:
	void Draw(VisibleFrame* frame, UActor* actor);

private:
	void DrawBrushPoly(VisibleFrame* frame, UModel* model, const Poly& poly, int pass, UMover* mover);
};
