
#include "Precomp.h"
#include "UConAudioList.h"
#include "Packages/Engine/Resources/USound.h"

void UConAudioList::Load(ObjectStream* stream)
{
	UConObject::Load(stream);

	int count = stream->ReadIndex();
	for (int i = 0; i < count; i++)
	{
		conAudioList.push_back(stream->ReadObject<USound>());
	}
	stream->ThrowIfNotEnd();
}
