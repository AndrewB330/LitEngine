#include <lit/common/time_utils.hpp>
#include <bit>

using namespace lit::common;

void ExampleTime() {
    Timer timer;

    double t = timer.GetTime();

    FpsTimer fps;

    for(int i = 0; i < 9999; i++) {
        fps.FrameStart();
        // frame rendering
        fps.FrameEnd();
    }

    double average_fps = fps.GetAverageFPS();
    double average_ms = fps.GetAverageMS();
}

int main() {
    ExampleTime();
}