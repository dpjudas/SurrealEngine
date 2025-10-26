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

    if (!m_ParentWidget)
        return;

    const double totalWidth = m_ParentWidget->GetWidth();

    double nonStretchingWidgetsTotalWidth = 0.0;

    int stretchingWidgetsCount = 0;

    double stretchWidths = 0.0;

    for (const auto& widget : m_Widgets)
    {
        if (!widget->GetStretching())
        {
            nonStretchingWidgetsTotalWidth += widget->GetNoncontentLeft() + widget->GetPreferredWidth() + widget->GetNoncontentRight();
            nonStretchingWidgetsTotalWidth += m_gapWidth;
        }
        else
            stretchingWidgetsCount++;
    }

    if (stretchingWidgetsCount > 0)
        stretchWidths = (totalWidth - nonStretchingWidgetsTotalWidth) / stretchingWidgetsCount;

    double left = 0.0;

    for (const auto& widget: m_Widgets)
    {
        double frameWidth = 0.0;

        if (!widget->GetStretching())
            frameWidth = widget->GetNoncontentLeft() + widget->GetPreferredWidth() + widget->GetNoncontentRight();
        else
            frameWidth = stretchWidths;

        widget->SetFrameGeometry(Rect::xywh(left, 0, frameWidth, m_ParentWidget->GetHeight()));
        left += frameWidth + m_gapWidth;
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

void HBoxLayout::SetGapWidth(const double newGapWidth)
{
    m_gapWidth = newGapWidth;
}
