#include "pipeline.hpp"
#include <lit/common/glm_ext/transform.hpp>

using lit::rendering::PipelineNode;

Pipeline::Pipeline(const std::shared_ptr<lit::gl::Context> &ctx) {
    shadow_draw = std::make_unique<PipelineNode<DrawShadowPrimitiveVert, DrawShadowPrimitiveFrag>>(ctx, false);
    draw = std::make_unique<PipelineNode<DrawPrimitiveVert, DrawPrimitiveFrag>>(ctx, true);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

mat4 GetLightView() {
    return glm::lookAt(glm::vec3(-4.0f, 10.0f, 3.0f),
                       glm::vec3( 0.0f, 0.0f,  0.0f),
                       glm::vec3( 0.0f, 1.0f,  0.0f));
}

mat4 GetLightProjection() {
    return glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.01f, 100.0f);
}

void Pipeline::Run(lit::voxels::Camera &camera, World &world) {
    auto[width, height] = camera.GetViewport();

    shadow_draw->Run(2048, 2048, [&] {
        glCullFace(GL_FRONT);
        auto &frag = shadow_draw->frag_shader;
        auto &vert = shadow_draw->vert_shader;

        vert.uni_projection = GetLightProjection();
        vert.uni_view = GetLightView();

        frag.uni_type = 0;

        for(auto & box : world.boxes) {
            vert.uni_size = box.size;
            vert.uni_model = lit::common::glm_ext::transform3(box.position, box.rotation, 1.0f).mat();
            frag.uni_model_view_inv = inverse(vert.uni_view * vert.uni_model);

            DrawCube();
        }

        frag.uni_type = 1;

        for(auto & particle : world.particles) {
            vert.uni_size = vec3(Particle::k_radius * 2.0f);
            vert.uni_model = lit::common::glm_ext::transform3(particle.position, quat(1, 0, 0, 0), 1.0f).mat();
            frag.uni_model_view_inv = inverse(vert.uni_view * vert.uni_model);
            frag.uni_radius = Particle::k_radius;

            DrawCube();
        }
    });

    draw->Run(width, height, [&] {
        glCullFace(GL_BACK);
        auto &frag = draw->frag_shader;
        auto &vert = draw->vert_shader;

        vert.uni_projection = camera.GetFrustumMat();
        vert.uni_view = camera.GetViewMat();

        frag.uni_type = 0;

        frag.uni_shadow_map.Bind(shadow_draw->frag_shader.gl_FragDepth_texture, 0);

        frag.uni_projection_light = GetLightProjection();
        frag.uni_view_light = GetLightView();

        for(auto & box : world.boxes) {
            vert.uni_size = box.size;
            vert.uni_model = lit::common::glm_ext::transform3(box.position, box.rotation, 1.0f).mat();
            frag.uni_model_view_inv = inverse(vert.uni_view * vert.uni_model);

            DrawCubeWithNormals();
        }

        frag.uni_type = 1;

        for(auto & particle : world.particles) {
            vert.uni_size = vec3(Particle::k_radius * 2.0f);
            vert.uni_model = lit::common::glm_ext::transform3(particle.position, quat(1, 0, 0, 0), 1.0f).mat();
            frag.uni_model_view_inv = inverse(vert.uni_view * vert.uni_model);
            frag.uni_radius = Particle::k_radius;

            DrawCubeWithNormals();
        }
    });
}
