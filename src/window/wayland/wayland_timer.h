#pragma once

#include <chrono>
#include <functional>

class WaylandTimer
{
public:
    using Duration = std::chrono::duration<double, std::milli>;
    using Clock = std::chrono::system_clock;
    using TimePoint = std::chrono::time_point<Clock, Duration>;
    using CallbackFunc = std::function<void()>;

    WaylandTimer();
    WaylandTimer(Duration duration_ms);
    WaylandTimer(Duration duration_ms, CallbackFunc callback);

    ~WaylandTimer();

    void Start();
    void Stop();
    void SetDuration(Duration duration_ms);
    void SetCallback(CallbackFunc callback);
    void SetRepeating(bool value);
    void Update(Duration deltaTime);

    bool IsStarted() { return m_timerStarted; }
    bool IsFinished() { return m_timerFinished; }

private:
    bool m_timerStarted = false;
    bool m_repeatingTimer = false;
    bool m_timerFinished = false;

    TimePoint m_startTime;
    TimePoint m_currentTime;
    Duration m_timerDuration;
    CallbackFunc m_callback;
};
