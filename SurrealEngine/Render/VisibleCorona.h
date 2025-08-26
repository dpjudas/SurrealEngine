#pragma once

class VisibleFrame;
class UActor;

class VisibleCorona
{
public:
	VisibleCorona(UActor* actor) : light(actor) {}

	void Draw(VisibleFrame* frame);

private:
	UActor* light = nullptr;
};
