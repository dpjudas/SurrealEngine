
#include "Precomp.h"
#include "UDXExtString.h"
#include "Utils/Logger.h"

void UDXExtString::Load(ObjectStream* stream)
{
	UObject::Load(stream);

	std::string text = stream->ReadString();
	if (!Text().empty())
		LogMessage("ExtString already has text content in its Text property!");
	Text() = text;
	stream->ThrowIfNotEnd();
}


void UDXExtString::AppendText(const std::string& newText)
{
	LogMessage("Usage check: ExtString.AppendText");
	Text() += newText;
}

int UDXExtString::GetFirstTextPart(std::string& outText)
{
	if (!Text().empty())
	{
		size_t len = Text().size();
		size_t N = std::min(len - 1, (size_t)239);
		outText = Text().substr(0, N);
	}
	else
	{
		outText = {};
	}
	SpeechPage() = 0; // seems it does that
	return 0;
}

int UDXExtString::GetNextTextPart(std::string& outText)
{
	/*
	speechPage() += 1;
	int len = Text().size();
	int pages = (len + 238) / 239;
	if (SpeechPage() > pages)
		return 1;

	int start = speechPage() * 239;
	outText = Text().substr(start, 239);
	*/
	LogUnimplemented("ExtString.GetNextTextPart");
	outText = "";
	return 0;
}

std::string& UDXExtString::GetText()
{
	LogMessage("Usage check: ExtString.GetText");
	return Text();
}

int UDXExtString::GetTextLength()
{
	LogMessage("Usage check: ExtString.GetTextLength");
	return (int)Text().length();
}

int UDXExtString::GetTextPart(int startPos, int count, std::string& outText)
{
	LogMessage("Usage check: ExtString.GetTextPart");
	outText = Text().substr(startPos, count);
	return (int)outText.length();
}

void UDXExtString::SetText(const std::string& newText)
{
	LogMessage("Usage check: ExtString.SetText");
	Text() = newText;
}
