
#include "Precomp.h"
#include "NameString.h"

Array<std::string> NameString::Names;
std::unordered_map<std::string, int> NameString::CompareStringToIndex;
std::unordered_map<std::string, std::pair<int, int>> NameString::SpellStringToIndex;
