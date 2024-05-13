#pragma once

#include "Commandlet/Commandlet.h"

class ExportCommandlet : public Commandlet
{
public:
	ExportCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;

private:
	void ExportAll(DebuggerApp* console, std::vector<std::string>& packages);
	void ExportScripts(DebuggerApp* console, std::vector<std::string>& packages);
	void ExportTextures(DebuggerApp* console, std::vector<std::string>& packages);
	void ExportSounds(DebuggerApp* console, std::vector<std::string>& packages);
	void ExportMusic(DebuggerApp* console, std::vector<std::string>& packages);
	void ExportMeshes(DebuggerApp* console, std::vector<std::string>& packages);
	void ExportLevel(DebuggerApp* console, std::vector<std::string>& packages);
};
