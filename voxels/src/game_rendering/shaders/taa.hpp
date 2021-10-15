#pragma once

#include <lit/rendering/cpp_shader.hpp>
#include <glm/vector_relational.hpp>

namespace lit::voxels {

    using namespace glm;
    using namespace rendering;

    shader_vert(TAAVert) {
        shader_in(location = 0, vec3, in_vert);
        shader_out(location = 0, vec2, out_uv);

        void main() {
            gl_Position = vec4(in_vert * 2.0f - 1.0f, 1);
            out_uv = in_vert.xy();
        }
    };

    shader_frag(TAAFrag) {
        shader_in(location = 0, vec2, in_uv);
        shader_out(location = 0, vec4, out_accumulated, gl::Attachment::RGB16);

        uniform(sampler2D, uni_normal);
        uniform(sampler2D, uni_depth);
        uniform(sampler2D, uni_velocity);

        uniform(sampler2D, uni_input);
        uniform(sampler2D, uni_position);

        uniform(sampler2D, uni_previous_input);
        uniform(sampler2D, uni_previous_position);

        ivec2 coord_offset = ivec2(0);
        float coord_depth;

        void update_coord_offset(ivec2 coord, ivec2 offset) {
            float c_depth = texelFetch(uni_depth, coord + offset, 0).x;
            if (c_depth < coord_depth) {
                coord_depth = c_depth;
                coord_offset = offset;
            }
        }

        void main() {
            if (texture(uni_depth, in_uv, 0).x > 0.999f) {
                out_accumulated = vec4(0);
                return;
            }

            ivec2 coord = ivec2(gl_FragCoord.xy());
            coord_depth = texelFetch(uni_depth, coord, 0).x;

            update_coord_offset(coord, ivec2(1, -1));
            update_coord_offset(coord, ivec2(1, 0));
            update_coord_offset(coord, ivec2(1, 1));

            update_coord_offset(coord, ivec2(-1, -1));
            update_coord_offset(coord, ivec2(-1, 0));
            update_coord_offset(coord, ivec2(-1, 1));

            update_coord_offset(coord, ivec2(0, -1));
            update_coord_offset(coord, ivec2(0, 1));

            vec2 offset = texelFetch(uni_velocity, coord + coord_offset, 0).xy();

            vec3 lightCur = texelFetch(uni_input, coord, 0).xyz();
            vec3 positionCur = texelFetch(uni_position, coord, 0).xyz();

            if (any(greaterThan(in_uv + offset, vec2(1.0f))) || any(lessThan(in_uv + offset, vec2(0.0f)))) {
                out_accumulated = vec4(lightCur, 1);
                return;
            }

            vec3 lightPrev = texture(uni_previous_input, in_uv + offset, 0).xyz();
            vec3 positionPrev = texture(uni_previous_position, in_uv + offset, 0).xyz();

            float len = clamp(length(positionPrev - positionCur), 0.0f, 1.0f);
            float v = clamp(1.0f - len * len * 0.1f / coord_depth, 0.0f, 1.0f) * 0.95f;

            out_accumulated = vec4((lightCur * (1 - v) + lightPrev * v), 1);
        }
    };

}