#pragma once

class VisibleFrame;
class BspNode;

class VisibleNode
{
public:
	void Draw(VisibleFrame* frame);

	BspNode* Node = nullptr;
	bool Front = false;
	uint32_t PolyFlags = 0;
};
