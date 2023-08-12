#pragma once

#include "Commandlet/Commandlet.h"

class CollisionCommandlet : public Commandlet
{
public:
	CollisionCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
