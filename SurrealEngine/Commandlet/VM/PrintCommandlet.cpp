
#include "Precomp.h"
#include "PrintCommandlet.h"
#include "DebuggerApp.h"
#include "VM/Frame.h"
#include "Engine.h"
#include "Packages/Core/UClass.h"
#include "Packages/Core/Properties/UClassProperty.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include <regex>

PrintCommandlet::PrintCommandlet()
{
	SetShortFormName("p");
	SetLongFormName("print");
	SetShortDescription("Print content of variable");
}

void PrintCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	if (args.empty())
	{
		for (UObject* actor : engine->Level->Actors)
		{
			if (actor)
			{
				PrintColumn(console, actor->Name.ToString(), actor->Class->Name.ToString(), true);
			}
		}
	}
	else if (args[0] == '/')
	{
		std::string searchstr;
		Array<std::string> chunks;
		size_t pos = args.find(' ');
		if (pos == std::string::npos)
		{
			searchstr = args.substr(1);
		}
		else
		{
			searchstr = args.substr(1, pos - 1);
			chunks = SplitString(args.substr(pos + 1), '.');
		}

		try
		{
			std::regex reg(searchstr, std::regex::ECMAScript | std::regex::icase);
			for (UObject* actor : engine->Level->Actors)
			{
				if (std::regex_search(actor->Name.ToString(), reg) || std::regex_search(actor->Class->Name.ToString(), reg))
				{
					if (chunks.empty())
					{
						PrintColumn(console, actor->Name.ToString(), actor->Class->Name.ToString(), true);
					}
					else
					{
						UObject* obj = actor;
						for (auto chunk = chunks.begin(); chunk != chunks.end(); chunk++)
						{
							bool bFoundObj = false;
							for (UProperty* prop : obj->PropertyData.Class->Properties)
							{
								if (prop->Name == (*chunk))
								{
									void* val = obj->PropertyData.Ptr(prop);

									if (chunk + 1 == chunks.end())
									{
										std::string name = prop->Name.ToString();
										std::string value = prop->PrintValue(val);
										PrintColumn(console, actor->Name.ToString(), actor->Class->Name.ToString(), false);
										PrintColumn(console, name, value, true);
										bFoundObj = true;
										break;
									}
									else if (UObject::TryCast<UObjectProperty>(prop) || UObject::TryCast<UClassProperty>(prop))
									{
										obj = *(UObject**)val;
										bFoundObj = true;
										break;
									}
								}
							}

							if (!bFoundObj)
								break;
						}
					}
				}
			}
		}
		catch (const std::exception& e)
		{
			console->WriteOutput(e.what() + NewLine());
		}
	}
	else
	{
		auto results = FindProperties(console, args);
		if (!results.empty())
		{
			for (size_t i = 0, count = results.size(); i < count; i++)
			{
				PrintColumn(console, results[i].first, results[i].second, (i % 2 == 1) || (i + 1 == count));
			}
		}
	}
}

