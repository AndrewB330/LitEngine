#include <lit/engine/components/sky_box.hpp>
#include <lit/rendering/opengl/texture.hpp>

using namespace lit::engine;

SkyBoxComponent::SkyBoxComponent(const std::filesystem::path &sky_box_path): m_texture_cube(OpenglTextureCube::Create(sky_box_path)) {

}

const SkyBoxComponent::OpenglTextureCube &SkyBoxComponent::GetTextureCube() const {
    return m_texture_cube;
}
