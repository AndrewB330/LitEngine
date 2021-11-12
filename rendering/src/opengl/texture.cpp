#include <lit/rendering/opengl/texture.hpp>
#include <GL/glew.h>
#include <spdlog/spdlog.h>
//#include <lit/application/assert.hpp>
#include <lit/rendering/opengl/utils.hpp>
#include <lit/rendering/opengl/assert.hpp>

using namespace lit::rendering::opengl;

GLint GetMinFilterEnum(TextureMinFilter filter) {
    switch (filter) {
        case TextureMinFilter::Nearest:
            return GL_NEAREST;
        case TextureMinFilter::Linear:
            return GL_LINEAR;
        case TextureMinFilter::NearestMipMapNearest:
            return GL_NEAREST_MIPMAP_NEAREST;
        case TextureMinFilter::LinearMipMapNearest:
            return GL_LINEAR_MIPMAP_NEAREST;
        case TextureMinFilter::NearestMipMapLinear:
            return GL_NEAREST_MIPMAP_LINEAR;
        case TextureMinFilter::LinearMipMapLinear:
            return GL_LINEAR_MIPMAP_LINEAR;
    }
    return GL_NEAREST;
}

GLint GetMagFilterEnum(TextureMagFilter filter) {
    return filter == TextureMagFilter::Nearest ? GL_NEAREST : GL_LINEAR;
}

GLint GetTextureWrapEnum(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::Repeat:
            return GL_REPEAT;
        case TextureWrap::ClampToEdge:
            return GL_CLAMP_TO_EDGE;
        case TextureWrap::MirroredRepeat:
            return GL_MIRRORED_REPEAT;
    }
    return GL_REPEAT;
}

GLint GetTextureInternalFormatEnum(TextureInternalFormat format) {
    switch (format) {
        case TextureInternalFormat::DepthComponent:
            return GL_DEPTH_COMPONENT;
        case TextureInternalFormat::R8UI:
            return GL_R8UI;
        case TextureInternalFormat::R16F:
            return GL_R16F;
        case TextureInternalFormat::RGB8:
            return GL_RGB8;
        case TextureInternalFormat::RGBA8:
            return GL_RGBA8;
        case TextureInternalFormat::R32F:
            return GL_R32F;
        case TextureInternalFormat::RGB32F:
            return GL_RGB32F;
        case TextureInternalFormat::RGB16F:
            return GL_RGB16F;
        case TextureInternalFormat::RGBA32F:
            return GL_RGBA32F;
        case TextureInternalFormat::RGB32I:
            return GL_RGB32I;
        case TextureInternalFormat::RGBA32I:
            return GL_RGBA32I;
        case TextureInternalFormat::RGB32UI:
            return GL_RGB32UI;
        case TextureInternalFormat::RGBA32UI:
            return GL_RGBA32UI;
        case TextureInternalFormat::R32UI:
            return GL_R32UI;
    }

    return GL_RGB;
}

GLint GetTextureDataFormatEnum(TextureDataFormat format) {
    switch (format) {
        case TextureDataFormat::DepthComponent:
            return GL_DEPTH_COMPONENT;
        case TextureDataFormat::RedInteger:
            return GL_RED_INTEGER;
        case TextureDataFormat::Red:
            return GL_RED;
        case TextureDataFormat::RGB:
            return GL_RGB;
        case TextureDataFormat::RGBA:
            return GL_RGBA;
        case TextureDataFormat::RGBAInteger:
            return GL_RGBA_INTEGER;
    }
    return GL_RGB;
}

GLint GetTextureDataTypeEnum(TextureDataType type) {
    switch (type) {
        case TextureDataType::UnsignedByte:
            return GL_UNSIGNED_BYTE;
        case TextureDataType::Float:
            return GL_FLOAT;
        case TextureDataType::UnsignedInt:
            return GL_UNSIGNED_INT;
    }
    return GL_UNSIGNED_BYTE;
}

