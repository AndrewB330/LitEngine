#include <lit/common/images/images.hpp>
#include <lit/common/images/lodepng.hpp>
#include <glm/detail/type_vec4.hpp>

#include <glm/fwd.hpp>

using namespace lit::common;

Image<uint8_t, 4> lit::common::ReadPNG_RGBA(const std::string &filename) {
    std::vector<unsigned char> data; //the raw pixels
    unsigned width, height;
    unsigned error = lodepng::decode(data, width, height, filename);

    Image<uint8_t, 4> res(std::max(width, 256u), std::max(height, 256u));

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            int offset = 4 * y * (int)width + 4 * x;
            res.SetPixel(x, y, Image<glm::uint8_t, 4>::pixel_t(data[offset + 0], data[offset + 1], data[offset + 2],
                                                               data[offset + 3]));
        }
    }

    return res;
}

Image<uint8_t, 3> lit::common::ReadPNG_RGB(const std::string &filename) {
    Image<uint8_t, 4> image_rgba = ReadPNG_RGBA(filename);
    auto width = image_rgba.GetWidth();
    auto height = image_rgba.GetHeight();
    Image<uint8_t, 3> res(width, height);
    for(int i = 0; i < width; i++) {
        for(int j = 0; j < height; j++) {
            res.SetPixel(i, j, Image<uint8_t, 3>::pixel_t(image_rgba.GetPixel(i, j)));
        }
    }
    return res;
}