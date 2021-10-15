#pragma once

#include <lit/common/images/image.hpp>
#include <lit/common/glm_ext/region.hpp>
#include <vector>
#include <GL/glew.h>
#include "context.hpp"

namespace lit::gl {

    using lit::common::glm_ext::iregion3;

    enum class TextureMinFilter : GLint {
        Nearest = GL_NEAREST,
        Linear = GL_LINEAR,
        NearestMipMapNearest = GL_NEAREST_MIPMAP_NEAREST,
        LinearMipMapNearest = GL_LINEAR_MIPMAP_NEAREST,
        NearestMipMapLinear = GL_NEAREST_MIPMAP_LINEAR,
        LinearMipMapLinear = GL_LINEAR_MIPMAP_LINEAR
    };

    enum class TextureMagFilter : GLint {
        Nearest = GL_NEAREST,
        Linear = GL_LINEAR
    };

    enum class TextureWrap : GLint {
        Repeat = GL_REPEAT,
        ClampToEdge = GL_CLAMP_TO_EDGE,
        MirroredRepeat = GL_MIRRORED_REPEAT
    };

    enum class TextureInternalFormat : GLint {
        DepthComponent = GL_DEPTH_COMPONENT,
        R8UI = GL_R8UI,
        R16F = GL_R16F,
        RGB = GL_RGB,
        RGBA = GL_RGBA,
        RGBA8 = GL_RGBA8,
        R32F = GL_R32F,
        RGB32F = GL_RGB32F,
        RGB16F = GL_RGB16F,
        RGBA32F = GL_RGBA32F,
        RGB32I = GL_RGB32I,
        RGBA32I = GL_RGBA32I,
        RGB32UI = GL_RGB32UI,
        RGBA32UI = GL_RGBA32UI,
    };

    enum class TextureDataFormat : GLint {
        DepthComponent = GL_DEPTH_COMPONENT,
        RedInteger = GL_RED_INTEGER,
        Red = GL_RED,
        RGB = GL_RGB,
        RGBA = GL_RGBA,
        RGBAInteger = GL_RGBA_INTEGER
    };

    enum class TextureDataType : GLint {
        UnsignedByte = GL_UNSIGNED_BYTE,
        Float = GL_FLOAT
    };

    struct Texture2DInfo {
        TextureMinFilter min_filter = TextureMinFilter::Linear;
        TextureMagFilter mag_filter = TextureMagFilter::Linear;
        TextureInternalFormat internal_format = TextureInternalFormat::RGB;
        TextureDataFormat data_format = TextureDataFormat::RGB;
        TextureDataType data_type = TextureDataType::UnsignedByte;
        TextureWrap wrap_s = TextureWrap::Repeat;
        TextureWrap wrap_t = TextureWrap::Repeat;
        int width = kDefaultTexSize;
        int height = kDefaultTexSize;
        int mip_levels = 0;
    };

    struct Texture3DInfo : public Texture2DInfo {
        TextureWrap wrap_r = TextureWrap::Repeat;
        int depth = kDefaultTexSize; // todo: maybe smaller? or do not derive from 2D info LOL
    };

    class Texture2D : public ContextObject {
    public:
        Texture2D(std::shared_ptr<Context> ctx, const Texture2DInfo &textureInfo);

        ~Texture2D();

        template<typename T, uint8_t C>
        void Update(const lit::common::image<T, C> &img, int level = 0);

        void Bind(int texture_index);

    private:
        friend class FrameBuffer;

        GLuint m_texture_id = 0;
        Texture2DInfo m_info;
    };

    class Texture3D : public ContextObject {
    public:
        const int kMinSide = 4;

        Texture3D(std::shared_ptr<Context> ctx, const Texture3DInfo &textureInfo);

        ~Texture3D();

        template<typename T>
        void Update(const lit::common::image3d<T> &img, int level = 0);

        template<typename T>
        void Update(const lit::common::image3d<T> &img, const iregion3 &region, int level = 0);

        void Bind(int texture_index);

    private:
        GLuint m_texture_id = 0;
        Texture3DInfo m_info;
    };

