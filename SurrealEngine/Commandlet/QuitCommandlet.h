#pragma once

#include "Commandlet/Commandlet.h"

class QuitCommandlet : public Commandlet
{
public:
	QuitCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;
};