Texture2D Texture2D::Create(const Texture2DInfo &textureInfo) {
    return Texture2D(textureInfo);
}

/*Texture2D Texture2D::CreateFromImagePath(const std::string& image_path) {
    return CreateFromImage( lit::common::ReadPNG_RGBA(image_path.string()));
}*/

Texture2D Texture2D::CreateFromImage(const lit::common::Image<uint8_t, 3> &image) {
    Texture2DInfo info;
    info.width = image.GetWidth();
    info.height = image.GetHeight();
    info.data.push_back((void*)image.GetDataPointer());
    return Texture2D(info);
}

Texture2D Texture2D::CreateFromImage(const lit::common::Image<uint8_t, 4> &image) {
    Texture2DInfo info;
    info.width = image.GetWidth();
    info.height = image.GetHeight();
    info.internal_format = TextureInternalFormat::RGBA8;
    info.data_format = TextureDataFormat::RGBA;
    info.data.push_back((void*)image.GetDataPointer());
    return Texture2D(info);
}

Texture2D::Texture2D(const Texture2DInfo &textureInfo) {
    m_info = textureInfo;

    m_texture_id = std::make_unique<uint32_t>();
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, m_texture_id.get());
    glBindTexture(GL_TEXTURE_2D, *m_texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetMinFilterEnum(m_info.min_filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetMagFilterEnum(m_info.mag_filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetTextureWrapEnum(m_info.wrap_s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetTextureWrapEnum(m_info.wrap_t));
    glTexImage2D(GL_TEXTURE_2D, 0, GetTextureInternalFormatEnum(m_info.internal_format),
                 (int) m_info.width, (int) m_info.height, 0,
                 GetTextureDataFormatEnum(m_info.data_format),
                 GetTextureDataTypeEnum(m_info.data_type), textureInfo.data.empty() ? nullptr : textureInfo.data[0]);
    for (int i = 1; i <= m_info.mip_levels; i++) {
        glTexImage2D(GL_TEXTURE_2D, i, GetTextureInternalFormatEnum(m_info.internal_format),
                     (int) (m_info.width >> i), (int) (m_info.height >> i), 0,
                     GetTextureDataFormatEnum(m_info.data_format),
                     GetTextureDataTypeEnum(m_info.data_type),
                     textureInfo.data.size() < i ? nullptr : textureInfo.data[i]);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    spdlog::default_logger()->trace("Texture2D {}x{} created.", m_info.width, m_info.height);
}

Texture2D::~Texture2D() {
    if (m_texture_id) {
        glDeleteTextures(1, m_texture_id.get());
        m_texture_id.reset();
        spdlog::default_logger()->trace("Texture2D {}x{} deleted.", m_info.width, m_info.height);
    }
}

void Texture2D::Bind(int texture_index) {
    if (m_texture_id) {
        glActiveTexture(GL_TEXTURE0 + texture_index);
        glBindTexture(GL_TEXTURE_2D, *m_texture_id);
    }
}

void Texture2D::BindToImage(int image_index) {
    if (m_texture_id) {
        glBindImageTexture(image_index, *m_texture_id, 0, GL_TRUE, 0, GL_WRITE_ONLY,
                           GetTextureInternalFormatEnum(m_info.internal_format));
    }
}

Texture3D::Texture3D(const Texture3DInfo &textureInfo) {
    m_info = textureInfo;

    glActiveTexture(GL_TEXTURE0);
    m_texture_id = std::make_unique<uint32_t>();
    glGenTextures(1, m_texture_id.get());
    glBindTexture(GL_TEXTURE_3D, *m_texture_id);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GetMinFilterEnum(m_info.min_filter));
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GetMagFilterEnum(m_info.mag_filter));
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GetTextureWrapEnum(m_info.wrap_s));
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GetTextureWrapEnum(m_info.wrap_t));
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GetTextureWrapEnum(m_info.wrap_r));

    glTexImage3D(GL_TEXTURE_3D, 0, GetTextureInternalFormatEnum(m_info.internal_format),
                 (int) m_info.width, (int) m_info.height, (int) m_info.depth, 0,
                 GetTextureDataFormatEnum(m_info.data_format),
                 GetTextureDataTypeEnum(m_info.data_type), textureInfo.data.empty() ? nullptr : textureInfo.data[0]);

    for (int i = 1; i <= m_info.mip_levels; i++) {
        glTexImage3D(GL_TEXTURE_3D, i, GetTextureInternalFormatEnum(m_info.internal_format),
                     (int) (m_info.width >> i), (int) (m_info.height >> i), (int) (m_info.depth >> i), 0,
                     GetTextureDataFormatEnum(m_info.data_format),
                     GetTextureDataTypeEnum(m_info.data_type),
                     textureInfo.data.size() < i ? nullptr : textureInfo.data[i]);
    }
    glBindTexture(GL_TEXTURE_3D, 0);

    spdlog::default_logger()->trace("Texture3D {}x{}x{} created.", m_info.width, m_info.height, m_info.depth);
}

