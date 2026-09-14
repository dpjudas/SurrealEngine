#include "widgets/slider/slider.h"

Slider::Slider(Widget* parent)
    : Widget(parent)
{
    SetStyleClass("slider");

    m_MouseDownTimer = new Timer(this);
    m_MouseDownTimer->FuncExpired = [this] { OnTimerExpired(); };

    UpdatePartPositions();
}

Slider::~Slider()
{
}

void Slider::SetMax(int newMax)
{
    if (newMax <= m_minValue)
        m_maxValue = m_value;
    else
    {
        m_maxValue = newMax;
        // Clamp the current value if need be
        if (m_value > m_maxValue)
            SetValue(m_maxValue);
    }

    UpdatePartPositions();
}

void Slider::SetMin(int newMin)
{
    if (newMin >= m_maxValue)
        m_minValue = m_value;
    else
    {
        m_minValue = newMin;
        // Clamp the current value if need be
        if (m_value < m_minValue)
            SetValue(m_minValue);
    }

    UpdatePartPositions();
}

void Slider::SetValue(int newValue)
{
    int oldValue = m_value;

    if (newValue < m_minValue)
        newValue = m_minValue;
    else if (newValue > m_maxValue)
        newValue = m_maxValue;

    m_value = newValue;

    if (oldValue != m_value && ValueChanged)
        ValueChanged(m_value);

    SetKnobValue(newValue);
}

void Slider::SetTickPosition(TickPosition newPos)
{
    m_TickPosition = newPos;
    UpdatePartPositions();
}

void Slider::SetKnobValue(int newValue)
{
    if (newValue < m_minValue)
        newValue = m_minValue;
    else if (newValue > m_maxValue)
        newValue = m_maxValue;

    m_knobValue = newValue;

    UpdatePartPositions();
}

void Slider::SetOrientation(Orientation newOrientation)
{
    m_orientationChanged = m_Orientation != newOrientation;
    m_Orientation = newOrientation;
    UpdatePartPositions();
}

bool Slider::OnMouseDown(const Point& pos, InputKey key)
{
    m_mouseDragStartPos = pos;

    if (key == InputKey::LeftMouse)
    {
        if (m_knobRect.contains(pos))
            m_MousePressed = true;
        else
        {
            m_MouseDownTimer->Start(500, false);
            MoveKnobSingleTick(pos);
        }

        SetPointerCapture();
    }

    return true;
}

bool Slider::OnMouseUp(const Point& pos, InputKey key)
{
    m_MousePressed = false;

    if (m_knobValue != m_value)
        SetValue(m_knobValue);

    m_MouseDownTimer->Stop();

    ReleasePointerCapture();
    return true;
}

void Slider::OnMouseMove(const Point& pos)
{
    if (!m_MousePressed)
    {
        if (m_knobRect.contains(pos) && m_knobWidget && m_knobWidget->GetStyleState().empty())
            m_knobWidget->SetStyleState("hover");
    }
    else
    {
        double posDiff;

        if (m_Orientation == Orientation::Horizontal)
            posDiff = (pos - m_mouseDragStartPos).x;
        else
            posDiff = (pos - m_mouseDragStartPos).y;

        int tickMoveCount = static_cast<int>(posDiff / m_lineSpacing);

        if (tickMoveCount != 0)
        {
            SetKnobValue(m_knobValue + tickMoveCount);
            m_mouseDragStartPos = pos;
        }

        UpdatePartPositions();
    }

    Update();
}

void Slider::OnMouseLeave()
{
    m_MousePressed = false;

    if (m_knobWidget && !m_knobWidget->GetStyleState().empty())
    {
        m_knobWidget->SetStyleState("");
    }
}

