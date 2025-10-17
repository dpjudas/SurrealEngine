#include "zwidget/core/layout.h"

Stretch::Stretch(Layout* parent)
    : Widget(parent)
{

}

bool Stretch::IsStretch() const
{
    return true;
}

Layout::Layout(Widget* parent)
    : Widget(parent)
{
}

void Layout::AddWidget(Widget* widget)
{
    m_Widgets.push_back(widget);
    widget->SetParent(this);
}

void Layout::AddStretch()
{
    m_Widgets.push_back(new Stretch(this));
}


