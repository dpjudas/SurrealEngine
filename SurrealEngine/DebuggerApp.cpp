
#include "Precomp.h"
#include "DebuggerApp.h"
#include "CommandLine.h"
#include "GameFolder.h"
#include "Engine.h"
#include "Commandlet/ExtractCommandlet.h"
#include "Commandlet/QuitCommandlet.h"
#include "Commandlet/RunCommandlet.h"
#include "Commandlet/Debug/CollisionCommandlet.h"
#include "Commandlet/VM/BreakpointCommandlet.h"
#include "Commandlet/VM/CallstackCommandlet.h"
#include "Commandlet/VM/DisassemblyCommandlet.h"
#include "Commandlet/VM/ListSourceCommandlet.h"
#include "Commandlet/VM/LocalsCommandlet.h"
#include "Commandlet/VM/PrintCommandlet.h"
#include "Commandlet/VM/StepCommandlet.h"
#include "VM/Frame.h"
#include "UTF16.h"
#include <iostream>

#ifndef WIN32
#include <sys/select.h>
#include <unistd.h>
#endif

int DebuggerApp::Main(std::vector<std::string> args)
{
	WriteOutput(ColorEscape(96) + "Welcome to the Surreal Engine debugger!" + ResetEscape() + NewLine());
	WriteOutput(NewLine());
	WriteOutput("Type " + ColorEscape(92) + "help" + ResetEscape() + " for a list of commands" + NewLine());
	WriteOutput(NewLine());

	CreateCommandlets();

	CommandLine cmd(args);
	commandline = &cmd;
	launchinfo = GameFolderSelection::GetLaunchInfo();

	Frame::RunDebugger = [=]() { FrameDebugBreak(); };

	Engine engine(launchinfo);
	engine.tickDebugger = [&]() { Tick(); };
	engine.printLogDebugger = [&](const LogMessageLine& line) { PrintLog(line); };

	WritePrompt();
	while (!ExitRequested)
	{
		WaitForInput();
		Tick();
	}
	EndPrompt();

	return 0;
}

void DebuggerApp::CreateCommandlets()
{
	Commandlets.push_back(std::make_unique<RunCommandlet>());
	Commandlets.push_back(std::make_unique<ExtractCommandlet>());
	Commandlets.push_back(std::make_unique<ListBreakpointsCommandlet>());
	Commandlets.push_back(std::make_unique<BreakpointCommandlet>());
	Commandlets.push_back(std::make_unique<WatchpointCommandlet>());
	Commandlets.push_back(std::make_unique<DeleteBreakpointCommandlet>());
	Commandlets.push_back(std::make_unique<ClearBreakpointsCommandlet>());
	Commandlets.push_back(std::make_unique<EnableBreakpointCommandlet>());
	Commandlets.push_back(std::make_unique<DisableBreakpointCommandlet>());
	Commandlets.push_back(std::make_unique<CallstackCommandlet>());
	Commandlets.push_back(std::make_unique<SelectFrameCommandlet>());
	Commandlets.push_back(std::make_unique<DisassemblyCommandlet>());
	Commandlets.push_back(std::make_unique<ListSourceCommandlet>());
	Commandlets.push_back(std::make_unique<LocalsCommandlet>());
	Commandlets.push_back(std::make_unique<PrintCommandlet>());
	Commandlets.push_back(std::make_unique<StepInCommandlet>());
	Commandlets.push_back(std::make_unique<StepOverCommandlet>());
	Commandlets.push_back(std::make_unique<StepOutCommandlet>());
	Commandlets.push_back(std::make_unique<ContinueCommandlet>());
	Commandlets.push_back(std::make_unique<QuitCommandlet>());
	Commandlets.push_back(std::make_unique<CollisionCommandlet>());
}

