#pragma once

#include "GameFolder.h"
#include "Commandlet/Commandlet.h"

class Frame;
class UObject;
struct LogMessageLine;

class DebuggerApp
{
public:
	int Main(Array<std::string> args);

	void WriteOutput(const std::string& text);
	std::string GetInput();

	void Exit() { ExitRequested = true; }
	void RunGame();

	Frame* GetCurrentFrame();

	GameLaunchInfo launchinfo;
	int CallstackIndex = 0;
	int ListSourceLineOffset = 0;

	static std::string ResetEscape();
	static std::string ColorEscape(int color);
	static std::string DeleteCharacterEscape(int count);
	static std::string EraseCharacterEscape(int count);
	static std::string CursorBackward(int count);
	static std::string NewLine();
	static std::string InputNewLine();

	bool ResumeProgram = false;

private:
	void CreateCommandlets();
	void Tick();

	void PrintLog(const LogMessageLine& line);
	static std::string ToFixed(float time);

	void OnCommandEntered();
	void WritePrompt();
	void EndPrompt();

	void WaitForInput();
	std::string ReadInput();

	void FrameDebugBreak();

	Array<std::unique_ptr<Commandlet>> Commandlets;

	std::string promptline;
	std::string cmdline;
	bool ExitRequested = false;

	bool PromptLineActive = false;
	bool GameRunning = false;
	bool InCommandlet = false;

	friend class Commandlet;
};
