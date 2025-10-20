#include "zwidget/widgets/layout/hboxlayout.h"

HBoxLayout::HBoxLayout(Widget* parent)
    : Layout(parent)
{
}

void HBoxLayout::OnGeometryChanged()
{
    // The idea is this:
    // Calculate all elements' preferred widths
    // Then subtract it from the parent widget's own width
    // Then distribute that remaining width evenly among all Stretches

    // Do nothing if there are no widgets added to layout
    if (m_Widgets.empty())
        return;

    const double totalWidth = GetWidth();

    double nonStretchWidgetsTotalWidth = 0.0;

    int stretchCount = 0;

    double stretchWidths = 0.0;

    for (const auto& widget : m_Widgets)
    {
        if (!widget->IsStretch())
        {
            nonStretchWidgetsTotalWidth += widget->GetNoncontentLeft() + widget->GetPreferredWidth() + widget->GetNoncontentRight();
        }
        else
            stretchCount++;
    }

    if (stretchCount > 0)
        stretchWidths = (totalWidth - nonStretchWidgetsTotalWidth) / stretchCount;

    double left = 0.0;

    for (const auto& widget: m_Widgets)
    {
        double frameWidth = 0.0;

        if (!widget->IsStretch())
            frameWidth = widget->GetNoncontentLeft() + widget->GetPreferredWidth() + widget->GetNoncontentRight();
        else
            frameWidth = stretchWidths;

        widget->SetFrameGeometry(Rect::xywh(left, 0, frameWidth, GetHeight()));
        left += widget->GetPreferredHeight();
    }
}

double HBoxLayout::GetPreferredWidth()
{
    double w = 0.0;
    for (const auto& widget : m_Widgets)
    {
        w += widget->GetNoncontentLeft() + widget->GetPreferredWidth() + widget->GetNoncontentRight();
    }
    return w;
}

double HBoxLayout::GetPreferredHeight()
{
    double h = 0.0;
    for (const auto& widget : m_Widgets)
    {
        h = std::max(h, widget->GetNoncontentTop() + widget->GetPreferredHeight() + widget->GetNoncontentBottom());
    }
    return h;
}
