
#include "Precomp.h"
#include "Iterator.h"
#include "Engine.h"
#include "UObject/ULevel.h"
#include "UObject/UActor.h"

AllObjectsIterator::AllObjectsIterator(UObject* BaseClass, UObject** ReturnValue, std::string MatchTag) : BaseClass(BaseClass), ReturnValue(ReturnValue), MatchTag(MatchTag)
{
	if (MatchTag == "None")
		MatchTag.clear();
}

bool AllObjectsIterator::Next()
{
	size_t size = engine->Level->Actors.size();
	while (index < size)
	{
		UActor* actor = engine->Level->Actors[index++];
		if (actor && actor->IsA(BaseClass->Name) && (MatchTag.empty() || actor->Tag() == MatchTag))
		{
			*ReturnValue = actor;
			return true;
		}
	}
	return false;
}
