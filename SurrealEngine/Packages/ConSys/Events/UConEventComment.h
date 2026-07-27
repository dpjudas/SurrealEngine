#pragma once

#include "UConEvent.h"

class UConEventComment : public UConEvent
{
public:
	using UConEvent::UConEvent;

	std::string& commentText() { return Value<std::string>(PropOffsets_ConEventComment.commentText); }
};
