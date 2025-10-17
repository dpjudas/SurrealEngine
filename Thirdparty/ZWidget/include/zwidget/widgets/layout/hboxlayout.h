#pragma once

#include "zwidget/core/layout.h"

class HBoxLayout final : public Layout
{
public:
    HBoxLayout(Widget* parent);

    void OnGeometryChanged() override;
};