#pragma once

#include <vector>
#include <optional>
#include <unordered_map>
#include <glm/vec3.hpp>
#include <memory>

namespace lit::rendering::opengl {

    class ComputeShader {
    public:
        static ComputeShader Create(const std::string &shader_path);

        static std::optional<ComputeShader> TryCreate(const std::string &shader_path);

        ComputeShader(ComputeShader &&) = default;

        ComputeShader &operator=(ComputeShader &&) = default;

        ~ComputeShader();

        void Bind();

        void Unbind();

        void Dispatch(glm::ivec3 total_size);

        template<typename T>
        void SetUniform(const std::string &name, T value) {
            SetUniform(GetUniformLocation(name), value);
        }

        template<typename T>
        void SetUniform(int location, T value);

        int GetUniformLocation(const std::string &name);

        glm::ivec3 GetLocalGroupSize() const;

    protected:
        explicit ComputeShader(uint32_t program_id);

        glm::ivec3 m_local_group_size = glm::ivec3(1);

        std::unordered_map<std::string, int> m_location_cache;

        std::unique_ptr<uint32_t> m_program_id;
    };

}
