
#include "Precomp.h"
#include "UPathNodeIterator.h"
#include "Utils/Logger.h"

void UPakPathNodeIterator::BuildPath(vec3& start, vec3& end)
{
	LogUnimplemented("PathNodeIterator.BuildPath()");
	NodeIndex() = 0;
}

void UPakPathNodeIterator::CheckUPak()
{
	// What does this even check?
}

UNavigationPoint* UPakPathNodeIterator::GetFirst()
{
	LogUnimplemented("PathNodeIterator.GetFirst()");
	//return NodePath().front();
	return nullptr;
}

UNavigationPoint* UPakPathNodeIterator::GetPrevious()
{
	LogUnimplemented("PathNodeIterator.GetPrevious()");
	// if (NodeIndex() > 0)
	// 	NodeIndex()--;
	// return NodePath()[NodeIndex()];
	return nullptr;
}

UNavigationPoint* UPakPathNodeIterator::GetCurrent()
{
	LogUnimplemented("PathNodeIterator.GetCurrent()");
	//return NodePath()[NodeIndex()];
	return nullptr;
}

UNavigationPoint* UPakPathNodeIterator::GetNext()
{
	LogUnimplemented("PathNodeIterator.GetNext()");
	// if (NodeIndex() < NodeCount() - 1)
	// 	NodeIndex()++;
	// return NodePath()[NodeIndex()];
	return nullptr;
}

UNavigationPoint* UPakPathNodeIterator::GetLast()
{
	LogUnimplemented("PathNodeIterator.GetLast()");
	//return NodePath().back();
	return nullptr;
}

UNavigationPoint* UPakPathNodeIterator::GetLastVisible()
{
	LogUnimplemented("PathNodeIterator.GetLastVisible()");
	return nullptr;
}
