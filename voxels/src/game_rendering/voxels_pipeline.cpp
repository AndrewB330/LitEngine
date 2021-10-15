#include "voxels_pipeline.hpp"
#include <lit/rendering/pipeline_node.hpp>

#include "shaders/ray_tracing.hpp"
#include "shaders/lighting.hpp"
#include "shaders/denoising.hpp"
#include "shaders/taa.hpp"
#include "shaders/copy.hpp"
#include "shaders/reflections.hpp"
#include "shaders/light_mixer.hpp"
#include "shaders/final.hpp"
#include "shaders/debug.hpp"
#include "../debug_options.hpp"
#include "../gpu_data_managers/voxel_data_manager.hpp"
#include "../gpu_data_managers/sky_box_loader.hpp"

using namespace lit::voxels::shaders;
using namespace lit::voxels;
using namespace lit;

void SetVoxelObjectUniform(RayTracingFrag::VoxelObjectData &uniform, const std::shared_ptr<VoxelObject> &object,
                           int texture_index) {
    uniform.dims = object->GetDims();
    uniform.lodSampler.Bind(VoxelDataManager::Instance().GetLodTexture(object->GetId()), texture_index);
    uniform.dataSampler.Bind(VoxelDataManager::Instance().GetDataTexture(object->GetId()), texture_index + 1);
    uniform.maxLod = object->GetHighestLod() - 1;
    uniform.transform = object->GetTransform().mat() * glm::translate(-object->GetCenter());
    uniform.transformInv = glm::translate(object->GetCenter()) * object->GetTransform().mat_inv();
    uniform.voxelSize = object->GetTransform().scale;
}

template<typename T>
void SetVoxelShadowUniform(T &uniform, const std::shared_ptr<VoxelObject> &object,
                           int texture_index) {
    uniform.dims = object->GetDims();
    uniform.lodSampler.Bind(VoxelDataManager::Instance().GetLodTexture(object->GetId()), texture_index);
    uniform.maxLod = object->GetHighestLod() - 1;
}

bool IsInside(const std::shared_ptr<VoxelObject> &object, const glm::vec3 &pos) {
    vec3 pos_obj = object->GetTransform().apply_inv(pos) + object->GetCenter();
    if (glm::any(glm::lessThan(pos_obj, vec3(-0.1f)))) {
        return false;
    }
    if (glm::any(glm::greaterThan(pos_obj, vec3(object->GetDims()) + 0.1f))) {
        return false;
    }
    return true;
}

const std::string assets_path = "../assets/";

struct VoxelsPipeline::VoxelsPipelineImpl {
    explicit VoxelsPipelineImpl(const std::shared_ptr<gl::Context> &ctx) {
        sky_box_loader.SetContext(ctx);
        ray_tracing = std::make_unique<rendering::PipelineNode<RayTracingVert, RayTracingFrag>>(ctx);
        lighting = std::make_unique<rendering::PipelineNode<LightingVert, LightingFrag>>(ctx);
        denoising = std::make_unique<rendering::PipelineNode<DenoisingVert, DenoisingFrag>>(ctx);
        taa = std::make_unique<rendering::PipelineNode<TAAVert, TAAFrag>>(ctx);
        copy = std::make_unique<rendering::PipelineNode<CopyVert, CopyFrag>>(ctx);
        reflections = std::make_unique<rendering::PipelineNode<ReflectionsVert, ReflectionsFrag>>(ctx);
        light_mixer = std::make_unique<rendering::PipelineNode<LightMixerVert, LightMixerFrag>>(ctx);
        final = std::make_unique<rendering::PipelineNode<FinalVert, FinalFrag>>(ctx);
        debug = std::make_unique<rendering::PipelineNode<DebugVert, DebugFrag>>(ctx, true);

        {
            // Init blue noise texture
            gl::Texture2DInfo blue_noise_info;
            auto blue_noise_image = common::read_png(assets_path + "blue_noise.png");
            blue_noise_info.width = (int) blue_noise_image.get_width();
            blue_noise_info.height = (int) blue_noise_image.get_height();
            blue_noise_info.data_format = gl::TextureDataFormat::RGBA;
            blue_noise_info.data_type = gl::TextureDataType::UnsignedByte;
            blue_noise_info.internal_format = gl::TextureInternalFormat::RGBA8;
            blue_noise_texture = std::make_shared<gl::Texture2D>(ctx, blue_noise_info);
            blue_noise_texture->Update(blue_noise_image);
        }

        {
            gl::Texture2DInfo palette_info;
            auto palette_image = common::read_png(assets_path + "palette.png");
            palette_info.width = (int) palette_image.get_width();
            palette_info.height = (int) palette_image.get_height();
            palette_info.data_format = gl::TextureDataFormat::RGBA;
            palette_info.data_type = gl::TextureDataType::UnsignedByte;
            palette_info.internal_format = gl::TextureInternalFormat::RGBA8;
            palette = std::make_shared<gl::Texture2D>(ctx, palette_info);
            palette->Update(palette_image);
        }
    }

