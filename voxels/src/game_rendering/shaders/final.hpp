#pragma once

#include <lit/rendering/cpp_shader.hpp>
#include <glm/vector_relational.hpp>

namespace lit::voxels {

    using namespace glm;
    using namespace rendering;

    shader_vert(FinalVert) {
        shader_in(location = 0, vec3, in_vert);
        shader_out(location = 0, vec3, out_direction);

        uniform(mat4, uni_projection);
        uniform(mat4, uni_view);

        void main() {
            out_direction = (in_vert * 2.0f - 1.0f);
            vec3 pos = (uni_view * vec4(in_vert * 2.0f - 1.0f, 0)).xyz();
            gl_Position = uni_projection * vec4(pos, 1);
        }
    };

    shader_frag(FinalFrag) {
        shader_in(location = 0, vec3, in_direction);
        shader_out(location = 0, vec4, out_color, gl::Attachment::RGBA);

        uniform(samplerCube, uni_cubemap);
        uniform(sampler2D, uni_color);
        uniform(sampler2D, uni_depth);

        uniform(float, uni_magic);
        uniform(float, uni_gamma);

        void main() {
            ivec2 coord = ivec2(gl_FragCoord.xy());
            if (texelFetch(uni_depth, coord, 0).x > 0.999f) {
                out_color = texture(uni_cubemap, in_direction);
                return;
            }
            vec3 col = texelFetch(uni_color, coord, 0).xyz();
            col = (col) / (col + uni_magic);
            float gamma = uni_gamma;
            out_color = vec4(pow(col.x, 1.0f / gamma), pow(col.y, 1.0f / gamma), pow(col.z, 1.0f / gamma), 1);
        }
    };

}