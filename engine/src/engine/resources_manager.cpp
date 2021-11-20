#include <lit/engine/resources_manager.hpp>
#include <string>
#include <fstream>

using namespace lit::engine;
/*
std::string ResourcesManager::GetShaderSources(const std::string &shader_path) {
    std::ifstream fin(kShadersRoot.string() + shader_path);
    return {(std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>()};
}*/

std::filesystem::path ResourcesManager::GetAssetPath(const std::filesystem::path &relative_asset_path) {
    return kAssetsRoot / relative_asset_path;
}

std::string ResourcesManager::GetShaderPath(const std::string &relative_asset_path) {
    return kShadersRoot + relative_asset_path;
}
