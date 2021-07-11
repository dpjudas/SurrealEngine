#pragma once

class Event
{
public:
	Event();

	void stopPropagation();
	void stopImmediatePropagation();
	void preventDefault();

	int getKeyCode();
	int getDetail();
	double getClientX();
	double getClientY();

private:
	bool stopPropagationFlag = false;
	bool stopImmediatePropagationFlag = false;
	bool preventDefaultFlag = false;
	friend class Element;
};
