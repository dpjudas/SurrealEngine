#include "zwidget/widgets/layout/vboxlayout.h"

VBoxLayout::VBoxLayout(Widget* parent)
    : Layout(parent)
{
}

void VBoxLayout::OnGeometryChanged()
{
    // The idea is this:
    // Calculate all elements' preferred heights
    // Then subtract it from the parent widget's own height
    // Then distribute that remaining height evenly among all Stretches

    // Do nothing if there are no widgets added to layout
    if (m_Widgets.empty())
        return;

    const double totalHeight = GetHeight();

    double nonStretchWidgetsTotalHeight = 0.0;

    int stretchCount = 0;

    double stretchHeights = 0.0;

    for (const auto& widget : m_Widgets)
    {
        if (!widget->IsStretch())
        {
            nonStretchWidgetsTotalHeight += widget->GetNoncontentTop() + widget->GetPreferredHeight() + GetNoncontentBottom();
        }
        else
            stretchCount++;
    }

    if (stretchCount > 0)
        stretchHeights = (totalHeight - nonStretchWidgetsTotalHeight) / stretchCount;

    double top = 0.0;

    for (const auto& widget: m_Widgets)
    {
        double frameHeight = 0.0;
        if (!widget->IsStretch())
            frameHeight = widget->GetNoncontentTop() + widget->GetPreferredHeight() + GetNoncontentBottom();
        else
            frameHeight = stretchHeights;

        widget->SetFrameGeometry(Rect::xywh(0, top, GetWidth(), frameHeight));
        top += frameHeight;
    }
}