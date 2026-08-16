
#include "Precomp.h"
#include "Utils/Exception.h"
#include "VM/Frame.h"
#include <ios>
#include <iostream>
#include <sstream>
#include <vector>

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

#elif defined __linux__ || defined __APPLE__

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
#if defined(_WIN64) && (defined(_M_X64) || defined(__x86_64__))

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
#elif defined __APPLE__

	if (maxframes <= 0)
		return 0;

	std::vector<void*> frames(static_cast<size_t>(maxframes));
	const int frameCount = backtrace(frames.data(), maxframes);
	if (frameCount <= 2)
		return 0;

	using SymbolList = std::unique_ptr<char*, decltype(&std::free)>;
	SymbolList symbols(backtrace_symbols(frames.data(), frameCount), &std::free);
	if (!symbols)
		return 0;

	auto formatSymbol = [](const char* rawSymbol)
	{
		if (!rawSymbol)
			return std::string("<unknown>");

		std::string symbol(rawSymbol);
		const size_t offsetPos = symbol.rfind(" + ");
		const size_t addressPos = symbol.find("0x");

		if (offsetPos == std::string::npos || addressPos == std::string::npos)
			return symbol;

		const size_t addressEnd = symbol.find_first_of(" \t", addressPos);
		if (addressEnd == std::string::npos)
			return symbol;

		const size_t nameBegin = symbol.find_first_not_of(" \t", addressEnd);
		if (nameBegin == std::string::npos || nameBegin >= offsetPos)
			return symbol;

		std::string functionName = symbol.substr(nameBegin, offsetPos - nameBegin);
		std::string mangledName = functionName;

		// Mach-O symbols have some underscores
		if (mangledName.compare(0, 3, "__Z") == 0)
			mangledName.erase(0, 1);

		if (mangledName.compare(0, 2, "_Z") == 0)
		{
			int status = 0;
			std::unique_ptr<char, decltype(&std::free)> demangled(
				abi::__cxa_demangle(mangledName.c_str(), nullptr, nullptr, &status),
				&std::free);

			if (status == 0 && demangled)
				functionName = demangled.get();
		}

		return functionName + symbol.substr(offsetPos);
	};

	for (size_t frame = 0; frame < frameCount; frame++)
	{
		sstream << "at " << formatSymbol(symbols.get()[frame]) << std::endl;
	}

	return frameCount;

/////////////////////////////////////////////////////////////////////
#else

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
