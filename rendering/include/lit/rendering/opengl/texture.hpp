#pragma once

#include <lit/common/images/images.hpp>
#include <lit/common/glm_ext/region.hpp>
#include <vector>
#include <memory>

namespace lit::rendering::opengl {

    using lit::common::glm_ext::iregion3;

    enum class TextureMinFilter {
        Nearest,
        Linear,
        NearestMipMapNearest,
        LinearMipMapNearest,
        NearestMipMapLinear,
        LinearMipMapLinear
    };

    enum class TextureMagFilter {
        Nearest,
        Linear
    };

    enum class TextureWrap {
        Repeat,
        ClampToEdge,
        MirroredRepeat
    };

    enum class TextureInternalFormat {
        DepthComponent,
        R8UI,
        R32UI,
        R16F,
        RGB8,
        RGBA8,
        R32F,
        RGB32F,
        RGB16F,
        RGBA32F,
        RGB32I,
        RGBA32I,
        RGB32UI,
        RGBA32UI,
    };

    enum class TextureDataFormat {
        DepthComponent,
        RedInteger,
        Red,
        RGB,
        RGBA,
        RGBAInteger
    };

    enum class TextureDataType {
        UnsignedInt,
        UnsignedByte,
        Float
    };

    struct Texture2DInfo {
        TextureMinFilter min_filter = TextureMinFilter::Linear;
        TextureMagFilter mag_filter = TextureMagFilter::Linear;
        TextureInternalFormat internal_format = TextureInternalFormat::RGB8;
        TextureDataFormat data_format = TextureDataFormat::RGB;
        TextureDataType data_type = TextureDataType::UnsignedByte;
        TextureWrap wrap_s = TextureWrap::Repeat;
        TextureWrap wrap_t = TextureWrap::Repeat;
        uint32_t width = 512;
        uint32_t height = 512;
        uint32_t mip_levels = 0;
        std::vector<void *> data{};
    };

    struct Texture3DInfo {
        TextureMinFilter min_filter = TextureMinFilter::Nearest;
        TextureMagFilter mag_filter = TextureMagFilter::Nearest;
        TextureInternalFormat internal_format = TextureInternalFormat::R8UI;
        TextureDataFormat data_format = TextureDataFormat::RedInteger;
        TextureDataType data_type = TextureDataType::UnsignedByte;
        TextureWrap wrap_s = TextureWrap::Repeat;
        TextureWrap wrap_t = TextureWrap::Repeat;
        TextureWrap wrap_r = TextureWrap::Repeat;
        uint32_t width = 64;
        uint32_t height = 64;
        uint32_t depth = 64;
        uint32_t mip_levels = 0;
        std::vector<void *> data{};
    };

    class Texture2D {
    public:
        static Texture2D Create(const Texture2DInfo &textureInfo);

        //static Texture2D CreateFromImagePath(const std::filesystem::path &image_path);

        static Texture2D CreateFromImage(const lit::common::Image<uint8_t, 3> &image);

        static Texture2D CreateFromImage(const lit::common::Image<uint8_t, 4> &image);

        ~Texture2D();

        Texture2D(Texture2D &&) = default;

        Texture2D &operator=(Texture2D &&) = default;

        void Update(const lit::common::Image<uint8_t, 3> &img, int level = 0);

        void Update(const lit::common::Image<uint8_t, 4> &img, int level = 0);

        void BindToImage(int image_index, bool read = false);

        void Bind(int texture_index);

    private:
        explicit Texture2D(const Texture2DInfo &textureInfo);

        friend class FrameBuffer;

        std::unique_ptr<uint32_t> m_texture_id = 0;
        Texture2DInfo m_info;
    };

    class Texture3D {
    public:
        static Texture3D Create(const Texture3DInfo &textureInfo);

        ~Texture3D();

        Texture3D(Texture3D &&) = default;

        Texture3D &operator=(Texture3D &&) = default;

        void Update(const lit::common::Image3D<uint8_t> &img, int level = 0);

        void Update(const lit::common::Image3D<uint32_t> &img, int level = 0);

        void Update(const lit::common::Image3D<uint8_t> &img, const iregion3 &src, const iregion3 &dst, int level = 0);

        void Update(const lit::common::Image3D<uint32_t> &img, const iregion3 &src, const iregion3 &dst, int level = 0);

        void Update(void *data, int level = 0);

        void BindToImage(int image_index, int level = 0, bool read = false);

        void Bind(int texture_index);

    private:
        explicit Texture3D(const Texture3DInfo &textureInfo);

        std::unique_ptr<uint32_t> m_texture_id;
        Texture3DInfo m_info;
    };
}
