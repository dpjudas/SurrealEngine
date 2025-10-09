#pragma once

#include "File.h"

class SHA1Sum
{
public:
    // Returns the SHA1Sum of a file given in filepath
    // or an empty string if checking the file fails for whatever reason.
    static std::string of_file(const fs::path& filepath);
};