
#include "Precomp.h"
#include "DebuggerApp.h"
#include "CommandLine.h"
#include "GameFolder.h"
#include "Engine.h"
#include "UObject/NativeObjExtractor.h"
#include "VM/NativeFuncExtractor.h"
#include "VM/Frame.h"
#include "File.h"
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

	CommandLine cmd(args);
	commandline = &cmd;
	launchinfo = GameFolderSelection::GetLaunchInfo();

	Frame::RunDebugger = [=]() { FrameDebugBreak(); };

	WritePrompt();
	while (!ExitRequested)
	{
		WaitForInput();
		Tick();
	}

	return 0;
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

		if (command == "quit" || command == "exit")
		{
			ExitRequested = true;
		}
		else if (command == "run")
		{
			RunGame();
		}
		else if (command == "extract nativefunc")
		{
			if (!launchinfo.folder.empty())
			{
				Engine engine(launchinfo);
				File::write_all_text("nativefuncs.txt", NativeFuncExtractor::Run(engine.packages.get()));
			}
		}
		else if (command == "extract-nativeobj")
		{
			if (!launchinfo.folder.empty())
			{
				Engine engine(launchinfo);
				File::write_all_text("nativeobjs.txt", NativeObjExtractor::Run(engine.packages.get()));
			}
		}
		else if (command == "bt")
		{
			PrintCallStack();
		}
		else if (command == "locals")
		{
			PrintLocals();
		}
		else if (command == "print")
		{
			PrintObject(args);
		}
		else if (command == "disasm")
		{
			PrintDisassembly();
		}
		else if (command == "help")
		{
			WriteOutput("Haha, there is no help!" + NewLine());
		}
		else
		{
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
			Engine engine(launchinfo);
			engine.tickDebugger = [&]() { Tick(); };
			engine.Run();
			GameRunning = false;
			EndPrompt();
			WriteOutput("Game ended." + NewLine());
		}
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

	PrintCallStack();

	WritePrompt();
	while (!ExitRequested)
	{
		WaitForInput();
		Tick();
	}
	throw std::runtime_error("Debugger exit");
}

void DebuggerApp::PrintCallStack()
{
	int index = 0;
	for (auto it = Frame::Callstack.rbegin(); it != Frame::Callstack.rend(); ++it)
	{
		Frame* frame = *it;
		UStruct* func = frame->Func;
		if (func)
		{
			std::string name;
			for (UStruct* s = func; s != nullptr; s = s->StructParent)
			{
				if (name.empty())
					name = s->Name.ToString();
				else
					name = s->Name.ToString() + "." + name;
			}

			if (name.size() < 40)
				name.resize(40, ' ');

			WriteOutput("#" + std::to_string(index) + ": " + ColorEscape(96) + name + ResetEscape() + " line " + ColorEscape(96) + std::to_string(func->Line) + ResetEscape() + NewLine());
		}
		index++;
	}
	WriteOutput(NewLine());
}

Frame* DebuggerApp::GetCurrentFrame()
{
	return Frame::Callstack[Frame::Callstack.size() - 1 - CallstackIndex];
}

void DebuggerApp::PrintLocals()
{
	Frame* frame = GetCurrentFrame();
	if (frame)
	{
		for (UProperty* prop : frame->Func->Properties)
		{
			void* ptr = ((uint8_t*)frame->Variables.get()) + prop->DataOffset;

			std::string name = prop->Name.ToString();
			std::string value = prop->PrintValue(ptr);

			if (name.size() < 40)
				name.resize(40, ' ');

			WriteOutput(ColorEscape(96) + name + ResetEscape() + " " + ColorEscape(96) + value + ResetEscape() + NewLine());
		}
	}
	WriteOutput(NewLine());
}

void DebuggerApp::PrintObject(const std::string& args)
{
	Frame* frame = GetCurrentFrame();

	UObject* obj = nullptr;
	if (NameString("self") == args)
	{
		obj = frame->Object;
	}
	else
	{
		for (UProperty* prop : frame->Func->Properties)
		{
			if (prop->Name == args && (UObject::TryCast<UObjectProperty>(prop) || UObject::TryCast<UClassProperty>(prop)))
			{
				void* ptr = ((uint8_t*)frame->Variables.get()) + prop->DataOffset;
				obj = *(UObject**)ptr;
				break;
			}
		}
	}

	if (!obj)
	{
		WriteOutput(ColorEscape(96) + "None" + ResetEscape() + NewLine());
		return;
	}

	auto props = obj->PropertyData.Class->Properties;
	std::stable_sort(props.begin(), props.end(), [](UProperty* a, UProperty* b) { return a->Name < b->Name; });

	for (UProperty* prop : props)
	{
		void* ptr = obj->PropertyData.Ptr(prop);

		std::string name = prop->Name.ToString();
		std::string value = prop->PrintValue(ptr);

		if (name.size() < 40)
			name.resize(40, ' ');

		WriteOutput(ColorEscape(96) + name + ResetEscape() + " " + ColorEscape(96) + value + ResetEscape() + NewLine());
	}
}

void DebuggerApp::PrintDisassembly()
{
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
		if (byteswrittes <= 0)
			break;
		pos += byteswritten;
	}
#endif
}
