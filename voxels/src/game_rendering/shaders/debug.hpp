#pragma once

#include <lit/rendering/cpp_shader.hpp>
#include <glm/vector_relational.hpp>

namespace lit::voxels {

    using namespace glm;
    using namespace rendering;

    shader_vert(DebugVert) {
        shader_in(location = 0, vec3, in_vert);

        void main() {
            gl_Position = vec4(in_vert * 2.0f - 1.0f, 1);
        }
    };

    shader_frag(DebugFrag) {
        shader_out(location = 0, vec4, out_color, gl::Attachment::RGBA);

        uniform(sampler2D, uni_color);

        uniform(vec2, viewport);
        uniform(int, channel);
        uniform(int, zoom);

        void main() {
            vec2 zoom_frag_coord = (gl_FragCoord.xy() - viewport * 0.5f) / float(zoom) + viewport * 0.5f;
            out_color = abs(texelFetch(uni_color, ivec2(zoom_frag_coord), 0));

            if (channel == 1)
                out_color = out_color.xxxw();
            if (channel == 2)
                out_color = out_color.yyyw();
            if (channel == 3)
                out_color = out_color.zzzw();
        }
    };

}