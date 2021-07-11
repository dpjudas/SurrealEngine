
#include "Precomp.h"
#include "MessageBox.h"
#include "Dialog.h"
#include "DialogButton.h"
#include "UI/Controls/TextLabel/TextLabel.h"

class MessageBoxView : public Dialog
{
public:
	MessageBoxView()
	{
		addClass("messagebox");
		labelText = new TextLabel(centerView);
		buttonOK = buttonbar->addButton("OK", [=]() { onOK(); });
		buttonOK->element->setStyle("width", "80px");
	}

	void addCancelButton()
	{
		buttonCancel = buttonbar->addButton("Cancel", [=]() { onCancel(); });
		buttonCancel->element->setStyle("width", "80px");
	}

	void onOK()
	{
		auto cb = okCallback;
		closeModal();
		if (cb)
			cb();
	}

	void onCancel()
	{
		auto cb = cancelCallback;
		closeModal();
		if (cb)
			cb();
	}

	void onClose(Event* event) override
	{
		auto cb = cancelCallback;
		Dialog::onClose(event);
		if (cb)
			cb();
	}

	TextLabel* labelText = nullptr;
	DialogButton* buttonOK = nullptr;
	DialogButton* buttonCancel = nullptr;
	std::function<void()> okCallback, cancelCallback;
};

void MessageBox::show(const std::string& title, const std::string& text, std::function<void()> okClicked, const std::string& okLabel)
{
	auto dialog = new MessageBoxView();
	dialog->setTitle(title);
	dialog->labelText->setText(text);
	dialog->buttonOK->setText(okLabel);
	dialog->okCallback = okClicked;
	dialog->cancelCallback = okClicked;
	dialog->showModal();
}

void MessageBox::question(const std::string& title, const std::string& text, std::function<void()> okClicked, std::function<void()> cancelClicked, const std::string& okLabel, const std::string& cancelLabel)
{
	auto dialog = new MessageBoxView();
	dialog->addCancelButton();
	dialog->setTitle(title);
	dialog->labelText->setText(text);
	dialog->buttonOK->setText(okLabel);
	dialog->buttonCancel->setText(cancelLabel);
	dialog->okCallback = okClicked;
	dialog->cancelCallback = cancelClicked;
	dialog->showModal();
}