Texture3D::~Texture3D() {
    if (m_texture_id) {
        glDeleteTextures(1, m_texture_id.get());
        m_texture_id.reset();
        spdlog::default_logger()->trace("Texture3D {}x{}x{} deleted.", m_info.width, m_info.height, m_info.depth);
    }
}

void Texture3D::Bind(int texture_index) {
    if (m_texture_id) {
        glActiveTexture(GL_TEXTURE0 + texture_index);
        glBindTexture(GL_TEXTURE_3D, *m_texture_id);
    }
}

uint8_t GetChannelsNumber(TextureDataFormat format) {
    switch (format) {
        case TextureDataFormat::RedInteger:
        case TextureDataFormat::Red:
        case TextureDataFormat::DepthComponent:
            return 1;
        case TextureDataFormat::RGB:
            return 3;
        case TextureDataFormat::RGBA:
        case TextureDataFormat::RGBAInteger:
            return 4;
    }
    return 0;
}

void Texture2D::Update(const lit::common::Image<uint8_t, 3> &img, int level) {
    auto l = spdlog::default_logger();
    LIT_ASSERT(m_info.data_type == TextureDataType::UnsignedByte, "Image type and texture data type must match", l)
    LIT_ASSERT(GetChannelsNumber(m_info.data_format) == 3, "Number of channels and data format must match", l)
    LIT_ASSERT(img.GetWidth() == (m_info.width >> level), "Image width and texture width must match", l)
    LIT_ASSERT(img.GetHeight() == (m_info.height >> level), "Image height and texture height must match", l)

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTextureSubImage2D(
            *m_texture_id,
            level,
            0, 0,
            (int) img.GetWidth(), (int) img.GetHeight(),
            GetTextureDataFormatEnum(m_info.data_format),
            GetTextureDataTypeEnum(m_info.data_type),
            img.GetDataPointer()
    );

    spdlog::default_logger()->trace("Texture2D {}x{} level {} updated.", m_info.width, m_info.height, level);
}

void Texture2D::Update(const lit::common::Image<uint8_t, 4> &img, int level) {
    auto l = spdlog::default_logger();
    LIT_ASSERT(m_info.data_type == TextureDataType::UnsignedByte, "Image type and texture data type must match", l)
    LIT_ASSERT(GetChannelsNumber(m_info.data_format) == 4, "Number of channels and data format must match", l)
    LIT_ASSERT(img.GetWidth() == (m_info.width >> level), "Image width and texture width must match", l)
    LIT_ASSERT(img.GetHeight() == (m_info.height >> level), "Image height and texture height must match", l)

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTextureSubImage2D(
            *m_texture_id,
            level,
            0, 0,
            (int) img.GetWidth(), (int) img.GetHeight(),
            GetTextureDataFormatEnum(m_info.data_format),
            GetTextureDataTypeEnum(m_info.data_type),
            img.GetDataPointer()
    );

    spdlog::default_logger()->trace("Texture2D {}x{} level {} updated.", m_info.width, m_info.height, level);
}

