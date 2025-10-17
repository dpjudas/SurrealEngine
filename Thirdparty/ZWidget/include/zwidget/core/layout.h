#pragma once

#include "widget.h"
#include <vector>

class Layout;

// A widget solely used to take the rest of the available space.
// Only Layout classes can create them.
class Stretch : public Widget
{
public:
    bool IsStretch() const override;
private:
    Stretch(Layout* parent);
    friend class Layout;
};

// The Base Layout class. Shouldn't be used by itself.
// Use its subclasses HBoxLayout and VBoxLayout instead
class Layout : public Widget
{
public:
    Layout(Widget* parent);
    void AddWidget(Widget* widget);
    void AddStretch();

    void OnGeometryChanged() override {} // Override AGAIN in subclasses

protected:
    std::vector<Widget*> m_Widgets;
};

