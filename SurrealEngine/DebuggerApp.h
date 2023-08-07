#pragma once

#include "GameFolder.h"

class DebuggerApp
{
public:
	int Main(std::vector<std::string> args);

private:
	void RunGame();
	void Tick();

	void OnCommandEntered(const std::string& line);
	void WritePrompt();
	void EndPrompt();

	void WaitForInput();
	std::string ReadInput();
	void WriteOutput(const std::string& text);

	GameLaunchInfo launchinfo;

	std::string promptline;
	bool ExitRequested = false;

	bool PromptLineActive = false;
	bool GameRunning = false;

	static std::string ResetEscape();
	static std::string ColorEscape(int color);
	static std::string DeleteCharacterEscape(int count);
	static std::string EraseCharacterEscape(int count);
	static std::string CursorBackward(int count);
	static std::string NewLine();
	static std::string InputNewLine();
};
