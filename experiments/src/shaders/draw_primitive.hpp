#pragma once

#include <lit/rendering/cpp_shader.hpp>
#include <glm/vector_relational.hpp>
#include <glm/glm.hpp>

using namespace glm;
using namespace lit::rendering;

namespace shaders {

    shader_vert(DrawShadowPrimitiveVert) {
        shader_in(location = 0, vec3, in_vert);

        shader_out(location = 0, vec3, out_pos);

        uniform(mat4, uni_projection);
        uniform(mat4, uni_view);
        uniform(mat4, uni_model);

        uniform(vec3, uni_size);

        void main() {
            out_pos = uni_size * (in_vert - 0.5f);
            gl_Position = uni_projection * uni_view * uni_model * vec4(out_pos, 1);
        }
    };

    shader_frag(DrawShadowPrimitiveFrag) {
        shader_in(location = 0, vec3, in_pos);

        shader_out(depth_any, float, gl_FragDepth, lit::gl::Attachment::DepthComponent);

        uniform(mat4, uni_projection);
        uniform(mat4, uni_view);
        uniform(mat4, uni_model);

        uniform(mat4, uni_model_view_inv);

        uniform(int, uni_type);
        uniform(float, uni_radius);

        void main() {
            if (uni_type == 0) { // box
                gl_FragDepth = gl_FragCoord.z;
            }
            if (uni_type == 1) { // particle
                vec3 eye_position = (uni_model_view_inv * vec4(0, 0, 0, 1)).xyz();
                vec3 direction = normalize(in_pos - eye_position);
                float h = dot(-eye_position, direction);
                float w = length(cross(-eye_position, direction));
                if (w > uni_radius) {
                    discard();
                }
                float depth = h + sqrt(uni_radius * uni_radius - w * w);
                vec3 pos = eye_position + direction * depth;
                vec4 frag_pos = uni_projection * uni_view * uni_model * vec4(pos, 1);
                gl_FragDepth = (frag_pos.z / frag_pos.w * 0.5f + 0.5f);
            }
        }
    };

    shader_vert(DrawPrimitiveVert) {
        shader_in(location = 0, vec3, in_vert);
        shader_in(location = 1, vec3, in_normal);

        shader_out(location = 0, vec3, out_pos);
        shader_out(location = 1, vec3, out_normal);
        shader_out(location = 2, vec4, out_position_light);

        uniform(mat4, uni_projection);
        uniform(mat4, uni_view);
        uniform(mat4, uni_model);
        uniform(vec3, uni_size);

        void main() {
            out_pos = uni_size * (in_vert - 0.5f);
            gl_Position = uni_projection * uni_view * uni_model * vec4(out_pos, 1);
            out_normal = (uni_model * vec4(in_normal, 0)).xyz();
        }
    };

    shader_frag(DrawPrimitiveFrag) {
        shader_in(location = 0, vec3, in_pos);
        shader_in(location = 1, vec3, in_normal);

        shader_out(location = 0, vec4, out_color, lit::gl::Attachment::RGBA);
        shader_out(depth_any, float, gl_FragDepth, lit::gl::Attachment::DepthComponent);

        uniform(mat4, uni_projection);
        uniform(mat4, uni_view);
        uniform(mat4, uni_model);

        uniform(mat4, uni_projection_light);
        uniform(mat4, uni_view_light);

        uniform(mat4, uni_model_view_inv);

        uniform(int, uni_type);
        uniform(float, uni_radius);

        uniform(sampler2D, uni_shadow_map);

        const vec3 light_dir = normalize(-vec3(-4.0, 10.0, 3.0));

        float shadow_calc(vec3 pos)
        {
            vec4 frag_coord_light = uni_projection_light * uni_view_light * uni_model * vec4(pos, 1);
            vec3 proj_coords = frag_coord_light.xyz() / frag_coord_light.w;
            proj_coords = proj_coords * 0.5 + 0.5;
            float closestDepth = texture(uni_shadow_map, proj_coords.xy(), 0).r;
            float currentDepth = proj_coords.z;
            float bias = max(0.00005f * (1.0f - dot(in_normal, light_dir)), 0.000005f) * 0.1f;
            float shadow = currentDepth - bias > closestDepth  ? 1.0f : 0.0f;
            return shadow;
        }

        void main() {
            if (uni_type == 0) { // box
                float shadow = clamp(-dot(light_dir, in_normal), 0.0f, 1.0f);

                out_color = vec4(vec3(1.0f) * shadow, 1.0f);
                out_color = 0.2f + out_color * 0.8f * (1.0f - shadow_calc(in_pos));

                gl_FragDepth = gl_FragCoord.z;
            }
            if (uni_type == 1) { // particle
                vec3 eye_position = (uni_model_view_inv * vec4(0, 0, 0, 1)).xyz();
                vec3 direction = normalize(in_pos - eye_position);
                float h = dot(-eye_position, direction);
                float w = length(cross(-eye_position, direction));
                if (w > uni_radius) {
                    discard();
                }
                float depth = h - sqrt(uni_radius * uni_radius - w * w);
                vec3 pos = eye_position + direction * depth;
                vec3 normal = pos / uni_radius;
                float shadow = clamp(-dot(light_dir, normal), 0.0f, 1.0f);

                out_color = vec4(vec3(1.0f) * shadow, 1.0f);
                out_color = 0.2f + out_color * 0.8f * (1.0f - shadow_calc(pos));

                vec4 frag_pos = uni_projection * uni_view * uni_model * vec4(pos, 1);
                gl_FragDepth = (frag_pos.z / frag_pos.w * 0.5f + 0.5f);
            }

        }
    };

}
