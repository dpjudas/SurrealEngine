#include "AboutDialog.h"

#include "surrealwidgets/widgets/layout/hboxlayout.h"
#include "surrealwidgets/widgets/layout/vboxlayout.h"

AboutDialog::AboutDialog(Widget* parent)
    : Dialog(parent)
{
    SetWindowTitle("About Surreal Engine");
    m_AboutTextLine1 = new TextLabel( this );
    m_AboutTextLine1->SetText("Surreal Editor");

    m_AboutTextLine2 = new TextLabel( this );
    m_AboutTextLine2->SetText("Development Build");

    m_AboutTextLine3 = new TextLabel( this );
    m_AboutTextLine3->SetText("This entire thing is a work in progress and currently just a glorified level viewer.");

    m_SEBannerImageBox = new ImageBox( this );
    m_SEBannerImageBox->SetImage(Image::LoadResource("banner.png"));

    m_SEIconImageBox = new ImageBox( this );
    m_SEIconImageBox->SetImage(Image::LoadResource("surreal-editor-icon-64.png"));
    m_SEIconImageBox->SetNoncontentSizes(5, 5, 5, 5);

    m_OkButton = AddDialogButton(DialogButton::Ok);

    auto aboutTextLayout = new VBoxLayout();
    aboutTextLayout->AddWidget(m_AboutTextLine1);
    aboutTextLayout->AddWidget(m_AboutTextLine2);
    aboutTextLayout->AddWidget(m_AboutTextLine3);
    aboutTextLayout->AddStretch();

    auto bodyLayout = new HBoxLayout();
    bodyLayout->AddWidget(m_SEIconImageBox);
    bodyLayout->AddLayout(aboutTextLayout);
    bodyLayout->AddStretch();

    auto buttonLayout = new HBoxLayout();
    buttonLayout->AddStretch();
    buttonLayout->AddWidget(m_OkButton);
    buttonLayout->AddStretch();

    auto mainLayout = new VBoxLayout();

    mainLayout->AddWidget(m_SEBannerImageBox);
    mainLayout->AddLayout(bodyLayout);
    mainLayout->AddStretch();
    mainLayout->AddLayout(buttonLayout);

    // SetFixedSize(600, 600);

    SetLayout(mainLayout);

    SetNoncontentSizes(0, 0, 0, 10);
}
