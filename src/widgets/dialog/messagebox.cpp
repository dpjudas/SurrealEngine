#include <surrealwidgets/widgets/dialog/messagebox.h>

#include <surrealwidgets/widgets/pushbutton/pushbutton.h>
#include <surrealwidgets/widgets/textlabel/textlabel.h>
#include <surrealwidgets/widgets/imagebox/imagebox.h>
#include <surrealwidgets/core/layout.h>

#include "surrealwidgets/widgets/layout/hboxlayout.h"
#include "surrealwidgets/widgets/layout/vboxlayout.h"

MessageBox::MessageBox(Widget* owner, const std::string& windowTitle, const std::string& message, MessageBoxIcon icon, DialogButton buttons)
        : Dialog(owner), m_Icon(new ImageBox(this)), m_MessageLabel(new TextLabel(this)), m_DialogButtons({})
{
    SetWindowTitle(windowTitle);

    if (owner && !owner->GetWindowIcon().empty())
    {
        /* Inherit the window icon from the owner */
        SetWindowIcon(owner->GetWindowIcon());
    }

    switch (icon)
    {
    default:
    case MessageBoxIcon::None:
        break;
    case MessageBoxIcon::Information:
        m_Icon->SetImage(Image::LoadResource("InformationIcon.png"));
        break;
    case MessageBoxIcon::Question:
        m_Icon->SetImage(Image::LoadResource("QuestionIcon.png"));
        break;
    case MessageBoxIcon::Warning:
        m_Icon->SetImage(Image::LoadResource("WarningIcon.png"));
        break;
    case MessageBoxIcon::Error:
        m_Icon->SetImage(Image::LoadResource("ErrorIcon.png"));
        break;
    }

    if (icon != MessageBoxIcon::None)
    {
        m_Icon->SetFixedSize(48.0, 48.0);
        m_Icon->SetImageMode(ImageBoxMode::Contain);
    }

    m_MessageLabel->SetText(message);
    m_MessageLabel->SetStretching(true);

    if (AllFlags(buttons, DialogButton::Ok))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::Ok));
    if (AllFlags(buttons, DialogButton::Cancel))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::Cancel));
    if (AllFlags(buttons, DialogButton::Apply))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::Apply));
    if (AllFlags(buttons, DialogButton::Yes))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::Yes));
    if (AllFlags(buttons, DialogButton::YesToAll))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::YesToAll));
    if (AllFlags(buttons, DialogButton::No))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::No));
    if (AllFlags(buttons, DialogButton::NoToAll))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::NoToAll));
    if (AllFlags(buttons, DialogButton::Abort))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::Abort));
    if (AllFlags(buttons, DialogButton::Ignore))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::Ignore));
    if (AllFlags(buttons, DialogButton::Retry))
        m_DialogButtons.push_back(AddDialogButton(DialogButton::Retry));

    /* Set up layout */

    auto mainLayout = new VBoxLayout();

    auto iconLayout = new VBoxLayout();
    iconLayout->AddWidget(m_Icon);
    iconLayout->AddStretch();

    auto messageLayout = new HBoxLayout();
    messageLayout->SetGapWidth(10.0);
    messageLayout->AddLayout(iconLayout);
    messageLayout->AddWidget(m_MessageLabel);

    auto buttonsLayout = new HBoxLayout();
    buttonsLayout->AddStretch();
    for (const auto button : m_DialogButtons)
        buttonsLayout->AddWidget(button);
    buttonsLayout->AddStretch();

    mainLayout->AddLayout(messageLayout);
    mainLayout->AddStretch();
    mainLayout->AddLayout(buttonsLayout);

    SetLayout(mainLayout);
    SetNoncontentSizes(15, 10, 15, 10);

    double layoutWidth = std::min(GetPreferredWidth(), 700.0);
    double layoutHeight = GetPreferredHeight(layoutWidth);
    double frameWidth = layoutWidth + GetNoncontentLeft() + GetNoncontentRight();
    double frameHeight = layoutHeight + GetNoncontentTop() + GetNoncontentBottom();
    if (Widget* parentWindow = Parent() ? Parent()->Window() : nullptr)
    {
        // Center on parent
        Rect parentBox = parentWindow->GetFrameGeometry();
        double x = parentBox.x + (parentBox.width - frameWidth) * 0.5;
        double y = parentBox.y + (parentBox.height - frameHeight) * 0.5;
        SetFrameGeometry(Rect::xywh(x, y, frameWidth, frameHeight));
    }
    else
    {
        // Center the window on primary screen
        auto screenSize = DisplayWindow::GetScreenSize();
        double x = (screenSize.width - frameWidth) * 0.5;
        double y = (screenSize.height - frameHeight) * 0.5;
        SetFrameGeometry(Rect::xywh(x, y, frameWidth, frameHeight));
    }

    SetWindowResizable(false);
}

DialogButton MessageBox::Information(Widget* owner, const std::string& message, const std::string& windowTitle)
{
    auto msgBox = MessageBox(owner, windowTitle, message, MessageBoxIcon::Information, DialogButton::Ok);
    return static_cast<DialogButton>(msgBox.Exec());
}

DialogButton MessageBox::Question(Widget* owner, const std::string& message, const std::string& windowTitle)
{
    auto msgBox = MessageBox(owner, windowTitle, message, MessageBoxIcon::Question, DialogButton::Yes | DialogButton::No);
    return static_cast<DialogButton>(msgBox.Exec());
}

DialogButton MessageBox::Warning(Widget* owner, const std::string& message, const std::string& windowTitle)
{
    auto msgBox = MessageBox(owner, windowTitle, message, MessageBoxIcon::Warning, DialogButton::Ok);
    return static_cast<DialogButton>(msgBox.Exec());
}

DialogButton MessageBox::Error(Widget* owner, const std::string& message, const std::string& windowTitle)
{
    auto msgBox = MessageBox(owner, windowTitle, message, MessageBoxIcon::Error, DialogButton::Ok);
    return static_cast<DialogButton>(msgBox.Exec());
}