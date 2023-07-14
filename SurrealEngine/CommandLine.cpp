
#include "Precomp.h"
#include "CommandLine.h"

CommandLine* commandline;

CommandLine::CommandLine(const std::vector<std::string>& args)
{
	for (const std::string& arg : args)
	{
		if (arg.size() > 1 && arg[0] == '-')
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
		else if (arg.size() > 2 && arg[0] == '-' && arg[1] == '-')
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
		else
		{
			Items.push_back(arg);
		}
	}
}

bool CommandLine::HasArg(const std::string& shortform, const std::string& longform) const
{
	return Args.find(shortform) != Args.end() || Args.find(longform) != Args.end();
}

std::string CommandLine::GetArg(const std::string& shortform, const std::string& longform, const std::string& defaultValue) const
{
	auto it = Args.find(shortform);

	if (it != Args.end())
	{
		return it->second;
	}

	it = Args.find(longform);
	if (it != Args.end())
	{
		return it->second;
	}

	return defaultValue;
}
