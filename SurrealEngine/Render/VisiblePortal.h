#pragma once

class BspNode;

class VisiblePortal
{
public:
	BspNode* Node = nullptr;
	uint32_t PolyFlags = 0;
};
