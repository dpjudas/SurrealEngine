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

    if (!m_ParentWidget)
        return;

    const double totalHeight = m_ParentWidget->GetHeight();

    double nonStretchWidgetsTotalHeight = 0.0;

    int stretchCount = 0;

    double stretchHeights = 0.0;

    for (const auto& widget : m_Widgets)
    {
        if (!widget->IsStretch())
        {
            nonStretchWidgetsTotalHeight += widget->GetNoncontentTop() + widget->GetPreferredHeight() + widget->GetNoncontentBottom();
            nonStretchWidgetsTotalHeight += m_gapHeight;
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
            frameHeight = widget->GetNoncontentTop() + widget->GetPreferredHeight() + widget->GetNoncontentBottom();
        else
            frameHeight = stretchHeights;

        widget->SetFrameGeometry(Rect::xywh(0, top, m_ParentWidget->GetWidth(), frameHeight));
        top += frameHeight + m_gapHeight;
    }
}

double VBoxLayout::GetPreferredWidth()
{
    double w = 0.0;
    for (const auto& widget : m_Widgets)
    {
        w = std::max(w, widget->GetNoncontentLeft() + widget->GetPreferredWidth() + widget->GetNoncontentRight());
    }
    return w;
}

double VBoxLayout::GetPreferredHeight()
{
    double h = 0.0;
    for (const auto& widget : m_Widgets)
    {
        h += widget->GetNoncontentTop() + widget->GetPreferredHeight() + widget->GetNoncontentBottom();
    }
    return h;
}

void VBoxLayout::SetGapHeight(const double newGapHeight)
{
    m_gapHeight = newGapHeight;
}
