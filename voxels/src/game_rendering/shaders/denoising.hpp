#pragma once

#include <lit/rendering/cpp_shader.hpp>
#include <glm/vector_relational.hpp>

namespace lit::voxels {

    using namespace glm;
    using namespace rendering;

    shader_vert(DenoisingVert) {
        shader_in(location = 0, vec3, in_vert);

        void main() {
            gl_Position = vec4(in_vert * 2.0f - 1.0f, 1);
        }
    };

    shader_frag(DenoisingFrag) {
        shader_out(location = 0, vec4, out_denoised, gl::Attachment::RGB16);

        uniform(sampler2D, uni_input);

        uniform(sampler2D, uni_depth);
        uniform(sampler2D, uni_normal);
        uniform(sampler2D, uni_position);

        const float SIGMA = 3.0f;
        const float BSIGMA = 0.6f;
        const int MSIZE = 5;

        float normpdf(float x, float sigma) {
            return 0.39894f * exp(-0.5f * x * x / (sigma * sigma)) / sigma;
        }

        float normpdf3(vec3 v, float sigma) {
            return 0.39894f * exp(-0.5f * dot(v, v) / (sigma * sigma)) / sigma;
        }

        void main() {
            ivec2 coord = ivec2(gl_FragCoord.xy());
            vec3 light = texelFetch(uni_input, coord, 0).xyz();
            vec3 normal = texelFetch(uni_normal, coord, 0).xyz();
            vec3 position = texelFetch(uni_position, coord, 0).xyz();

            const int kSize = (MSIZE - 1) / 2;
            float kernel[5];
            vec3 final_colour = vec3(0.0);

            //create the 1-D kernel
            float Z = 0.0;
            for (int j = 0; j <= kSize; ++j) {
                kernel[kSize + j] = kernel[kSize - j] = normpdf(float(j), SIGMA);
            }

            vec3 light_near;
            vec3 normal_near;
            vec3 position_near;
            float factor;
            float bZ = 1.0f / normpdf(0.0, BSIGMA);
            //read out the texels
            for (int i = -kSize; i <= kSize; ++i) {
                for (int j = -kSize; j <= kSize; ++j) {
                    light_near = texelFetch(uni_input, coord + ivec2(i, j), 0).xyz();
                    normal_near = texelFetch(uni_normal, coord + ivec2(i, j), 0).xyz();
                    position_near = texelFetch(uni_position, coord + ivec2(i, j), 0).xyz();
                    factor = normpdf3(normal_near - normal, BSIGMA) * normpdf3(position_near - position, BSIGMA) * bZ *
                             kernel[kSize + j] * kernel[kSize + i];
                    Z += factor;
                    final_colour += factor * light_near;
                }
            }

            out_denoised = vec4(final_colour / Z, 1.0);
            //out_position = texelFetch(uni_position, ivec2(gl_FragCoord.xy()), 0);
        }
    };

}