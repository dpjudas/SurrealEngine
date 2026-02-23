
#include "Precomp.h"
#include "NDeusExDecoration.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "UObject/ULevel.h"
#include "Engine.h"
#include "Utils/StrCompare.h"

void NDeusExDecoration::RegisterFunctions()
{
	RegisterVMNativeFunc_0("DeusExDecoration", "ConBindEvents", &NDeusExDecoration::ConBindEvents, 2101);
}

void NDeusExDecoration::ConBindEvents(UObject* Self)
{
    /* Do not implement just yet!!
	for (UActor* actor : engine->Level->Actors)
	{
		if (actor)
		{
			NameString className = UObject::GetUClassName(actor);
			if (className == "DeusExLevelInfo")
				UActor* levelInfo = actor;
				break;
		}
		if (!levelInfo)
		{
			LogMessage("DeusExLevelInfo object missing!");
			return;
		}
	// TODO: Demangle this weird AI-made pseudocode
    if (levelInfo->missionNumber < 0)
        return;
    
    // TODO: Do we even need this ConversationPackage logic? I didn't see it in any map yet, but I haven't opened that many
    std::string* PackageName = (levelInfo->ConversationPackage != nullptr) ? LevelInfo->ConversationPackage : NameString("DeusExConversations");

    if (StrCompare::equals_ignore_case(PackageName, NameString("DeusExConversations")) == 0)
    {
        PackageName = NameString("DeusExCon");
    }

    char ConListName[64];
    sprintf(ConListName, NameString("ConList_Mission%02d"), levelInfo->missionNumber);

    char FullObjectName[256];
    sprintf(FullObjectName, NameString("%sText.%s"), PackageName, ConListName);

    DConversationList* ConList =
        (DConversationList*)UObject::StaticLoadObject(
            DConversationList::StaticClass(),
            nullptr,
            FullObjectName,
            nullptr,
            LOAD_NoFail,
            nullptr
        );

    if (ConList)
    {
        ConList->BindConversations(this);
    }
	}
    */
}