    uint8_t GetChannelsNumber(TextureDataFormat format);

    template<typename T>
    bool CheckDataTypeMatch(const TextureDataType &data_type) {
        return (std::is_same<T, float>::value == (data_type == TextureDataType::Float)) &&
               (std::is_same<T, uint8_t>::value == (data_type == TextureDataType::UnsignedByte));
    }

    template<typename T, uint8_t C>
    void Texture2D::Update(const lit::common::image<T, C> &img, int level) {
        auto l = m_ctx->GetLogger();
        LIT_ASSERT(CheckDataTypeMatch<T>(m_info.data_type), "Image type and texture data type must match", l)
        LIT_ASSERT(GetChannelsNumber(m_info.data_format) == C, "Number of channels and data format must match", l)
        LIT_ASSERT(img.get_width() == (m_info.width >> level), "Image width and texture width must match", l)
        LIT_ASSERT(img.get_height() == (m_info.height >> level), "Image height and texture height must match", l)

        glTextureSubImage2D(
                m_texture_id,
                level,
                0, 0,
                img.get_width(), img.get_height(),
                (GLenum) m_info.data_format,
                (GLenum) m_info.data_type,
                img.data()
        );
    }

    template<typename T>
    void Texture3D::Update(const lit::common::image3d<T> &img, int level) {
        auto l = m_ctx->GetLogger();
        LIT_ASSERT(CheckDataTypeMatch<T>(m_info.data_type), "Image type and texture data type must match", l)
        LIT_ASSERT(GetChannelsNumber(m_info.data_format) == 1, "Number of channels and data format must match", l)
        LIT_ASSERT(img.get_width() == (m_info.width >> level), "Image width and texture width must match", l)
        LIT_ASSERT(img.get_height() == (m_info.height >> level), "Image height and texture height must match", l)
        LIT_ASSERT(img.get_depth() == (m_info.depth >> level), "Image depth and texture depth must match", l)

        glTextureSubImage3D(
                m_texture_id,
                level,
                0, 0, 0,
                img.get_width(), img.get_height(), img.get_depth(),
                (GLenum) m_info.data_format,
                (GLenum) m_info.data_type,
                img.data()
        );
    }

    template<typename T>
    void Texture3D::Update(const lit::common::image3d<T> &img, const iregion3 &region, int level) {
        auto l = m_ctx->GetLogger();
        LIT_ASSERT(CheckDataTypeMatch<T>(m_info.data_type), "Image type and texture data type must match", l)
        LIT_ASSERT(GetChannelsNumber(m_info.data_format) == 1, "Number of channels and data format must match", l)

        glm::ivec3 dims = region.end - region.begin;

        LIT_ASSERT(dims.x <= (m_info.width >> level), "Region width must be not larger than texture width", l)
        LIT_ASSERT(dims.y <= (m_info.height >> level), "Region height must be not larger than texture height", l)
        LIT_ASSERT(dims.z <= (m_info.depth >> level), "Region depth must be not larger than texture depth", l)

        LIT_ASSERT(dims.x >= kMinSide, "Region width must be at least kMinSide", l)
        LIT_ASSERT(dims.y >= kMinSide, "Region height must be at least kMinSide", l)
        LIT_ASSERT(dims.z >= kMinSide, "Region depth must be at least kMinSide", l)

        std::vector<T> region_data;
        // resize
        region_data.assign(glm::compMul(dims), 0);
        for (int x = 0; x < dims.x; x++) {
            for (int y = 0; y < dims.y; y++) {
                for (int z = 0; z < dims.z; z++) {
                    int new_index = x + y * dims.x + z * dims.x * dims.y;
                    region_data[new_index] = img.get_pixel(region.begin + glm::ivec3(x, y, z));
                }
            }
        }

        glTextureSubImage3D(
                m_texture_id,
                level,
                region.begin.x, region.begin.y, region.begin.z,
                dims.x, dims.y, dims.z,
                (GLenum) m_info.data_format,
                (GLenum) m_info.data_type,
                region_data.data()
        );
    }
}
