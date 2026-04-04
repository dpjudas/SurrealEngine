#include "surrealwidgets/widgets/dialog/dialog.h"
#include "surrealwidgets/widgets/textlabel/textlabel.h"
#include "surrealwidgets/widgets/pushbutton/pushbutton.h"
#include "surrealwidgets/widgets/imagebox/imagebox.h"

class AboutDialog : public Dialog
{
public:
    AboutDialog(Widget* parent);

private:
    PushButton* m_OkButton = nullptr;
    TextLabel* m_AboutTextLine1 = nullptr;
    TextLabel* m_AboutTextLine2 = nullptr;
    TextLabel* m_AboutTextLine3 = nullptr;
    ImageBox* m_SEBannerImageBox = nullptr;
    ImageBox* m_SEIconImageBox = nullptr;
};