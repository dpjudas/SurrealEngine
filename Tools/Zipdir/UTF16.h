#pragma once

#include <string>

std::wstring to_utf16(const std::string& str);
std::string from_utf16(const std::wstring& str);
