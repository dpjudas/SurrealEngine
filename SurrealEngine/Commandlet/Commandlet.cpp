
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

std::vector<std::string> Commandlet::SplitString(const std::string& text)
{
	std::vector<std::string> result;

	size_t pos = 0;
	while (pos < text.size())
	{
		// Eat whitespace
		pos = std::min(text.find_first_not_of(' ', pos), text.size());

		// Find next whitespace and grab the text in between
		size_t endpos = std::min(text.find_first_of(' ', pos), text.size());
		if (pos < endpos)
		{
			result.push_back(text.substr(pos, endpos - pos));
		}
		pos = endpos;
	}

	return result;
}
