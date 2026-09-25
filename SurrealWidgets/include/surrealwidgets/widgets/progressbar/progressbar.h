#pragma once

#include "../../core/widget.h"
#include "../../core/orientation.h"

enum class ProgressState
{
    Normal,
    Paused,
    Error
};

class ProgressBar : public Widget
{
public:
    ProgressBar(Widget* parent);
    ~ProgressBar();

    std::function<void(int)> ValueChanged;

    int GetMin() const { return m_minValue; }
    void SetMin(int newMin);

    int GetMax() const { return m_maxValue; }
    void SetMax(int newMax);

    int GetValue() const { return m_value; }
    void SetValue(int newValue);

    void ShowPercentageText(bool enable) { m_showPercText = enable; }

    Orientation GetOrientation() const { return m_Orientation; }
    void SetOrientation(Orientation newOrientation);

    ProgressState GetState() const { return m_progressState; }
    void SetProgressState(ProgressState newState);

    double GetPreferredWidth() override { return 20.0; }
    double GetPreferredHeight() override { return 20.0; }

protected:
    void OnPaint(Canvas* canvas) override;

private:
    int m_minValue = 0;
    int m_value = 5;
    int m_maxValue = 10;

    std::unique_ptr<PseudoWidget> m_knobWidget;
    std::unique_ptr<PseudoWidget> m_trackWidget;

    Orientation m_Orientation = Orientation::Horizontal;
    ProgressState m_progressState = ProgressState::Normal;

    bool m_orientationChanged = false;
    bool m_MousePressed = false;
    bool m_showPercText = true;
};