void DebuggerApp::Tick()
{
	WritePrompt();

	std::string text = ReadInput();
	size_t pos = 0;
	while (pos < text.size())
	{
		size_t endpos = std::min(text.find(InputNewLine(), pos), text.size());
		size_t promptpos = promptline.size();
		for (size_t i = pos; i < endpos; i++)
		{
			if (text[i] >= 32 && text[i] != 127)
			{
				promptline.push_back(text[i]);
			}
			else if (text[i] == 127)
			{
				WriteOutput(promptline.substr(promptpos));
				if (!promptline.empty())
				{
					promptline.pop_back();
					promptpos = promptline.size();
					WriteOutput(CursorBackward(1) + DeleteCharacterEscape(1));
				}
			}
			else // Ignore arrow keys and such for now
			{
				break;
			}
		}
		WriteOutput(promptline.substr(promptpos));

		if (endpos != text.size())
		{
			EndPrompt();
			WriteOutput(NewLine());
			std::string cmdline = promptline;
			promptline.clear();
			OnCommandEntered(cmdline);
			WritePrompt();
		}
		pos = std::min(endpos + InputNewLine().size(), text.size());
	}
}

void DebuggerApp::WritePrompt()
{
	if (!PromptLineActive)
	{
		WriteOutput("> " + ColorEscape(92));
		PromptLineActive = true;
	}
}

void DebuggerApp::EndPrompt()
{
	if (PromptLineActive)
	{
		if (promptline.empty())
		{
			WriteOutput(ResetEscape() + CursorBackward(2) + DeleteCharacterEscape(2));
		}
		else
		{
			WriteOutput(ResetEscape() + NewLine());
		}
		PromptLineActive = false;
	}
}

void DebuggerApp::OnCommandEntered(const std::string& line)
{
	try
	{
		std::string command;
		std::string args;
		size_t pos = std::min(line.find(' '), line.size());
		if (pos == line.size())
		{
			command = line;
		}
		else
		{
			command = line.substr(0, pos);
			pos = std::min(line.find_first_not_of(' ', pos + 1), line.size());
			args = line.substr(pos);
		}

		if (command == "help")
		{
			if (args.empty())
			{
				for (auto& cmdlet : Commandlets)
				{
					std::string longname = cmdlet->GetLongFormName();
					std::string description = cmdlet->GetShortDescription();
					if (longname.size() < 20)
						longname.resize(20, ' ');
					WriteOutput(ColorEscape(92) + longname + ResetEscape() + " " + description + NewLine());
				}
			}
			else
			{
				for (auto& cmdlet : Commandlets)
				{
					if (cmdlet->GetShortFormName() == args || cmdlet->GetLongFormName() == args)
					{
						cmdlet->OnPrintHelp(this);
					}
				}
			}
			WriteOutput(NewLine());
		}
		else
		{
			bool found = false;
			for (auto& cmdlet : Commandlets)
			{
				if (cmdlet->GetShortFormName() == command || cmdlet->GetLongFormName() == command)
				{
					cmdlet->OnCommand(this, args);
					found = true;
					break;
				}
			}

			if (!found)
				WriteOutput("Unknown command" + NewLine());
		}
	}
	catch (const std::exception& e)
	{
		EndPrompt();
		if (!ExitRequested)
			WriteOutput(ColorEscape(91) + e.what() + ResetEscape() + NewLine());
	}
}

void DebuggerApp::RunGame()
{
	if (GameRunning)
	{
		WriteOutput(ColorEscape(91) + "Game is already running!" + ResetEscape() + NewLine());
	}
	else
	{
		if (!launchinfo.folder.empty())
		{
			WriteOutput("Launching game..." + NewLine());
			GameRunning = true;
			engine->Run();
			GameRunning = false;
			EndPrompt();
			WriteOutput("Game ended." + NewLine());
		}
	}
}

std::string DebuggerApp::ToFixed(float time)
{
	std::string fixedTime = std::to_string((int64_t)(time * 1000.0));
	if (fixedTime.size() < 4)
		fixedTime.resize(4, '0');
	return fixedTime.substr(0, fixedTime.size() - 3) + "." + fixedTime.substr(fixedTime.size() - 3);
}

void DebuggerApp::PrintLog(const LogMessageLine& line)
{
	if (PromptLineActive)
	{
		WriteOutput(ResetEscape() + CursorBackward(2 + (int)promptline.size()) + DeleteCharacterEscape(2 + (int)promptline.size()));
	}

	std::string timeText = ToFixed(line.Time);
	if (timeText.size() < 10)
		timeText.resize(10, ' ');

	std::string sourceText = line.Source;
	if (sourceText.size() < 50)
		sourceText.resize(50, ' ');

	WriteOutput(ColorEscape(96) + timeText + ResetEscape() + " " + sourceText + " " + line.Text + NewLine());

	if (PromptLineActive)
	{
		WriteOutput("> " + ColorEscape(92));
		WriteOutput(promptline);
	}
}

