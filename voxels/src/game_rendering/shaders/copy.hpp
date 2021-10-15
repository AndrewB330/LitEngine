#pragma once

#include <lit/rendering/cpp_shader.hpp>
#include <glm/vector_relational.hpp>

namespace lit::voxels {

    using namespace glm;
    using namespace rendering;

    shader_vert(CopyVert) {
        shader_in(location = 0, vec3, in_vert);

        void main() {
            gl_Position = vec4(in_vert * 2.0f - 1.0f, 1);
        }
    };

    shader_frag(CopyFrag) {
        shader_out(location = 0, vec4, out_color, gl::Attachment::RGB16);
        shader_out(location = 1, vec4, out_position, gl::Attachment::RGB32);

        uniform(sampler2D, uni_color);
        uniform(sampler2D, uni_position);

        void main() {
            out_color = texelFetch(uni_color, ivec2(gl_FragCoord.xy()), 0);
            out_position = texelFetch(uni_position, ivec2(gl_FragCoord.xy()), 0);
        }
    };

}
