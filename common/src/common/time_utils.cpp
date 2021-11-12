#include <lit/common/time_utils.hpp>
#include <iostream>

using namespace lit::common;

typedef std::chrono::duration<double, std::ratio<1> > second_;

Timer::Timer() {
    m_start_time = std::chrono::high_resolution_clock::now();
}

double Timer::GetTime() const {
    auto cur_time = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<second_>(cur_time - m_start_time).count();
    return delta;
}

void Timer::Reset() {
    m_start_time = std::chrono::high_resolution_clock::now();
}

double Timer::GetTimeAndReset() {
    double res = GetTime();
    Reset();
    return res;
}

double FpsTimer::GetAverageFPS() const {
    if (m_times.empty()) {
        return 0;
    }
    return m_times.size() / (m_times.back() - m_times.front());
}

double FpsTimer::GetAverageMS() const {
    if (m_times.empty()) {
        return 0;
    }
    return (m_times.back() - m_times.front()) / m_times.size() * 1000.0;
}

void FpsTimer::FrameStart() {
    if (m_frame_started) {
        FrameEnd();
    }
    m_frame_started = true;
    m_timer = Timer();
}

void FpsTimer::FrameEnd() {
    double time = m_timer.GetTime();
    while(m_times.size() > kMaxFramesCount) {
        m_times.pop();
    }
    m_times.push((m_times.empty() ? 0 : m_times.back()) + time);
    m_frame_started = false;
}
