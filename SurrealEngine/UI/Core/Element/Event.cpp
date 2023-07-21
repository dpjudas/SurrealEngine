
#include "Precomp.h"
#include "Event.h"

Event::Event()
{
}

void Event::stopPropagation()
{
	stopPropagationFlag = true;
}

void Event::stopImmediatePropagation()
{
	stopImmediatePropagationFlag = true;
}

void Event::preventDefault()
{
	preventDefaultFlag = true;
}

int Event::getKeyCode()
{
	return keyCode;
}

int Event::getDetail()
{
	return detail;
}

int Event::getButton()
{
	return button;
}

double Event::getClientX()
{
	return clientX;
}

double Event::getClientY()
{
	return clientY;
}