    void Run(Camera &camera, VoxelWorld &world) {
        auto[width, height] = camera.GetViewport();
        SkyBox sky_box = GetSkyBox();

        ray_tracing->Run(width, height, [&] {
            auto &frag = ray_tracing->frag_shader;
            auto &vert = ray_tracing->vert_shader;

            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            frag.zFar = 100.0f;

            frag.ViewportSize = camera.GetViewportSize();

            vert.ProjectionMatrix = camera.GetFrustumMat();
            vert.ViewMatrix = camera.GetViewMat();

            frag.ProjectionMatrixPrev = prev_projection;
            frag.ViewMatrixPrev = prev_view;

            prev_projection = vert.ProjectionMatrix;
            prev_view = vert.ViewMatrix;

            frag.shadowTransformInv = world.GetShadow()->GetGlobalModelMatInv();

            frag.uni_palette.Bind(palette, 9);

            for (const auto &object : world.GetObjects()) {
                if (IsInside(object, camera.GetPosition())) {
                    glCullFace(GL_FRONT);
                } else {
                    glCullFace(GL_BACK);
                }

                vert.ModelMatrix = object->GetGlobalModelMat();
                vert.VoxelObjectDims = object->GetDims();
                frag.ModelViewMatrixInverse = glm::inverse(vert.ViewMatrix * vert.ModelMatrix);

                frag.ModelMatrixPrev = prev_model_matrix[object->GetId()];
                prev_model_matrix[object->GetId()] = object->GetGlobalModelMat();

                SetVoxelObjectUniform(frag.currentObject, object, 2);
                DrawCube();
            }
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        });

        lighting->Run(width, height, [&] {
            auto &frag = lighting->frag_shader;
            auto &vert = lighting->vert_shader;

            frag.uni_normal.Bind(ray_tracing->frag_shader.out_normal_texture, 2);
            frag.uni_depth.Bind(ray_tracing->frag_shader.gl_FragDepth_texture, 3);
            frag.uni_position.Bind(ray_tracing->frag_shader.out_shadow_position_texture, 4);
            frag.uni_blue_noise.Bind(blue_noise_texture, 5);
            frag.uni_seed = (int) (seed = (seed + 1) % 1000);
            frag.uni_light_color = sky_box.sky_color;
            frag.uni_light_type = 0;

            SetVoxelShadowUniform(frag.shadowObject, world.GetShadow(), 6);

            DrawQuad();

            if (sky_box.has_sun) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ONE);
                frag.uni_light_type = 1;
                frag.uni_light_color = sky_box.sun_color;
                frag.uni_light_pos = sky_box.sun_direction;
                frag.uni_radius = 0.03f;

                DrawQuad();
                glDisable(GL_BLEND);
            }
        });

        denoising->Run(width, height, [&] {
            auto &frag = denoising->frag_shader;
            auto &vert = denoising->vert_shader;

            frag.uni_input.Bind(lighting->frag_shader.out_light_texture, 2);
            frag.uni_depth.Bind(ray_tracing->frag_shader.gl_FragDepth_texture, 3);
            frag.uni_normal.Bind(ray_tracing->frag_shader.out_normal_texture, 4);
            frag.uni_position.Bind(ray_tracing->frag_shader.out_shadow_position_texture, 5);

            DrawQuad();
        });
        taa->Run(width, height, [&] {
            auto &frag = taa->frag_shader;

            frag.uni_input.Bind(denoising->frag_shader.out_denoised_texture, 1);
            frag.uni_normal.Bind(ray_tracing->frag_shader.out_normal_texture, 2);
            frag.uni_depth.Bind(ray_tracing->frag_shader.gl_FragDepth_texture, 3);
            frag.uni_velocity.Bind(ray_tracing->frag_shader.out_frame_offset_texture, 4);
            frag.uni_position.Bind(ray_tracing->frag_shader.out_shadow_position_texture, 5);
            if (!copy->frag_shader.out_color_texture.expired() && !copy->frag_shader.out_position_texture.expired()) {
                frag.uni_previous_input.Bind(copy->frag_shader.out_color_texture, 6);
                frag.uni_previous_position.Bind(copy->frag_shader.out_position_texture, 7);
            }

            DrawQuad();
        });
        copy->Run(width, height, [&] {
            auto &frag = copy->frag_shader;

            frag.uni_color.Bind(taa->frag_shader.out_accumulated_texture, 1);
            frag.uni_position.Bind(ray_tracing->frag_shader.out_shadow_position_texture, 2);

            DrawQuad();
        });
        reflections->Run(width, height, [&] {
            auto &frag = reflections->frag_shader;

            frag.uni_depth.Bind(ray_tracing->frag_shader.gl_FragDepth_texture, 1);
            frag.uni_normal.Bind(ray_tracing->frag_shader.out_normal_texture, 2);
            frag.uni_direction.Bind(ray_tracing->frag_shader.out_direction_texture, 3);
            frag.uni_position.Bind(ray_tracing->frag_shader.out_shadow_position_texture, 4);
            frag.uni_material.Bind(ray_tracing->frag_shader.out_material_texture, 5);
            frag.uni_cubemap.Bind(sky_box.cubemap, 6);

            SetVoxelShadowUniform(frag.shadowObject, world.GetShadow(), 7);

            DrawQuad();
        });
        light_mixer->Run(width, height, [&] {
            auto &frag = light_mixer->frag_shader;

            frag.uni_color.Bind(ray_tracing->frag_shader.out_color_texture, 1);
            frag.uni_light.Bind(taa->frag_shader.out_accumulated_texture, 2);
            frag.uni_reflection.Bind(reflections->frag_shader.out_reflection_texture, 3);

            DrawQuad();
        });
        final->Run(width, height, [&] {
            auto &frag = final->frag_shader;
            auto &vert = final->vert_shader;

            frag.uni_cubemap.Bind(GetSkyBox().cubemap, 1);
            frag.uni_depth.Bind(ray_tracing->frag_shader.gl_FragDepth_texture, 2);
            frag.uni_color.Bind(light_mixer->frag_shader.out_mixed_texture, 3);
            vert.uni_projection = camera.GetFrustumMat();
            vert.uni_view = camera.GetViewMat();

            frag.uni_gamma = DebugOptions::Instance().gamma;
            frag.uni_magic = DebugOptions::Instance().magic;

            DrawCube();
        });
        debug->Run(width, height, [&] {
            auto &frag = debug->frag_shader;

            frag.channel = DebugOptions::Instance().draw_channel;
            frag.zoom = DebugOptions::Instance().zoom;
            frag.viewport = camera.GetViewportSize();

            switch (DebugOptions::Instance().draw_buffer_option) {
                case DrawBufferOption::Final:
                    frag.uni_color.Bind(final->frag_shader.out_color_texture, 1);
                    break;
                case DrawBufferOption::TAAOutput:
                    frag.uni_color.Bind(taa->frag_shader.out_accumulated_texture, 1);
                    break;
                case DrawBufferOption::Color:
                    frag.uni_color.Bind(ray_tracing->frag_shader.out_color_texture, 1);
                    break;
                case DrawBufferOption::Depth:
                    frag.uni_color.Bind(ray_tracing->frag_shader.gl_FragDepth_texture, 1);
                    break;
                case DrawBufferOption::Normal:
                    frag.uni_color.Bind(ray_tracing->frag_shader.out_normal_texture, 1);
                    break;
                case DrawBufferOption::Velocity:
                    frag.uni_color.Bind(ray_tracing->frag_shader.out_frame_offset_texture, 1);
                    break;
                case DrawBufferOption::Lighting:
                    frag.uni_color.Bind(lighting->frag_shader.out_light_texture, 1);
                    break;
                case DrawBufferOption::DenoisedLighting:
                    frag.uni_color.Bind(denoising->frag_shader.out_denoised_texture, 1);
                    break;
                case DrawBufferOption::Noise:
                    frag.uni_color.Bind(blue_noise_texture, 1);
                    break;
            }

            DrawQuad();
        });
    }

    lit::voxels::SkyBox GetSkyBox() {
        switch (DebugOptions::Instance().sky_box_option) {
            case SkyBoxOption::Standard:
                return sky_box_loader.LoadSkyBox("standard");
            case SkyBoxOption::Sunset:
                return sky_box_loader.LoadSkyBox("sunset");
            case SkyBoxOption::Pink:
                return sky_box_loader.LoadSkyBox("pink");
            case SkyBoxOption::DeepDusk:
                return sky_box_loader.LoadSkyBox("deep_dusk");
            case SkyBoxOption::Space:
                return sky_box_loader.LoadSkyBox("space");
        }
        return sky_box_loader.LoadSkyBox("sunset");
    }

    std::unique_ptr<rendering::PipelineNode<RayTracingVert, RayTracingFrag>> ray_tracing;
    std::unique_ptr<rendering::PipelineNode<LightingVert, LightingFrag>> lighting;
    std::unique_ptr<rendering::PipelineNode<DenoisingVert, DenoisingFrag>> denoising;
    std::unique_ptr<rendering::PipelineNode<TAAVert, TAAFrag>> taa;
    std::unique_ptr<rendering::PipelineNode<CopyVert, CopyFrag>> copy;
    std::unique_ptr<rendering::PipelineNode<ReflectionsVert, ReflectionsFrag>> reflections;
    std::unique_ptr<rendering::PipelineNode<LightMixerVert, LightMixerFrag>> light_mixer;
    std::unique_ptr<rendering::PipelineNode<FinalVert, FinalFrag>> final;
    std::unique_ptr<rendering::PipelineNode<DebugVert, DebugFrag>> debug;

    uint32_t seed = 0;
    std::map<int, glm::mat4> prev_model_matrix;
    glm::mat4 prev_projection;
    glm::mat4 prev_view;

    std::shared_ptr<gl::Texture2D> blue_noise_texture;
    std::shared_ptr<gl::Texture2D> palette;

    lit::voxels::SkyBoxLoader sky_box_loader;
};

void lit::voxels::VoxelsPipeline::VoxelsPipelineImplDeleter::operator()(lit::voxels::VoxelsPipeline::VoxelsPipelineImpl *obj) {
    delete obj;
}

VoxelsPipeline::VoxelsPipeline(const std::shared_ptr<gl::Context> &ctx) {
    m_impl = std::unique_ptr<VoxelsPipelineImpl, VoxelsPipelineImplDeleter>(new VoxelsPipelineImpl(ctx));
}

void VoxelsPipeline::Run(Camera &camera, VoxelWorld &world) {
    m_impl->Run(camera, world);
}
