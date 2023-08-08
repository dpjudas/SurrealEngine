#pragma once

#include "GameFolder.h"
#include "Commandlet/Commandlet.h"

class Frame;
class UObject;

class DebuggerApp
{
public:
	int Main(std::vector<std::string> args);

	void WriteOutput(const std::string& text);

	void Exit() { ExitRequested = true; }
	void RunGame();

	Frame* GetCurrentFrame();

	GameLaunchInfo launchinfo;
	int CallstackIndex = 0;
	int ListSourceLineOffset = 0;

private:
	void CreateCommandlets();
	void Tick();

	void OnCommandEntered(const std::string& line);
	void WritePrompt();
	void EndPrompt();

	void WaitForInput();
	std::string ReadInput();

	void FrameDebugBreak();

	std::vector<std::unique_ptr<Commandlet>> Commandlets;

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

	friend class Commandlet;
};
