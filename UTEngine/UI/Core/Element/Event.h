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
	int getButton();
	double getClientX();
	double getClientY();

private:
	int keyCode = 0;
	int detail = 0;
	int button = 0;
	double clientX = 0;
	double clientY = 0;
	bool stopPropagationFlag = false;
	bool stopImmediatePropagationFlag = false;
	bool preventDefaultFlag = false;
	friend class Element;
	friend class WindowFrameImpl;
};
