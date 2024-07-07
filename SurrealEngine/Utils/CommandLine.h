#pragma once

class CommandLine
{
public:
	CommandLine(const Array<std::string>& args);

	bool HasArg(const std::string& shortform, const std::string& longform) const;
	std::string GetArg(const std::string& shortform, const std::string& longform, const std::string& defaultValue = {}) const;

	int GetArgInt(const std::string& shortform, const std::string& longform, int defaultValue = 0) const
	{
		if (HasArg(shortform, longform))
			return std::atoi(GetArg(shortform, longform).c_str());
		else
			return defaultValue;
	}

	float GetArgFloat(const std::string& shortform, const std::string& longform, float defaultValue = 0.0f) const
	{
		if (HasArg(shortform, longform))
			return (float)std::atof(GetArg(shortform, longform).c_str());
		else
			return defaultValue;
	}

	double GetArgDouble(const std::string& shortform, const std::string& longform, double defaultValue = 0.0) const
	{
		if (HasArg(shortform, longform))
			return std::atof(GetArg(shortform, longform).c_str());
		else
			return defaultValue;
	}

	const Array<std::string>& GetItems() const { return Items; }

private:
	std::map<std::string, std::string> Args;
	Array<std::string> Items;
};

extern CommandLine* commandline;
