
#include "Precomp.h"
#include "DisassemblyPage.h"
#include "UObject/UClass.h"
#include "UObject/UClient.h"
#include "UI/Controls/ListView/ListView.h"
#include "ExpressionItemBuilder.h"
#include "VM/Bytecode.h"
#include "VM/ScriptCall.h"
#include "Engine.h"

DisassemblyPage::DisassemblyPage(View* parent) : VBoxView(parent)
{
	listview = new ListView(this);
	listview->setExpanding();

	listview->addColumn("Expression", 500);
	listview->addColumn("Name", 150);
	listview->addColumn("Value", 200, true);

	addClass("disassemblypage");
	listview->addClass("disassemblypage-listview");

	setFunction(FindEventFunction(engine->console, "Tick"));
}

void DisassemblyPage::setFunction(UFunction* func)
{
	listview->clearList();

	if (func)
	{
		int index = 0;
		for (Expression* expr : func->Code->Statements)
		{
			listview->rootItem()->add(ExpressionItemBuilder::createItem("Statement[" + std::to_string(index) + "]", expr));
			index++;
		}
	}
}
