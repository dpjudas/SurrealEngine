#include "SHA1Sum.h"
#include "TinySHA1/TinySHA1.hpp"

std::string SHA1Sum::of_file(const fs::path& filepath)
{
    // Check if the file exists
    if (File::try_open_existing(filepath.string()))
    {
        // Such file exists, let's try to take SHA1Sum of it
        auto bytes = File::read_all_bytes(filepath.string());

        sha1::SHA1 s;
        s.processBytes(bytes.data(), bytes.size());
        uint32_t digest[5];
        s.getDigest(digest);

        char temp[41];
        snprintf(temp, 41, "%08x%08x%08x%08x%08x", digest[0], digest[1], digest[2], digest[3], digest[4]);

        return std::string{temp};
    }

    return "";
}
