#pragma once

#include <lit/engine/systems/system.hpp>

namespace lit::engine {
    class DebugSystem : public BasicSystem {
    public:
        explicit DebugSystem(entt::registry &registry);

        ~DebugSystem() override = default;

        void Update(double dt) override;

    private:
    };
}