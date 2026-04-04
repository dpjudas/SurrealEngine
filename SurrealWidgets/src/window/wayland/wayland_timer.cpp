#include "wayland_timer.h"

WaylandTimer::WaylandTimer()
{
}

WaylandTimer::WaylandTimer(Duration duration_ms) : m_timerDuration(duration_ms)
{
}

WaylandTimer::WaylandTimer(Duration duration_ms, CallbackFunc callback)
            : m_timerDuration(duration_ms), m_callback(callback)
{
}

WaylandTimer::~WaylandTimer()
{
    Stop();
}

void WaylandTimer::Start()
{
    m_startTime = Clock::now();
    m_currentTime = m_startTime;
    m_timerStarted = true;
    m_timerFinished = false;
}

void WaylandTimer::Stop()
{
    m_timerStarted = false;
}

void WaylandTimer::SetDuration(Duration duration_ms)
{
    if (m_timerStarted)
        return;
    m_timerDuration = duration_ms;
}

void WaylandTimer::SetCallback(std::function<void()> callback)
{
    if (m_timerStarted)
        return;
    m_callback = callback;
}

void WaylandTimer::SetRepeating(bool value)
{
    if (m_timerStarted)
        return;
    m_repeatingTimer = value;
}

void WaylandTimer::Update(Duration deltaTime)
{
    if (!m_timerStarted)
        return;

    m_currentTime += deltaTime;

    if (m_currentTime >= m_startTime + m_timerDuration)
    {
        m_callback();
        if (!m_repeatingTimer)
        {
            Stop();
            m_timerFinished = true;
        }
        else
            Start();
    }
}
