#pragma once

#include "zwidget/core/layout.h"

class HBoxLayout final : public Layout
{
public:
    HBoxLayout(Widget* parent);

    void OnGeometryChanged() override;

    double GetPreferredWidth() override;
    double GetPreferredHeight() override;
};