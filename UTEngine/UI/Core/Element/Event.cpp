
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
	return 0;
}

int Event::getDetail()
{
	return 0;
}

double Event::getClientX()
{
	return 0;
}

double Event::getClientY()
{
	return 0;
}
