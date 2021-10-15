#pragma once

namespace lit::voxels {

    enum class DrawBufferOption {
        Final,
        TAAOutput,
        Color,
        Depth,
        Normal,
        Lighting,
        DenoisedLighting,
        Velocity,
        Noise
    };

    enum class SkyBoxOption {
        Standard,
        Sunset,
        Pink,
        DeepDusk,
        Space
    };

    struct DebugOptions {
        DrawBufferOption draw_buffer_option = DrawBufferOption::Final;
        SkyBoxOption sky_box_option = SkyBoxOption::Pink;

        int draw_channel = 0; // 0 - all, 1 - red, 2 - green, 3 - blue
        int zoom = 1; // zoom level >=1
        bool jitter = false;

        float magic = 0.3f;
        float gamma = 0.5f;
        
        static DebugOptions & Instance() {
            static DebugOptions options;
            return options;
        }
    };

}
