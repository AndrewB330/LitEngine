#pragma once

namespace lit {

    struct DebugOptions {

        bool recompile_shaders = false;
        bool regenerate_tree = true;
        bool update_chunks = true;
        bool phase0 = true;
        bool phase1 = true;
        
        static DebugOptions & Instance() {
            static DebugOptions options;
            return options;
        }
    };

}
