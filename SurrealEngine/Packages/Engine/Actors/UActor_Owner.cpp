
#include "Precomp.h"
#include "UActor.h"
#include "VM/ScriptCall.h"

void UActor::SetOwner(UActor* newOwner)
{
	if (Owner())
	{
		CallEvent(Owner(), EventName::LostChild, { ExpressionValue::ObjectValue(this) });
		Owner()->RemoveChildActor(this);
	}

	Owner() = newOwner;

	if (Owner())
	{
		CallEvent(Owner(), EventName::GainedChild, { ExpressionValue::ObjectValue(this) });
		Owner()->AddChildActor(this);
	}
}

void UActor::AddChildActor(UActor* actor)
{
	if (actor)
		ChildActors.push_back(actor);
}

void UActor::RemoveChildActor(UActor* actor)
{
	if (!actor)
		return;

	auto it = ChildActors.begin();

	while (it != ChildActors.end())
	{
		if (*it == actor)
		{
			ChildActors.erase(it);
			return;
		}
		it++;
	}
}

bool UActor::IsOwnedBy(UActor* owner)
{
	for (UActor* cur = this; cur; cur = cur->Owner())
	{
		if (cur == owner)
		{
			return true;
		}
	}
	return false;
}
