#pragma once

#include "../../core/widget.h"
#include "../../core/timer.h"
#include "../../core/orientation.h"

enum TickPosition : uint8_t
{
    NoTicks,
    TicksAbove,
    TicksBelow,
    TicksBothSides,
    TicksLeft = TicksAbove,
    TicksRight = TicksBelow
};

class Slider : public Widget
{
public:
    Slider(Widget* parent);
    ~Slider();

    std::function<void(int)> ValueChanged;

    int GetMin() const { return m_minValue; }
    void SetMin(int newMin);

    int GetMax() const { return m_maxValue; }
    void SetMax(int newMax);

    int GetValue() const { return m_value; }
    void SetValue(int newValue);

    TickPosition GetTickPosition() const { return m_TickPosition; }
    void SetTickPosition(TickPosition newPos);

    Orientation GetOrientation() const { return m_Orientation; }
    void SetOrientation(Orientation newOrientation);

    double GetPreferredWidth() override { return 20.0; }
    double GetPreferredHeight() override { return 20.0; }

protected:
    void OnGeometryChanged() override;
    bool OnMouseDown(const Point& pos, InputKey key) override;
    bool OnMouseUp(const Point& pos, InputKey key) override;
    void OnMouseMove(const Point& pos) override;
    void OnMouseLeave() override;
    void OnPaint(Canvas* canvas) override;

private:
    int m_minValue = 0;
    int m_value = 5;
    int m_knobValue = 5; // The value the knob shows. It might not always be equal to set value (i.e. when moving the knob around)
    int m_maxValue = 10;

    double m_lineSpacing = 1.0; // The amount of pixels the mouse needs to move in a relevant direction to knob to move a single tick.
    Point m_mouseDragStartPos;

    Rect m_knobRect;
    std::unique_ptr<PseudoWidget> m_knobWidget;
    std::unique_ptr<PseudoWidget> m_trackWidget;

    Orientation m_Orientation = Orientation::Horizontal;

    bool m_orientationChanged = false;
    bool m_MousePressed = false;

    Timer* m_MouseDownTimer;

    TickPosition m_TickPosition = NoTicks;

    void UpdatePartPositions();
    void SetKnobValue(int newValue);
    void MoveKnobSingleTick(const Point& pos);
    void OnTimerExpired();
};
