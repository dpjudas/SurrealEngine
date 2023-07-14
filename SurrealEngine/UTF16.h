#pragma once

#include <string>

std::string from_utf16(const std::wstring& str);
std::wstring to_utf16(const std::string& str);
