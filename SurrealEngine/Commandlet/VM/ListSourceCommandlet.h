#pragma once

#include "Commandlet/Commandlet.h"

class ListSourceCommandlet : public Commandlet
{
public:
	ListSourceCommandlet();

	void OnCommand(DebuggerApp* console, const std::string& args) override;
	void OnPrintHelp(DebuggerApp* console) override;

private:
	struct TextSpan
	{
		size_t start = 0;
		size_t end = 0;
		int color = 0;
	};

	std::string SyntaxHighlight(const std::string& text);
	Array<TextSpan> CreateTextSpans(const std::string& text);
	void InsertHighlight(Array<TextSpan>& spans, size_t pos, size_t length, int color);
};
