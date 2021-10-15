#pragma once

#include <cstdint>
#include <lit/gl/context.hpp>
#include "model.hpp"
#include <game_rendering/camera.hpp>
#include <lit/rendering/pipeline_node.hpp>
#include "shaders/draw_primitive.hpp"

using namespace shaders;

class Pipeline {
public:
    explicit Pipeline(const std::shared_ptr<lit::gl::Context> &ctx);

    void Run(lit::voxels::Camera &camera, World &world);

private:
    std::shared_ptr<lit::rendering::PipelineNode<DrawShadowPrimitiveVert, DrawShadowPrimitiveFrag>> shadow_draw;
    std::shared_ptr<lit::rendering::PipelineNode<DrawPrimitiveVert, DrawPrimitiveFrag>> draw;
};
