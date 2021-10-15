#include <lit/common/images/image.hpp>
#include <lit/common/images/lodepng.hpp>
#include <glm/detail/type_vec4.hpp>

#include <glm/fwd.hpp>

using namespace lit::common;

image<uint8_t, 4> lit::common::read_png(const std::string &filename) {
    std::vector<unsigned char> data; //the raw pixels
    unsigned width, height;
    unsigned error = lodepng::decode(data, width, height, filename);

    image<uint8_t, 4> res(std::max(width, 256u), std::max(height, 256u));

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            int offset = 4 * y * (int)width + 4 * x;
            res.set_pixel(x, y, image<glm::uint8_t, 4>::pixel_t(data[offset + 0], data[offset + 1], data[offset + 2],
                                                                data[offset + 3]));
        }
    }

    return res;
}

image<uint8_t, 3> lit::common::read_png_rgb(const std::string &filename) {
    image<uint8_t, 4> image_rgba = read_png(filename);
    auto width = image_rgba.get_width();
    auto height = image_rgba.get_height();
    image<uint8_t, 3> res(width, height);
    for(int i = 0; i < width; i++) {
        for(int j = 0; j < height; j++) {
            res.set_pixel(i, j, image<uint8_t, 3>::pixel_t(image_rgba.get_pixel(i, j)));
        }
    }
    return res;
}