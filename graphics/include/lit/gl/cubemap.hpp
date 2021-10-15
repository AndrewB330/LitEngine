#pragma once

#include <lit/common/images/image.hpp>
#include <array>
#include "context.hpp"

namespace lit::gl {

    class Cubemap : public ContextObject {
    public:
        using image_t = lit::common::image<uint8_t, 4>;

        Cubemap(std::shared_ptr<Context> ctx, const std::array<image_t, 6> & sides);

        ~Cubemap();

        void Bind(int texture_index);

    private:

        uint32_t m_texture_id = 0;
    };

}
