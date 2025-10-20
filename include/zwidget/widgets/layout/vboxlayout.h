#pragma once

#include "zwidget/core/layout.h"

class VBoxLayout final : public Layout
{
public:
    VBoxLayout(Widget* parent);

    void OnGeometryChanged() override;

    double GetPreferredWidth() override;
    double GetPreferredHeight() override;
};
