#pragma once

#include <lit/rendering/cpp_shader.hpp>
#include <glm/vector_relational.hpp>

namespace lit::voxels {

    using namespace glm;
    using namespace rendering;

    shader_vert(LightMixerVert) {
        shader_in(location = 0, vec3, in_vert);

        void main() {
            gl_Position = vec4(in_vert * 2.0f - 1.0f, 1);
        }
    };

    shader_frag(LightMixerFrag) {
        shader_out(location = 0, vec4, out_mixed, gl::Attachment::RGB16);

        uniform(sampler2D, uni_color);
        uniform(sampler2D, uni_light);
        uniform(sampler2D, uni_reflection);

        void main() {
            ivec2 coord = ivec2(gl_FragCoord.xy());
            out_mixed = texelFetch(uni_color, coord, 0) * texelFetch(uni_light, coord, 0) + texelFetch(uni_reflection, coord, 0);
        }
    };

}