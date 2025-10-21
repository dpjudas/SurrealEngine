#include "zwidget/core/layout.h"

Stretch::Stretch()
    : Widget(nullptr)
{
}

bool Stretch::IsStretch() const
{
    return true;
}

Layout::Layout(Widget* parent)
    : m_ParentWidget(parent)
{
    if (m_ParentWidget)
        SetParent(m_ParentWidget);
}

void Layout::AddWidget(Widget* widget)
{
    if (!widget)
        return;

    m_Widgets.push_back(widget);
    if (m_ParentWidget)
        widget->SetParent(m_ParentWidget);
}

void Layout::AddLayout(Layout* layout)
{
    if (!layout)
        return;

    const auto widget = new Widget();

    widget->SetLayout(layout);
    widget->SetParent(m_ParentWidget);

    m_Widgets.push_back(widget);
}

void Layout::AddStretch()
{
    m_Widgets.push_back(new Stretch());
}

Widget* Layout::Parent() const
{
    return m_ParentWidget;
}

void Layout::SetParent(Widget* parent)
{
    m_ParentWidget = parent;

    for (const auto widget : m_Widgets)
    {
        widget->SetParent(m_ParentWidget);
    }
}
