#pragma once

#define GLM_FORCE_SWIZZLE

#include <lit/application/window_renderer.hpp>
#include <lit/gl/cubemap.hpp>
#include "lit/voxels/voxel_world.hpp"
#include "controls/player_controller.hpp"
#include "debug_options.hpp"
#include "gpu_data_managers/sky_box_loader.hpp"
#include "game_rendering/voxels_pipeline.hpp"
#include <engine/physics/physics_engine.hpp>

namespace lit::voxels {

    using lit::application::WindowRenderer;

    class GameComponent : public WindowRenderer {
    public:
        explicit GameComponent(std::shared_ptr<lit::voxels::Camera> camera);

        ~GameComponent() override = default;

        bool Init() override;

        void Redraw() override;

    private:

        void InitPipeline();

        void InitWorld();

        glm::vec2 GetViewport() const;

        bool m_old_active = false;

        std::shared_ptr<gl::Context> m_ctx;

        std::shared_ptr<lit::voxels::Camera> m_camera;
        std::unique_ptr<lit::voxels::VoxelsPipeline> m_pipeline;
        std::unique_ptr<lit::voxels::VoxelWorld> m_world;

        std::unique_ptr<PhysicsEngine> m_physics;

        std::shared_ptr<const Collider> m_box_collider;
        std::shared_ptr<VoxelObject> m_box_object;
        std::shared_ptr<const Collider> m_box_collider2;
        std::shared_ptr<VoxelObject> m_box_object2;
    };

}
