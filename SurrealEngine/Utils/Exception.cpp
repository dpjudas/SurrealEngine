#include <ios>
#include <iostream>
#include <sstream>
#include <vector>
#include "Utils/Exception.h"
#include "VM/Frame.h"

#ifdef WIN32

#include <Windows.h>
#include <DbgHelp.h>

struct InitDbgHelp
{
	InitDbgHelp()
	{
		bHasSymbols = false;
		HANDLE hCurrentProcess = GetCurrentProcess();
		SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
		if (DuplicateHandle(hCurrentProcess, hCurrentProcess, hCurrentProcess, &hProcess, 0, FALSE, DUPLICATE_SAME_ACCESS))
		{
			// initialize symbols
			bHasSymbols = SymInitialize(hProcess, NULL, TRUE);
		}
	}

	~InitDbgHelp()
	{
		SymCleanup(hProcess);
	}

	HANDLE hProcess;
	bool bHasSymbols;
};

#elif defined __linux__

#include <execinfo.h>
#include <cxxabi.h>
#include <cstring>
#include <cstdlib>
#include <memory>

#else

#error "Exception.cpp: Unsupported Target"

#endif

std::mutex Exception::mutex;

int Exception::CaptureStackFrames(std::ostringstream& sstream, int maxframes)
{
/////////////////////////////////////////////////////////////////////
#if defined _WIN64

	static InitDbgHelp dbgHelp;

	CONTEXT context;
	RtlCaptureContext(&context);

	UNWIND_HISTORY_TABLE history;
	memset(&history, 0, sizeof(UNWIND_HISTORY_TABLE));

	ULONG64 establisherframe = 0;
	PVOID handlerdata = nullptr;
	HANDLE hProcess = dbgHelp.hProcess;

	int frame;
	for (frame = 0; frame < maxframes; frame++)
	{

		ULONG64 imagebase;
		PRUNTIME_FUNCTION rtfunc = RtlLookupFunctionEntry(context.Rip, &imagebase, &history);

		KNONVOLATILE_CONTEXT_POINTERS nvcontext;
		memset(&nvcontext, 0, sizeof(KNONVOLATILE_CONTEXT_POINTERS));
		if (!rtfunc)
		{
			// Leaf function
			context.Rip = (ULONG64)(*(PULONG64)context.Rsp);
			context.Rsp += 8;
		}
		else
		{
			RtlVirtualUnwind(UNW_FLAG_NHANDLER, imagebase, context.Rip, rtfunc, &context, &handlerdata, &establisherframe, &nvcontext);
		}

		if (!context.Rip)
			break;

		// Skip Exception::Throw frame
		if (!frame)
			continue;

		char sibuf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];

		bool gotSymbol = false;
		DWORD64 symDisp = 0;
		DWORD lineDisp = 0;
		PSYMBOL_INFO si = reinterpret_cast<PSYMBOL_INFO>(sibuf);
		IMAGEHLP_LINE ih;
		si->SizeOfStruct = sizeof(SYMBOL_INFO);
		si->MaxNameLen = MAX_SYM_NAME;

		mutex.lock();

		if (dbgHelp.bHasSymbols)
			gotSymbol = SymFromAddr(hProcess, (DWORD64)context.Rip, &symDisp, si);

		mutex.unlock();

		if (gotSymbol)
		{
			if (strncmp(si->Name, "wmain", si->NameLen) == 0)
				break;

			SymGetLineFromAddr(hProcess, (DWORD64)context.Rip, &lineDisp, &ih);
			std::string funcStr(&si->Name[0], si->NameLen);
			sstream << "at " << funcStr << " line " << ih.LineNumber << std::endl;
		}
		else
		{
			sstream << "at 0x" << std::hex << context.Rip << std::dec << std::endl;
		}
	}

	return frame;


/////////////////////////////////////////////////////////////////////
#elif defined __linux__

	return 0;

/////////////////////////////////////////////////////////////////////
#endif
}

void Exception::Throw(const std::string& text)
{
	std::ostringstream sstream;
	sstream << text << std::endl;

	std::string scriptcallstack = Frame::GetCallstack();
	if (scriptcallstack.empty())
	{
		CaptureStackFrames(sstream, 32);
	}
	else
	{
		sstream << std::endl << "Script call stack:" << std::endl << scriptcallstack << std::endl << std::endl << "Native call stack: " << std::endl;
		CaptureStackFrames(sstream, 4);
	}

	throw std::runtime_error(sstream.str());
}
