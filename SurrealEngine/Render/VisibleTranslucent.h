#pragma once

#include "VisibleNode.h"
#include "VisibleActor.h"
#include "Engine.h"
#include "UObject/ULevel.h"
#include "UObject/UActor.h"

class VisibleFrame;

class VisibleTranslucent
{
public:
	VisibleTranslucent(const VisibleNode& node, float distSqr) : Node(node), DistSqr(distSqr) {}
	VisibleTranslucent(const VisibleActor& actor, float distSqr) : Actor(actor), DistSqr(distSqr) {}

	VisibleActor Actor;
	VisibleNode Node;
	float DistSqr = 0.0f;

	void Draw(VisibleFrame* frame)
	{
		if (Actor.Actor)
			Actor.DrawTranslucent(frame);
		else
			Node.Draw(frame);
	}
};
