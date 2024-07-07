
#include "Precomp.h"
#include "ListSourceCommandlet.h"
#include "DebuggerApp.h"
#include "VM/Frame.h"
#include "UObject/UTextBuffer.h"
#include <regex>

ListSourceCommandlet::ListSourceCommandlet()
{
	SetShortFormName("l");
	SetLongFormName("list");
	SetShortDescription("Show source code");
}

void ListSourceCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	Frame* frame = console->GetCurrentFrame();
	if (frame)
	{
		UTextBuffer* scriptText = nullptr;
		UStruct* func = frame->Func;
		while (func && !scriptText)
		{
			scriptText = func->ScriptText;
			func = func->StructParent;
		}

		if (scriptText)
		{
			Array<std::string_view> lines;
			size_t pos = 0;
			while (pos < scriptText->Text.size())
			{
				size_t endpos = std::min(scriptText->Text.find("\r\n", pos), scriptText->Text.size());
				lines.push_back(std::string_view(scriptText->Text).substr(pos, endpos - pos));
				pos = endpos + 2;
			}
			
			const int tabsize = 4;
			const int linesToDisplay = 15;

			size_t startLine = std::max((int)frame->Func->Line - 5 + console->ListSourceLineOffset, 0);
			size_t endLine = std::min(startLine + linesToDisplay, lines.size());
			for (size_t i = startLine; i < endLine; i++)
			{
				std::string line;
				int linepos = 0;
				for (char c : lines[i])
				{
					if (c == '\t')
					{
						int tabpos = (linepos / tabsize + 1) * tabsize;
						while (linepos < tabpos)
						{
							line.push_back(' ');
							linepos++;
						}
					}
					else
					{
						line.push_back(c);
					}
					linepos++;
				}
			
				console->WriteOutput("    " + SyntaxHighlight(line) + NewLine());
			}

			console->ListSourceLineOffset += linesToDisplay;

			console->WriteOutput(NewLine());
		}
	}
}

void ListSourceCommandlet::OnPrintHelp(DebuggerApp* console)
{
}

std::string ListSourceCommandlet::SyntaxHighlight(const std::string& text)
{
	Array<ListSourceCommandlet::TextSpan> spans = CreateTextSpans(text);

	std::string output;
	output.reserve(text.size() + (spans.size() + 1) * 5);

	int color = 0;
	for (const TextSpan& span : spans)
	{
		if (span.color != color)
		{
			color = span.color;
			if (color != 0)
				output += ColorEscape(color);
			else
				output += ResetEscape();
		}
		output.insert(output.end(), text.begin() + span.start, text.begin() + span.end);
	}

	if (color != 0)
		output += ResetEscape();

	return output;
}

Array<ListSourceCommandlet::TextSpan> ListSourceCommandlet::CreateTextSpans(const std::string& text)
{
	struct Pattern
	{
		std::regex regex;
		int color;
		size_t group;
	};

	static Array<Pattern> patterns =
	{
		{ std::regex("(^|[^a-z_0-9])(local|function|for(each)?|if|then|else(\\s+(if|do|while))?|do|while|return|break|goto|optional|coerce|class|enum|none|out|event|simulated(\\s+(function|event))?|state|true|false|super|ignores|static(\\s+function)?|exec)($|[^a-z_0-9])", std::regex::icase), 96, 2 }, // keywords
		{ std::regex("[+\\-=/\\\\$[\\]\\!&~\\^,;()*]+"), 90, 0 }, // symbols
		{ std::regex("(^|[^a-z_0-9])(bool|byte|int|float|name|string|vector|rotator)($|[^a-z_0-9])", std::regex::icase), 93, 2 }, // type keywords
		{ std::regex("['\"].*?['\"]"), 97, 0 }, // quoted text
	};

	Array<TextSpan> spans;
	spans.push_back({ 0, text.length(), 0 });

	for (const Pattern& pattern : patterns)
	{
		try
		{
			std::regex regex(pattern.regex);
			for (std::sregex_iterator it(text.begin(), text.end(), regex); it != std::sregex_iterator(); ++it)
			{
				std::smatch match = *it;
				InsertHighlight(spans, match.position(pattern.group), match.length(pattern.group), pattern.color);
			}
		}
		catch (...)
		{
			// std::regex throws if it encounters invalid characters. Skip highlighting if this should happen.
		}
	}

	return spans;
}

void ListSourceCommandlet::InsertHighlight(Array<TextSpan>& spans, size_t pos, size_t length, int color)
{
	size_t start = pos;
	size_t end = pos + length;

	size_t index = 0;
	while (index < spans.size())
	{
		TextSpan& span = spans[index];
		if (span.start >= end)
		{
			//         [span]
			// [hlht]
			break;
		}
		else if (span.end <= start)
		{
			// [span]
			//         [hlht]
			index++;
		}
		else if (span.start >= start && span.end <= end)
		{
			//    [span]
			// [---hlht---]
			span.color = color;
			index++;
		}
		else if (span.start < start && span.end > end)
		{
			// [---span---]
			//    [hlht]
			TextSpan newspans[2] = { { start, end, color }, span };
			span.end = start;
			newspans[1].start = end;
			spans.insert(spans.begin() + index + 1, newspans, newspans + 2);
			index += 3;
		}
		else if (span.end > end)
		{
			//    [span]
			// [hlht]
			TextSpan newspan = { span.start, end, color };
			span.start = end;
			spans.insert(spans.begin() + index, newspan);
			index += 2;
		}
		else
		{
			// [span]
			//    [hlht]
			TextSpan newspan = { start, span.end, color };
			span.end = start;
			spans.insert(spans.begin() + index + 1, newspan);
			index += 2;
		}
	}
}