std::vector<std::pair<std::string, std::string>> PrintCommandlet::FindProperties(DebuggerApp* console, const std::string& args)
{
	std::vector<std::pair<std::string, std::string>> results;

	Frame* frame = console->GetCurrentFrame();

	UObject* obj = nullptr;
	bool bFoundObj = false;

	Array<std::string> chunks = SplitString(args, '.');

	if (frame && chunks[0] == "this")
	{
		obj = frame->GetUObject();
	}
	else if (frame)
	{
		// Search local variables
		for (UProperty* prop : frame->GetFunc()->Properties)
		{
			if (prop->Name == chunks[0])
			{
				void* ptr = (uint8_t*)frame->GetVariables() + prop->DataOffset.DataOffset;
				if (UObject::TryCast<UObjectProperty>(prop) || UObject::TryCast<UClassProperty>(prop))
				{
					obj = *(UObject**)ptr;
					bFoundObj = true;
					break;
				}
				else
				{
					for (int i = 0; i < prop->ArrayDimension; i++)
					{
						std::string name = prop->Name.ToString();
						if (prop->ArrayDimension > 1)
						{
							name += '[';
							name += std::to_string(i);
							name += ']';
						}

						std::string value = prop->PrintValue(prop->GetElement(ptr, i));
						results.push_back({ name, value });
					}
					return results;
				}
			}
		}
	}

	if (!obj && frame)
	{
		for (UProperty* prop : frame->GetUObject()->PropertyData.Class->Properties)
		{
			if (prop->Name == chunks[0])
			{
				void* ptr = ((uint8_t*)frame->GetUObject()->PropertyData.Data) + prop->DataOffset.DataOffset;
				if (UObject::TryCast<UObjectProperty>(prop) || UObject::TryCast<UClassProperty>(prop))
				{
					obj = *(UObject**)ptr;
					bFoundObj = true;
					break;
				}
				else
				{
					for (int i = 0; i < prop->ArrayDimension; i++)
					{
						std::string name = prop->Name.ToString();
						if (prop->ArrayDimension > 1)
						{
							name += '[';
							name += std::to_string(i);
							name += ']';
						}

						std::string value = prop->PrintValue(prop->GetElement(ptr, i));
						results.push_back({ name, value });
					}
					return results;
				}
			}
		}
	}

	if (!obj)
	{
		// Search named level actors
		for (UObject* actor : engine->Level->Actors)
		{
			if (actor && actor->Name == chunks[0])
			{
				obj = actor;
				bFoundObj = true;
				break;
			}
		}
	}

	if (!obj)
	{
		if (!bFoundObj)
			console->WriteOutput("Unknown variable " + chunks[0] + ResetEscape() + NewLine());
		else
			console->WriteOutput("None" + ResetEscape() + NewLine());
		return results;
	}

	std::string name;
	std::string value;
	for (auto chunk = chunks.begin() + 1; chunk != chunks.end(); chunk++)
	{
		if (!obj)
		{
			results.push_back({ *chunk, "None" });
			return results;
		}

		bFoundObj = false;
		for (UProperty* prop : obj->PropertyData.Class->Properties)
		{
			if (prop->Name == (*chunk))
			{
				void* val = obj->PropertyData.Ptr(prop);

				if (chunk + 1 == chunks.end())
				{
					name = prop->Name.ToString();
					value = prop->PrintValue(val);
					results.push_back({ name, value });
					return results;
				}
				else if (UObject::TryCast<UObjectProperty>(prop) || UObject::TryCast<UClassProperty>(prop))
				{
					obj = *(UObject**)val;
					bFoundObj = true;
					break;
				}
			}
		}

		if (!bFoundObj)
		{
			console->WriteOutput("Unknown variable " + *chunk + ResetEscape() + NewLine());
			return results;
		}
	}

	if (!obj)
	{
		std::string name = *(chunks.end() - 1);
		results.push_back({ name, "None" });
		return results;
	}

	auto props = obj->PropertyData.Class->Properties;
	std::stable_sort(props.begin(), props.end(), [](UProperty* a, UProperty* b) { return a->Name.ToString() < b->Name.ToString(); });
	for (UProperty* prop : props)
	{
		void* ptr = obj->PropertyData.Ptr(prop);

		std::string name = prop->Name.ToString();
		std::string value = prop->PrintValue(ptr);
		results.push_back({ name, value });
	}
	return results;
}

void PrintCommandlet::PrintColumn(DebuggerApp* console, const std::string& name, const std::string& value, bool lastColumn)
{
	std::string space1, space2;
	space1.resize(std::max(32 - (int)name.size(), 1), ' ');
	if (!lastColumn)
		space2.resize(std::max(56 - (int)value.size(), 1), ' ');

	console->WriteOutput(name + ResetEscape() + space1 + ColorEscape(96) + value + space2 + ResetEscape());
	if (lastColumn)
		console->WriteOutput(NewLine());
}

void PrintCommandlet::OnPrintHelp(DebuggerApp* console)
{
	console->WriteOutput("Syntax: print                            - list all objects" + NewLine());
	console->WriteOutput("        print <object name> <property>   - print object property" + NewLine());
	console->WriteOutput("        print /<regex>                   - list objects matching regular expression" + NewLine());
	console->WriteOutput("        print /<regex> <property>        - print property in objects matching regular expression" + NewLine());
}