void Slider::OnPaint(Canvas* canvas)
{
    double w = GetWidth();
    double h = GetHeight();
    double kw = GetStyleDouble("knob-image-width");
    double kh = GetStyleDouble("knob-image-height");
    double tw = GetStyleDouble("track-image-width");
    double th = GetStyleDouble("track-image-height");
    Colorf trackColor = GetStyleColor("track-background-color");
    Colorf knobColor = GetStyleColor("knob-background-color");
    Colorf tickColor = GetStyleColor("tick-color");

    auto knobImage = GetStyleImage("knob-image");

    double track_height = 4.0;

    // Draw the track
    auto trackImage = GetStyleImage("track-image");

    Rect trackRect;

    if (trackImage)
    {
        if (tw == 0.0) tw = (double)trackImage->GetWidth();
        if (th == 0.0) th = (double)trackImage->GetHeight();

        if (m_Orientation == Orientation::Horizontal)
            trackRect = {0, (h - th) / 2, w, th};
        else
            trackRect = {(w - tw) / 2, 0, tw, h};

        canvas->drawImage(trackImage, trackRect);
    }
    else
    {
        if (tw == 0.0) tw = 4.f;
        if (th == 0.0) th = 4.f;

        if (m_Orientation == Orientation::Horizontal)
            trackRect = {0, (h - th) / 2, w, th};
        else
            trackRect = {(w - tw) / 2, 0, tw, h};

        canvas->fillRect(trackRect, trackColor);
    }

    // Calculate knob geometry
    if (knobImage)
    {
        if (kw == 0.0) kw = (double)knobImage->GetWidth();
        if (kh == 0.0) kh = (double)knobImage->GetHeight();
    }
    else
    {
        if (kw == 0.0) kw = 8.f;
        if (kh == 0.0) kh = h - 4.f;
    }

    // Draw ticks
    if (m_TickPosition != NoTicks)
    {
        auto numTicks = m_maxValue - m_minValue + 1;

        if (m_Orientation == Orientation::Horizontal)
        {
            double tickLength = kh / 4;

            for (int i = m_minValue ; i <= m_maxValue ; i++)
            {
                auto tickx = 0.5 * kw + (i - m_minValue) * (w - kw) / (m_maxValue - m_minValue);

                if (m_TickPosition & TicksAbove)
                    canvas->line({ tickx, (h - th) / 2 - tickLength }, { tickx, (h - th) / 2}, tickColor);

                if (m_TickPosition & TicksBelow)
                    canvas->line({tickx, (h + th) / 2 }, {tickx, (h + th) / 2 + tickLength }, tickColor);
            }
        }
        else
        {
            double tickLength = kw / 4;

            for (int i = m_minValue ; i <= m_maxValue ; i++)
            {
                auto ticky = 0.5 * kh + (i - m_minValue) * (h - kh) / (m_maxValue - m_minValue);

                if (m_TickPosition & TicksLeft)
                    canvas->line({ (w - tw) / 2 - tickLength, ticky - tickLength }, { (w - tw) / 2, ticky}, tickColor);

                if (m_TickPosition & TicksRight)
                    canvas->line({(w + tw) / 2, ticky }, {(w + tw) / 2 + tickLength, ticky }, tickColor);
            }
        }
    }

    if (knobImage)
        canvas->drawImage(knobImage, m_knobRect);
    else
        canvas->fillRect(m_knobRect, knobColor);
}

void Slider::OnGeometryChanged()
{
    UpdatePartPositions();
}

void Slider::UpdatePartPositions()
{
    if (!m_knobWidget || m_orientationChanged)
    {
        m_knobWidget = std::make_unique<PseudoWidget>(m_Orientation == Orientation::Horizontal ? "slider-knob-hor" : "slider-knob-ver");
        m_trackWidget = std::make_unique<PseudoWidget>(m_Orientation == Orientation::Horizontal ? "slider-track-hor" : "slider-track-ver");
        m_orientationChanged = false;
    }

    auto width = GetWidth();
    auto height = GetHeight();

    double knobWidth = GetStyleDouble("knob-image-width");
    double knobHeight = GetStyleDouble("knob-image-height");

    auto image = GetStyleImage("knob-image");

    if (image)
    {
        if (knobWidth == 0.0) knobWidth = (double)image->GetWidth();
        if (knobHeight == 0.0) knobHeight = (double)image->GetHeight();
    }
    else
    {
        if (knobWidth == 0.0) knobWidth = 8.f;
        if (knobHeight == 0.0) knobHeight = height - 4.f;
    }

    if (m_Orientation == Orientation::Horizontal)
    {
        double x = (m_minValue != m_maxValue) ? (m_knobValue - m_minValue) * (width - knobWidth) / (m_maxValue - m_minValue) : (width - knobWidth) / 2;
        m_lineSpacing = width / (m_maxValue - m_minValue);
        m_knobRect = {x, (height - knobHeight) / 2, knobWidth, knobHeight};
    }
    else
    {
        // Vertical
        double y = (m_minValue != m_maxValue) ? (m_knobValue - m_minValue) * (height - knobHeight) / (m_maxValue - m_minValue) : (height - knobHeight) / 2;
        m_lineSpacing = height / (m_maxValue - m_minValue);
        m_knobRect = {(width - knobWidth) / 2, y, knobWidth, knobHeight};
    }
}

void Slider::MoveKnobSingleTick(const Point& pos)
{
    // Move the knob in a single tick from a calculated direction
    double posDiff;

    if (m_Orientation == Orientation::Horizontal)
        posDiff = pos.x - m_knobRect.x + m_knobRect.width / 2;
    else
        posDiff = pos.y - m_knobRect.y + m_knobRect.height / 2;

    int tickMoveCount = static_cast<int>(std::round(posDiff / m_lineSpacing));

    if (tickMoveCount > 0)
        SetKnobValue(m_knobValue + 1);
    else if (tickMoveCount < 0)
        SetKnobValue(m_knobValue - 1);

    Update();
}

void Slider::OnTimerExpired()
{
    m_MouseDownTimer->Start(100, false);
    MoveKnobSingleTick(m_mouseDragStartPos);
}
