#pragma once

#include <string>
#include <memory>

class View;
class WindowFrameImpl;

class WindowFrame
{
public:
	WindowFrame();
	~WindowFrame();

	virtual void onClose() = 0;

	void setIcon(const std::string& src);
	void setTitle(const std::string& title);
	void setSize(double width, double height);
	void show();
	void hide();

	void setContentView(std::unique_ptr<View> view);
	View* contentView();

	void setNeedsRender();

private:
	std::unique_ptr<WindowFrameImpl> impl;
};