void DebuggerApp::FrameDebugBreak()
{
	EndPrompt();

	CallstackIndex = 0;

	if (!Frame::ExceptionText.empty())
	{
		WriteOutput(NewLine() + ColorEscape(91) + Frame::ExceptionText + ResetEscape() + NewLine() + NewLine());
	}
	else
	{
		WriteOutput(NewLine() + "Debugger breakpoint encountered" + NewLine() + NewLine());
	}

	WritePrompt();
	while (!ExitRequested && !ResumeProgram)
	{
		WaitForInput();
		Tick();
	}
	ResumeProgram = false;

	if (ExitRequested)
		throw std::runtime_error("Debugger exit");
}

Frame* DebuggerApp::GetCurrentFrame()
{
	return Frame::Callstack[Frame::Callstack.size() - 1 - CallstackIndex];
}

std::string DebuggerApp::ResetEscape()
{
	return "\x1b[0m";
}

std::string DebuggerApp::ColorEscape(int color)
{
	return "\x1b[" + std::to_string(color) + "m";
}

std::string DebuggerApp::DeleteCharacterEscape(int count)
{
	return "\x1b[" + std::to_string(count) + "P";
}

std::string DebuggerApp::EraseCharacterEscape(int count)
{
	return "\x1b[" + std::to_string(count) + "X";
}

std::string DebuggerApp::CursorBackward(int count)
{
	return "\x1b[" + std::to_string(count) + "D";
}

std::string DebuggerApp::NewLine()
{
#ifdef WIN32
	return "\r\n";
#else
	return "\n";
#endif
}

std::string DebuggerApp::InputNewLine()
{
#ifdef WIN32
	return "\r";
#else
	return "\n";
#endif
}

void DebuggerApp::WaitForInput()
{
#ifdef WIN32
	WaitForSingleObject(GetStdHandle(STD_INPUT_HANDLE), INFINITE);
#else
	fd_set rfds;
	timeval tv;
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	tv.tv_sec = 365*24*60*60;
	tv.tv_usec = 0;
	select(1, &rfds, NULL, NULL, &tv);
#endif
}

std::string DebuggerApp::ReadInput()
{
#ifdef WIN32
	std::string text;
	while (WaitForSingleObject(GetStdHandle(STD_INPUT_HANDLE), 0) == WAIT_OBJECT_0)
	{
		// We have to do it in this incredibly stupid way because if we just use ReadFile or ReadConsole then the input handle remains signaled!

		INPUT_RECORD buffer[256];
		DWORD eventsRead = 0;
		if (!ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), buffer, 256, &eventsRead))
			break;

		std::wstring buffer16;
		for (DWORD i = 0; i < eventsRead; i++)
		{
			if (buffer[i].EventType == KEY_EVENT && buffer[i].Event.KeyEvent.bKeyDown && buffer[i].Event.KeyEvent.uChar.UnicodeChar != 0)
			{
				for (int j = 0; j < buffer[i].Event.KeyEvent.wRepeatCount; j++)
				{
					buffer16.push_back(buffer[i].Event.KeyEvent.uChar.UnicodeChar);
				}
			}
		}
		text += from_utf16(buffer16);
	}
	return text;
#else
	std::string text;
	while (true)
	{
		fd_set rfds;
		timeval tv;
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		int retval = select(1, &rfds, NULL, NULL, &tv);
		if (retval <= 0)
			break;

		char buffer[1024];
		auto bytesread = read(0, buffer, 1024);
		if (bytesread <= 0)
			break;

		text += std::string(buffer, bytesread);
	}
	return text;
#endif
}

void DebuggerApp::WriteOutput(const std::string& text)
{
	if (text.empty())
		return;

#ifdef WIN32
	std::wstring text16 = to_utf16(text);
	size_t pos = 0;
	while (pos < text16.size())
	{
		DWORD written = 0;
		if (!WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), text16.data() + pos, (DWORD)(text16.size() - pos), &written, nullptr))
			break;
		pos += written;
	}
#else
	size_t pos = 0;
	while (pos < text.size())
	{
		auto byteswritten = write(1, text.data() + pos, text.size() - pos);
		if (byteswritten <= 0)
			break;
		pos += byteswritten;
	}
#endif
}
