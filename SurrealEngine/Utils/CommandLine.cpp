
#include "Precomp.h"
#include "Utils/CommandLine.h"

CommandLine* commandline;

CommandLine::CommandLine(const Array<std::string>& args)
{
	for (const std::string& arg : args)
	{
		if (arg.size() > 2 && arg[0] == '-' && arg[1] == '-')
		{
			size_t pos = arg.find('=', 2);
			if (pos != std::string::npos)
			{
				Args[arg.substr(0, pos)] = arg.substr(pos + 1);
			}
			else
			{
				Args[arg] = {};
			}
		}
		else if (arg.size() > 1 && arg[0] == '-')
		{
			size_t pos = arg.find('=', 1);
			if (pos != std::string::npos)
			{
				Args[arg.substr(0, pos)] = arg.substr(pos + 1);
			}
			else
			{
				Args[arg] = {};
			}
		}
		else
		{
			Items.push_back(arg);
		}
	}
}

bool CommandLine::HasArg(const std::string& shortform, const std::string& longform) const
{
	return (!shortform.empty() && Args.find(shortform) != Args.end()) || (!longform.empty() && Args.find(longform) != Args.end());
}

std::string CommandLine::GetArg(const std::string& shortform, const std::string& longform, const std::string& defaultValue) const
{
	if (!shortform.empty())
	{
		auto it = Args.find(shortform);
		if (it != Args.end())
			return it->second;
	}

	if (!longform.empty())
	{
		auto it = Args.find(longform);
		if (it != Args.end())
			return it->second;
	}

	return defaultValue;
}
