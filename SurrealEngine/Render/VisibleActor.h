#pragma once

#include "UObject/UActor.h"

class VisibleFrame;

class VisibleActor
{
public:
	void Process(VisibleFrame* frame, UActor* actor);
	void DrawOpaque(VisibleFrame* frame);
	void DrawTranslucent(VisibleFrame* frame);

	EDrawType Type = DT_None;
	UActor* Actor = nullptr;
};
