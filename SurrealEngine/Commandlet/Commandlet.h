#pragma once

class DebuggerApp;

class Commandlet
{
public:
	virtual ~Commandlet() = default;

	virtual void OnCommand(DebuggerApp* console, const std::string& args) = 0;
	virtual void OnPrintHelp(DebuggerApp* console) = 0;

	const std::string& GetShortFormName() const { return ShortFormName; }
	const std::string& GetLongFormName() const { return LongFormName; }
	const std::string& GetShortDescription() const { return ShortDescription; }

	static Array<std::string> SplitString(const std::string& text, char separator = ' ');

protected:
	void SetShortFormName(std::string name) { ShortFormName = std::move(name); }
	void SetLongFormName(std::string name) { LongFormName = std::move(name); }
	void SetShortDescription(std::string desc) { ShortDescription = std::move(desc); }

	static std::string ResetEscape();
	static std::string ColorEscape(int color);
	static std::string DeleteCharacterEscape(int count);
	static std::string EraseCharacterEscape(int count);
	static std::string CursorBackward(int count);
	static std::string NewLine();

private:
	std::string ShortFormName;
	std::string LongFormName;
	std::string ShortDescription;
};
