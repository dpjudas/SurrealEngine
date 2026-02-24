#include "UDXExtString.h"

#include "Utils/Logger.h"

void UDXExtString::AppendText(const std::string& newText)
{
    LogUnimplemented("Usage check: ExtString.AppendText");
    Text() += newText;
}

int UDXExtString::GetFirstTextPart(std::string& outText)
{
    LogUnimplemented("ExtString.GetFirstTextPart");
    outText = "";
    return 0;
}

int UDXExtString::GetNextTextPart(std::string& outText)
{
    LogUnimplemented("ExtString.GetNextTextPart");
    outText = "";
    return 0;
}

std::string& UDXExtString::GetText()
{
    LogUnimplemented("Usage check: ExtString.GetText");
    return Text();
}

int UDXExtString::GetTextLength()
{
    LogUnimplemented("Usage check: ExtString.GetTextLength");
    return (int)Text().length();
}

int UDXExtString::GetTextPart(int startPos, int count, std::string& outText)
{
    LogUnimplemented("Usage check: ExtString.GetTextPart");
    outText = Text().substr(startPos, count);
    return (int)outText.length();
}

void UDXExtString::SetText(const std::string& newText)
{
    LogUnimplemented("Usage check: ExtString.SetText");
    Text() = newText;
}