void Texture3D::Update(const lit::common::Image3D<uint8_t> &img, int level) {
    auto l = spdlog::default_logger();
    LIT_ASSERT(m_info.data_type == TextureDataType::UnsignedByte, "Image type and texture data type must match", l)
    LIT_ASSERT(GetChannelsNumber(m_info.data_format) == 1, "Number of channels and data format must match", l)
    LIT_ASSERT(img.GetWidth() == (m_info.width >> level), "Image width and texture width must match", l)
    LIT_ASSERT(img.GetHeight() == (m_info.height >> level), "Image height and texture height must match", l)
    LIT_ASSERT(img.GetDepth() == (m_info.depth >> level), "Image depth and texture depth must match", l)

    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

    GL_CALL(glTextureSubImage3D(
            *m_texture_id,
            level,
            0, 0, 0,
            (int) img.GetWidth(), (int) img.GetHeight(), (int) img.GetDepth(),
            GetTextureDataFormatEnum(m_info.data_format),
            GetTextureDataTypeEnum(m_info.data_type),
            (void *) img.GetDataPointer()
    ));

    spdlog::default_logger()->trace("Texture3D {}x{}x{} level {} updated.",
                                    m_info.width, m_info.height, m_info.depth, level);
}

void Texture3D::Update(const lit::common::Image3D<uint32_t> &img, int level) {
    auto l = spdlog::default_logger();
    LIT_ASSERT(m_info.data_type == TextureDataType::UnsignedInt, "Image type and texture data type must match", l)
    LIT_ASSERT(GetChannelsNumber(m_info.data_format) == 1, "Number of channels and data format must match", l)
    LIT_ASSERT(img.GetWidth() == (m_info.width >> level), "Image width and texture width must match", l)
    LIT_ASSERT(img.GetHeight() == (m_info.height >> level), "Image height and texture height must match", l)
    LIT_ASSERT(img.GetDepth() == (m_info.depth >> level), "Image depth and texture depth must match", l)

    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));

    GL_CALL(glTextureSubImage3D(
            *m_texture_id,
            level,
            0, 0, 0,
            (int) img.GetWidth(), (int) img.GetHeight(), (int) img.GetDepth(),
            GetTextureDataFormatEnum(m_info.data_format),
            GetTextureDataTypeEnum(m_info.data_type),
            (void *) img.GetDataPointer()
    ));

    spdlog::default_logger()->trace("Texture3D {}x{}x{} level {} updated.",
                                    m_info.width, m_info.height, m_info.depth, level);
}

void Texture3D::Update(const lit::common::Image3D<uint8_t> &img, const iregion3 &src, const iregion3 &dst, int level) {
    auto l = spdlog::default_logger();
    LIT_ASSERT(m_info.data_type == TextureDataType::UnsignedByte, "Image type and texture data type must match", l)
    LIT_ASSERT(GetChannelsNumber(m_info.data_format) == 1, "Number of channels and data format must match", l)

    glm::ivec3 dims = src.end - src.begin;

    LIT_ASSERT(dims == (dst.end - dst.begin), "Src and Dst dims must be equal", l);

    LIT_ASSERT(dims.x <= (m_info.width >> level), "Region width must be not larger than texture width", l)
    LIT_ASSERT(dims.y <= (m_info.height >> level), "Region height must be not larger than texture height", l)
    LIT_ASSERT(dims.z <= (m_info.depth >> level), "Region depth must be not larger than texture depth", l)

    std::vector<uint8_t> region_data(glm::compMul(dims), 0);
    for (int x = 0; x < dims.x; x++) {
        for (int y = 0; y < dims.y; y++) {
            for (int z = 0; z < dims.z; z++) {
                int new_index = x + y * dims.x + z * dims.x * dims.y;
                region_data[new_index] = img.GetPixel(src.begin + glm::ivec3(x, y, z));
            }
        }
    }

    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

    GL_CALL(glTextureSubImage3D(
            *m_texture_id,
            level,
            dst.begin.x, dst.begin.y, dst.begin.z,
            dims.x, dims.y, dims.z,
            GetTextureDataFormatEnum(m_info.data_format),
            GetTextureDataTypeEnum(m_info.data_type),
            region_data.data()
    ));

    spdlog::default_logger()->trace("Texture3D region {}x{}x{} with offset {};{};{} at level {} updated.",
                                    dims.x, dims.y, dims.z, dst.begin.x, dst.begin.y, dst.begin.z, level);
}

