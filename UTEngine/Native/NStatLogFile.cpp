
#include "Precomp.h"
#include "NStatLogFile.h"
#include "VM/NativeFunc.h"

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
	throw std::runtime_error("StatLogFile.CloseLog not implemented");
}

void NStatLogFile::FileFlush(UObject* Self)
{
	throw std::runtime_error("StatLogFile.FileFlush not implemented");
}

void NStatLogFile::FileLog(UObject* Self, const std::string& EventString)
{
	throw std::runtime_error("StatLogFile.FileLog not implemented");
}

void NStatLogFile::GetChecksum(UObject* Self, std::string& Checksum)
{
	throw std::runtime_error("StatLogFile.GetChecksum not implemented");
}

void NStatLogFile::OpenLog(UObject* Self)
{
	throw std::runtime_error("StatLogFile.OpenLog not implemented");
}

void NStatLogFile::Watermark(UObject* Self, const std::string& EventString)
{
	throw std::runtime_error("StatLogFile.Watermark not implemented");
}
