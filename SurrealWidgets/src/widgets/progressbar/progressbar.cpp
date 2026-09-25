#include "widgets/progressbar/progressbar.h"

ProgressBar::ProgressBar(Widget* parent)
    : Widget(parent)
{
    SetStyleClass("progressbar");
}

ProgressBar::~ProgressBar()
{
}

void ProgressBar::SetMax(int newMax)
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

    Update();
}

void ProgressBar::SetMin(int newMin)
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

    Update();
}

void ProgressBar::SetValue(int newValue)
{
    int oldValue = m_value;

    if (newValue < m_minValue)
        newValue = m_minValue;
    else if (newValue > m_maxValue)
        newValue = m_maxValue;

    m_value = newValue;

    if (oldValue != m_value && ValueChanged)
        ValueChanged(m_value);

    Update();
}

void ProgressBar::SetOrientation(Orientation newOrientation)
{
    m_orientationChanged = m_Orientation != newOrientation;
    m_Orientation = newOrientation;

    Update();
}

void ProgressBar::SetProgressState(ProgressState newState)
{
    m_progressState = newState;

    switch (m_progressState)
    {
        case ProgressState::Normal:
            SetStyleState("");
            break;
        case ProgressState::Paused:
            SetStyleState("paused");
            break;
        case ProgressState::Error:
            SetStyleState("error");
            break;
    }
}

void ProgressBar::OnPaint(Canvas* canvas)
{
    double w = GetWidth();
    double h = GetHeight();

    Colorf bgColor = GetStyleColor("background-color");
    Colorf progressColor = GetStyleColor("progress-color");
    Colorf textColor = GetStyleColor("color");

    Rect bgRect;
    Rect progressRect;

    // Draw the background
    auto bgImage = GetStyleImage("background-image");

    // Encompass the entire area
    bgRect = {0, 0, w, h};

    if (bgImage)
        canvas->drawImage(bgImage, bgRect);
    else
        canvas->fillRect(bgRect, bgColor);

    // Draw the progress bar
    auto progImage = GetStyleImage("progress-image");

    double percentage = static_cast<double>(m_value - m_minValue) / (m_maxValue - m_minValue);

    if (m_Orientation == Orientation::Horizontal)
        progressRect = {0, 0, w * percentage, h};
    else
    {
        // Draw the bar from bottom-up
        double yStart = h * (1 - percentage);
        progressRect = {0, yStart, w, h * percentage};
    }

    if (progImage)
        canvas->drawImage(progImage, progressRect);
    else
        canvas->fillRect(progressRect, progressColor);

    // Draw the percentage text
    if (m_showPercText)
    {
        auto font = GetFont();

        int percInt = static_cast<int>(std::floor(percentage * 100.0));

        std::string percText = std::to_string(percInt) + "%";

        auto metrics = canvas->getFontMetrics(font);

        canvas->drawText(font, Point((bgRect.width - canvas->measureText(GetFont(), percText).width) * 0.5, bgRect.y + (bgRect.height - metrics.height) / 2 + metrics.ascent), percText, textColor);
    }
}
