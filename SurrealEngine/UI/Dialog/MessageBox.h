#pragma once

#include <string>
#include <functional>

class MessageBox
{
public:
	static void show(const std::string& title, const std::string& text, std::function<void()> okClicked = {}, const std::string& okLabel = "OK");
	static void question(const std::string& title, const std::string& text, std::function<void()> okClicked, std::function<void()> cancelClicked = {}, const std::string& okLabel = "OK", const std::string& cancelLabel = "Cancel");
};
