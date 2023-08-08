
#include "Precomp.h"
#include "ListSourceCommandlet.h"
#include "DebuggerApp.h"
#include "VM/Frame.h"
#include "UObject/UTextBuffer.h"

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
			std::vector<std::string_view> lines;
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
			
				console->WriteOutput("    " + line + NewLine());
			}

			console->ListSourceLineOffset += linesToDisplay;

			console->WriteOutput(NewLine());
		}
	}
}

void ListSourceCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
