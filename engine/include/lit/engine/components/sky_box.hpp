#pragma once
#include <lit/rendering/opengl/texture.hpp>

namespace lit::engine {

    class SkyBoxComponent {
    public:
        using OpenglTextureCube = lit::rendering::opengl::TextureCube;

        explicit SkyBoxComponent(const std::filesystem::path & sky_box_path);

        const OpenglTextureCube & GetTextureCube() const;

    private:
        OpenglTextureCube m_texture_cube;
    };

}