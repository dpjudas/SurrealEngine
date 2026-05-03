
#include "Precomp.h"
#include "UDXSaveInfo.h"
#include <ctime>

void UDXSaveInfo::UpdateTimeStamp()
{
    // Update the time fields
    std::time_t now = std::time(nullptr);
    std::tm* timedesc = std::localtime(&now);

    Year() = timedesc->tm_year;
    Month() = timedesc->tm_mon;
    Day() = timedesc->tm_mday;
    Hour() = timedesc->tm_hour;
    Minute() = timedesc->tm_min;
    Second() = timedesc->tm_sec;
}
