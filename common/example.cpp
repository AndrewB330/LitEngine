#include <lit/common/time_utils.hpp>
#include <lit/common/logging.hpp>
#include <lit/common/assert.hpp>
#include <bit>

using namespace lit::common;

void ExampleTime() {
    timer timer;

    double t = timer.get_elapsed_seconds();

    fps_timer fps;

    for(int i = 0; i < 9999; i++) {
        fps.frame_start();
        // frame rendering
        fps.frame_end();
    }

    double average_fps = fps.get_average_fps();
    double average_ms = fps.get_average_ms();
}

void ExampleLogging() {
    Logger::LogInfo("Some information %d", 5);
    Logger::LogWarning("Warning!!! 2+2!=%d", 4);
    Logger::LogError("Error!");
}

void ExampleAssert() {
    LIT_ASSERT(2 + 2 == 4, "2 + 2 should be 4")
    int x = 5, y = 7, z = x + y;
    LIT_ENSURE(x + y == z)
    LIT_ENSURE_EQ(x, 5)
    LIT_ENSURE_BETWEEN(x, 4, 7)
}

int main() {
    ExampleTime();
    ExampleLogging();
    ExampleAssert();
}