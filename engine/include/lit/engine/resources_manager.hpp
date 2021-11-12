#pragma once

#include <string>

namespace lit::engine {
    class ResourcesManager {
    public:
        //static std::string GetShaderSources(const std::string &shader_path);

        //static std::filesystem::path GetAssetPath(const std::filesystem::path &relative_asset_path);

        static std::string GetShaderPath(const std::string &relative_shader_path);

    private:
        inline static const std::string kAssetsRoot = "../assets/";
        inline static const std::string kShadersRoot = "../shaders/";
    };
}