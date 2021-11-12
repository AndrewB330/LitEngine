#pragma once

#include <chrono>
#include <queue>

namespace lit::common {

    class Timer {
    public:
        Timer();

        void Reset();

        double GetTime() const;

        double GetTimeAndReset();

    private:
        #ifdef _MSC_VER
        std::chrono::steady_clock::time_point m_start_time;
        #else
        std::chrono::system_clock::time_point m_start_time;
        #endif
    };

    class FpsTimer {
    public:
        FpsTimer() = default;

        void FrameStart();

        void FrameEnd();

        double GetAverageFPS() const;

        double GetAverageMS() const;

    private:
        const int kMaxFramesCount = 60;

        bool m_frame_started = false;

        std::queue<double> m_times;

        Timer m_timer;
    };

}
