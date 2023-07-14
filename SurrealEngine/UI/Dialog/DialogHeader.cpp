
#include "Precomp.h"
#include "DialogHeader.h"
#include "DialogHeaderButton.h"
#include "Dialog.h"

DialogHeader::DialogHeader(View* p) : HBoxView(p)
{
	addClass("dialogheader");
	caption = new TextLabel(this);
	caption->addClass("dialogheader-caption");
	closeButton = new DialogHeaderButton(this, CloseIconSvg);
	closeButton->addClass("dialogheader-closebutton");
	closeButton->element->addEventListener("click", [this](Event* event) { static_cast<Dialog*>(parent())->onClose(event); });
	caption->setExpanding();
}

const std::string DialogHeader::CloseIconSvg = "M6, 5L8.8, 2.2C9.1, 1.9 9.1, 1.5 8.8, 1.2C8.5, 0.9 8.1, 0.9 7.8, 1.2L5, 4L2.2, 1.2C1.9, 0.9 1.5, 0.9 1.2, 1.2C0.9, 1.5 0.9, 1.9 1.2, 2.2L4, 5L1.2, 7.8C0.9, 8.1 0.9, 8.5 1.2, 8.8C1.5, 9.1 1.9, 9.1 2.2, 8.8L5, 6L7.8, 8.8C8.1, 9.1 8.5, 9.1 8.8, 8.8C9.1, 8.5 9.1, 8.1 8.8, 7.8L6, 5z";
const std::string DialogHeader::MaximizedIconSvg = "M9, 1L3, 1L3, 3L1, 3L1, 9L7, 9L7, 7L9, 7L9, 1zM6, 8L2, 8L2, 4L6, 4L6, 8zM8, 6L7, 6L7, 3L4, 3L4, 2L8, 2L8, 6z";
const std::string DialogHeader::MaximizeIconSvg = "M1, 1L1, 9L9, 9L9, 1L1, 1zM8, 8L2, 8L2, 2L8, 2L8, 8z";
const std::string DialogHeader::MinimizeIconSvg = "M1, 5L9, 5L9, 6L1, 6z";
