#pragma once

#include <functional>

#include <lit/gl/program.hpp>
#include <lit/gl/frame_buffer.hpp>
#include "cpp_shader.hpp"

namespace lit::rendering {

    template<typename VertShaderT, typename FragShaderT>
    class PipelineNode final : public gl::ContextObject {
    public:
        using UpdateFunc = std::function<void()>;

        explicit PipelineNode(const std::shared_ptr<gl::Context> &ctx, bool final = false);

        ~PipelineNode() = default;

        void Run(uint32_t width, uint32_t height, const UpdateFunc& func);

        VertShaderT vert_shader;

        FragShaderT frag_shader;

    private:
        void FillShaderUniforms(CppShader &shader);

        std::unique_ptr<gl::FrameBuffer> m_output = nullptr;
        std::unique_ptr<gl::Program> m_program = nullptr;
    };

    template<typename VertShaderT, typename FragShaderT>
    void PipelineNode<VertShaderT, FragShaderT>::Run(uint32_t width, uint32_t height, const UpdateFunc& func) {
        if (m_output && (width != m_output->GetWidth() || height != m_output->GetHeight())) {
            m_output = std::make_unique<gl::FrameBuffer>(m_ctx, gl::FrameBufferInfo{(int) width, (int) height,
                                                                                    frag_shader.GetOutputTypes()});

            /*LIT_THROW_IF(frag_shader.m_output_textures.size() != m_output->GetAttachments().size(),
                         "Number of shader outputs and frame buffer attachments must match")*/
            for (size_t i = 0; i < frag_shader.m_output_textures.size(); i++) {
                *(frag_shader.m_output_textures[i]) = m_output->GetAttachments()[i];
            }
        }

        if (m_output) {
            m_output->Bind();
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        }
        m_program->Use();
        glViewport(0, 0, width, height);
        func();
    }

    template<typename VertShaderT, typename FragShaderT>
    PipelineNode<VertShaderT, FragShaderT>::PipelineNode(const std::shared_ptr<gl::Context> &ctx, bool final)
            :ContextObject(ctx) {
        auto vert = std::make_shared<gl::Shader>(ctx, gl::ShaderType::Vertex, vert_shader.GetGlslCode());
        auto frag = std::make_shared<gl::Shader>(ctx, gl::ShaderType::Fragment, frag_shader.GetGlslCode());

        if (!final) {
            m_output = std::make_unique<gl::FrameBuffer>(ctx,
                                                         gl::FrameBufferInfo{gl::kDefaultTexSize, gl::kDefaultTexSize,
                                                                             frag_shader.GetOutputTypes()});
        }

        m_program = std::make_unique<gl::Program>(ctx, gl::ProgramInfo());
        m_program->AttachShader(vert);
        m_program->AttachShader(frag);
        m_program->Link();

        FillShaderUniforms(vert_shader);
        FillShaderUniforms(frag_shader);
    }

    template<typename VertShaderT, typename FragShaderT>
    void PipelineNode<VertShaderT, FragShaderT>::FillShaderUniforms(CppShader &shader) {
        for (auto &[name, res] : shader.m_uniform_locations) {
            *res = m_program->GetUniformLocation(name);
        }
        for (auto &[structure_name, structure] : shader.m_uniform_struct_holders) {
            for (auto &[name, res] : structure->m_uniform_locations) {
                std::string concatenated = structure_name + '.';
                concatenated += name;
                *res = m_program->GetUniformLocation(concatenated);
            }
        }
    }

    void DrawQuad();

    void DrawCube();

    void DrawCubeWithNormals();
}