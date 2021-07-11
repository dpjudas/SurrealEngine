
#include "Precomp.h"
#include "DockHeader.h"
#include "DockHeaderButton.h"

DockHeader::DockHeader(View* p, std::function<void()> onClose) : HBoxView(p)
{
	addClass("dockheader");
	caption = new TextLabel(this);
	caption->addClass("dockheader-caption");
	closeButton = new DockHeaderButton(this, CloseIconSvg);
	closeButton->addClass("dockheader-closebutton");
	closeButton->element->addEventListener("click", [=](Event* event) { event->stopPropagation(); if (onClose) onClose(); });
	caption->setExpanding();
}

const std::string DockHeader::CloseIconSvg =
	"77u/PD94bWwgdmVyc2lvbj0nMS4wJyBlbmNvZGluZz0nVVRGLTgnPz4NCjxzdmcgeD0iMHB4IiB"
	"5PSIwcHgiIHZpZXdCb3g9IjAgMCAxMCAxMCIgdmVyc2lvbj0iMS4xIiB4bWxucz0iaHR0cDovL3"
	"d3dy53My5vcmcvMjAwMC9zdmciIHhtbG5zOnhsaW5rPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5L"
	"3hsaW5rIiB4bWw6c3BhY2U9InByZXNlcnZlIiB0YWc9IkVsZW1lbnQyIj4NCiAgPGcgaWQ9Iklu"
	"c2lkZV9Cb3JkZXIiIHRhZz0iRWxlbWVudCI+DQogICAgPHBhdGggZD0iTTYsIDVMOC44LCAyLjJ"
	"DOS4xLCAxLjkgOS4xLCAxLjUgOC44LCAxLjJDOC41LCAwLjkgOC4xLCAwLjkgNy44LCAxLjJMNS"
	"wgNEwyLjIsIDEuMkMxLjksIDAuOSAxLjUsIDAuOSAxLjIsIDEuMkMwLjksIDEuNSAwLjksIDEuO"
	"SAxLjIsIDIuMkw0LCA1TDEuMiwgNy44QzAuOSwgOC4xIDAuOSwgOC41IDEuMiwgOC44QzEuNSwg"
	"OS4xIDEuOSwgOS4xIDIuMiwgOC44TDUsIDZMNy44LCA4LjhDOC4xLCA5LjEgOC41LCA5LjEgOC4"
	"4LCA4LjhDOS4xLCA4LjUgOS4xLCA4LjEgOC44LCA3LjhMNiwgNXoiIGZpbGw9IiMwMDAwMDAiIG"
	"NsYXNzPSJNYXNrIiB0YWc9Ik1hc2siIC8+DQogIDwvZz4NCiAgPGcgaWQ9IkxheWVyXzIiIHRhZ"
	"z0iRWxlbWVudDEiIC8+DQo8L3N2Zz4=";