void Texture3D::Update(const lit::common::Image3D<uint32_t> &img, const iregion3 &src, const iregion3 &dst, int level) {
    auto l = spdlog::default_logger();
    LIT_ASSERT(m_info.data_type == TextureDataType::UnsignedInt, "Image type and texture data type must match", l)
    LIT_ASSERT(GetChannelsNumber(m_info.data_format) == 1, "Number of channels and data format must match", l)

    glm::ivec3 dims = src.end - src.begin;

    LIT_ASSERT(dims == (dst.end - dst.begin), "Src and Dst dims must be equal", l);

    if (dims.x > (m_info.width >> level)){
        int c=0;
    }
    LIT_ASSERT(dims.x <= (m_info.width >> level), "Region width must be not larger than texture width", l)
    LIT_ASSERT(dims.y <= (m_info.height >> level), "Region height must be not larger than texture height", l)
    LIT_ASSERT(dims.z <= (m_info.depth >> level), "Region depth must be not larger than texture depth", l)

    std::vector<uint32_t> region_data(glm::compMul(dims), 0);
    for (int x = 0; x < dims.x; x++) {
        for (int y = 0; y < dims.y; y++) {
            for (int z = 0; z < dims.z; z++) {
                int new_index = x + y * dims.x + z * dims.x * dims.y;
                region_data[new_index] = img.GetPixel(src.begin + glm::ivec3(x, y, z));
            }
        }
    }

    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));

    GL_CALL(glTextureSubImage3D(
            *m_texture_id,
            level,
            dst.begin.x, dst.begin.y, dst.begin.z,
            dims.x, dims.y, dims.z,
            GetTextureDataFormatEnum(m_info.data_format),
            GetTextureDataTypeEnum(m_info.data_type),
            region_data.data()
    ));

    spdlog::default_logger()->trace("Texture3D region {}x{}x{} with offset {};{};{} at level {} updated.",
                                    dims.x, dims.y, dims.z, dst.begin.x, dst.begin.y, dst.begin.z, level);
}

Texture3D Texture3D::Create(const Texture3DInfo &textureInfo) {
    return Texture3D(textureInfo);
}

void Texture3D::BindToImage(int image_index, int level, bool read) {
    if (m_texture_id) {
        GL_CALL(glBindImageTexture(image_index, *m_texture_id, level, GL_TRUE, 0, read ? GL_READ_ONLY : GL_WRITE_ONLY,
                           GetTextureInternalFormatEnum(m_info.internal_format)));
    }
}

void Texture3D::Update(void * data, int level) {
    GL_CALL(glTextureSubImage3D(
            *m_texture_id,
            level,
            0, 0, 0,
            (int) m_info.width, (int) m_info.height, (int) m_info.depth,
            GetTextureDataFormatEnum(m_info.data_format),
            GetTextureDataTypeEnum(m_info.data_type),
            data
    ));

    spdlog::default_logger()->trace("Texture3D {}x{}x{} level {} updated.",
                                    m_info.width, m_info.height, m_info.depth, level);
}
