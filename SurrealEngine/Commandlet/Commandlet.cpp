
#include "Precomp.h"
#include "Commandlet.h"
#include "DebuggerApp.h"

std::string Commandlet::ResetEscape()
{
	return DebuggerApp::ResetEscape();
}

std::string Commandlet::ColorEscape(int color)
{
	return DebuggerApp::ColorEscape(color);
}

std::string Commandlet::DeleteCharacterEscape(int count)
{
	return DebuggerApp::DeleteCharacterEscape(count);
}

std::string Commandlet::EraseCharacterEscape(int count)
{
	return DebuggerApp::EraseCharacterEscape(count);
}

std::string Commandlet::CursorBackward(int count)
{
	return DebuggerApp::CursorBackward(count);
}

std::string Commandlet::NewLine()
{
	return DebuggerApp::NewLine();
}
