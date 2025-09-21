#pragma once

class NameString;
class UObject;

class PackageWriter
{
public:
	int GetVersion() const;
	int GetNameIndex(NameString name);
	int GetObjectReference(UObject* obj);
};
