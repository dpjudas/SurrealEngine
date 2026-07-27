#pragma once

#include "Packages/Engine/Actors/Decoration/UDecoration.h"

class UDeusExDecoration : public UDecoration
{
public:
	using UDecoration::UDecoration;

	void ConBindEvents();
};
