
#include "Precomp.h"
#include "UConst.h"

void UConst::Load(ObjectStream* stream)
{
	UField::Load(stream);
	Constant = stream->ReadString();
}

void UConst::Save(PackageStreamWriter* stream)
{
	UField::Save(stream);
	stream->WriteString(Constant);
}
