
#include "Precomp.h"
#include "UAnyProperty.h"
#include "Utils/Logger.h"

void UAnyProperty::Load(ObjectStream* stream)
{
	UProperty::Load(stream);
	LogUnimplemented("AnyProperty.Load");
}

void UAnyProperty::Save(PackageStreamWriter* stream)
{
	UProperty::Save(stream);
	LogUnimplemented("AnyProperty.Save");
}
