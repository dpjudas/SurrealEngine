
#include "Precomp.h"
#include "NStatLogFile.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NStatLogFile::RegisterFunctions()
{
	RegisterVMNativeFunc_0("StatLogFile", "CloseLog", &NStatLogFile::CloseLog, 0);
	RegisterVMNativeFunc_0("StatLogFile", "FileFlush", &NStatLogFile::FileFlush, 0);
	RegisterVMNativeFunc_1("StatLogFile", "FileLog", &NStatLogFile::FileLog, 0);
	RegisterVMNativeFunc_1("StatLogFile", "GetChecksum", &NStatLogFile::GetChecksum, 0);
	RegisterVMNativeFunc_0("StatLogFile", "OpenLog", &NStatLogFile::OpenLog, 0);
	RegisterVMNativeFunc_1("StatLogFile", "Watermark", &NStatLogFile::Watermark, 0);
}

void NStatLogFile::CloseLog(UObject* Self)
{
	engine->Log.push_back("Warning: StatLogFile.CloseLog not implemented");
}

void NStatLogFile::FileFlush(UObject* Self)
{
	engine->Log.push_back("Warning: StatLogFile.FileFlush not implemented");
}

void NStatLogFile::FileLog(UObject* Self, const std::string& EventString)
{
	engine->Log.push_back("Warning: StatLogFile.FileLog tried to log: " + EventString);
}

void NStatLogFile::GetChecksum(UObject* Self, std::string& Checksum)
{
	Checksum = "GetChecksum dummy value";
	engine->Log.push_back("Warning: StatLogFile.GetChecksum not implemented");
}

void NStatLogFile::OpenLog(UObject* Self)
{
	engine->Log.push_back("Warning: StatLogFile.OpenLog not implemented");
}

void NStatLogFile::Watermark(UObject* Self, const std::string& EventString)
{
	engine->Log.push_back("Warning: StatLogFile.Watermark not implemented");
}
