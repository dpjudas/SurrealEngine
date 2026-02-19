#pragma once

#include "UObject/UObject.h"

class NDeusExTextParser
{
public:
	static void RegisterFunctions();

	static void CloseText(UObject* Self);
	static void GetColor(UObject* Self, Color& ReturnValue);
	static void GetEmailInfo(UObject* Self, std::string& emailName, std::string& emailSubject, std::string& emailFrom, std::string& emailTo, std::string& emailCC);
	static void GetFileInfo(UObject* Self, std::string& FileName, std::string& fileDescription);
	static void GetName(UObject* Self, NameString& ReturnValue);
	static void GetTag(UObject* Self, uint8_t& ReturnValue);
	static void GetText(UObject* Self, std::string& ReturnValue);
	static void GotoLabel(UObject* Self, const std::string& Label);
	static void IsEOF(UObject* Self, BitfieldBool& ReturnValue);
	static void OpenText(UObject* Self, const NameString& textName, std::string* TextPackage, BitfieldBool& ReturnValue);
	static void ProcessText(UObject* Self, BitfieldBool& ReturnValue);
	static void SetPlayerName(UObject* Self, const std::string& newPlayerName);
};
