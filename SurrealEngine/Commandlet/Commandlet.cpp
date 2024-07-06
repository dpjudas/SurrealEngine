
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

Array<std::string> Commandlet::SplitString(const std::string& text, char separator)
{
	Array<std::string> result;

	size_t pos = 0;
	while (pos < text.size())
	{
		// Eat separator
		pos = std::min(text.find_first_not_of(separator, pos), text.size());

		// Find next separator and grab the text in between
		size_t endpos = std::min(text.find_first_of(separator, pos), text.size());
		if (pos < endpos)
		{
			result.push_back(text.substr(pos, endpos - pos));
		}
		pos = endpos;
	}

	return result;
}
