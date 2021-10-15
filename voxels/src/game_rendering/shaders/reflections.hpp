#pragma once

#include <lit/rendering/cpp_shader.hpp>
#include <glm/vector_relational.hpp>

namespace lit::voxels {

    using namespace glm;
    using namespace rendering;

    shader_vert(ReflectionsVert) {
        shader_in(location = 0, vec3, in_vert);

        void main() {
            gl_Position = vec4(in_vert * 2.0f - 1.0f, 1);
        }
    };

    shader_frag(ReflectionsFrag) {
        shader_out(location = 0, vec4, out_reflection, gl::Attachment::RGB16);

        uniform(sampler2D, uni_depth);
        uniform(sampler2D, uni_normal);
        uniform(sampler2D, uni_direction);
        uniform(sampler2D, uni_position);
        uniform(sampler2D, uni_material);

        uniform(samplerCube, uni_cubemap);

        //uniform(sampler2D, uni_blue_noise);

        //uniform(int, uni_seed);

        //uniform(vec3, uni_light_color);
        //uniform(int, uni_light_type); // 0 - AO, 1 - global directional, 2 - spot/area
        //uniform(vec3, uni_light_pos); // direction for type 1, and position in shadow coordinates for type 2
        //uniform(float, uni_radius);

        struct ShadowObjectData : UniformHolder {
            uniform_field(usampler3D, lodSampler);
            uniform_field(ivec3, dims);
            uniform_field(int, maxLod);
        };

        uniform_struct(ShadowObjectData, shadowObject);

        struct CastResult {
            bool hit; // has hit any non-zero voxel?
            vec3 pos; // hit position in object-space
            ivec3 cell; // coordinates f the voxel that was hit
            float depth; // distance the ray traveled before the hit
            ivec3 normal; // normal
        };

        ivec3 ApplyInverse(ivec3 cell, ivec3 dims, ivec3 inversed) {
            return (cell * (1 - inversed) + ((dims - 1) - cell) * inversed);
        }

        vec3 ApplyInverse(vec3 pos, ivec3 dims, ivec3 inversed) {
            return pos + inversed * (dims - pos * 2);
        }

        bool HasVoxel(ShadowObjectData object, int level, ivec3 cell) {
            cell >>= level;
            int bit = (cell.x & 1) | ((cell.y & 1) << 1) | ((cell.z & 1) << 2);
            return bool((texelFetch(object.lodSampler, cell >> 1, level).r >> bit) & 1);
        }

        bool HasVoxelFast(ShadowObjectData object, int level, ivec3 cell) {
            return texelFetch(object.lodSampler, cell >> level, level - 1).r > 0;
        }

        int LodMask(int lod) {
            return ~((1 << lod) - 1);
        }

        bool CanLodUp(ivec3 cell, int lod, int maxLod) {
            return lod < maxLod && any(bvec3((cell & LodMask(lod)) == cell));
        }

        CastResult RayCast(ShadowObjectData object, vec3 ray_origin, vec3 ray_direction, int max_iterations) {
            // ray_direction should be positive, inverse axes if needed
            ivec3 signs = ivec3(sign(ray_direction + 1e-4f));
            ivec3 axes_inversed = (1 - signs) >> 1;

            ray_origin = ApplyInverse(ray_origin, object.dims, axes_inversed);
            ray_direction = normalize(abs(ray_direction) + 1e-6f);

            vec3 ray_direction_inversed = 1.0f / ray_direction; // to speed up division
            vec3 time = vec3(0); // time when we can hit a plane (Y-0-Z, X-0-Z, Y-0-X planes)
            vec3 shifted_ray_origin = ray_origin; // ray origin is shifted each step to reduce floating point errors
            ivec3 cell = ivec3(floor(ray_origin));
            int lod = 0;
            bool hit = false;

            int iteration = 0;
            for (; iteration < max_iterations && all(lessThan(cell, object.dims)); iteration++) {
                ivec3 cell_real = ApplyInverse(cell, object.dims, axes_inversed);

                while (lod > 0 && HasVoxelFast(object, lod, cell_real)) lod--;
                if (lod == 0 && (hit = HasVoxel(object, lod, cell_real))) break;

                time = ((cell | ((1 << lod) - 1)) + 1 - shifted_ray_origin) * ray_direction_inversed;
                shifted_ray_origin += min(time.x, min(time.y, time.z)) * ray_direction;

                cell = ivec3(shifted_ray_origin);

                if (CanLodUp(cell, lod, object.maxLod)) lod++;
            }

            CastResult res;
            res.hit = hit;
            res.cell = ApplyInverse(cell, object.dims, axes_inversed);
            res.depth = dot(shifted_ray_origin - ray_origin, ray_direction);
            res.pos = ApplyInverse(shifted_ray_origin, object.dims, axes_inversed);
            res.normal = (iteration > 0 ?
                          ivec3(step(time.xyz(), time.yzx()) * step(time.xyz(), time.zxy())) :
                          ivec3(step(ray_origin.xyz(), ray_origin.yzx()) * step(ray_origin.xyz(), ray_origin.zxy())))
                         * (2 * axes_inversed - 1);

            return res;
        }

        /*const float g = 1.22074408460575947536;
        const vec3 G = vec3(1 / g, 1 / (g * g), 1 / (g * g * g));

        vec3 getNoise(int seed) {
            ivec2 texel = ivec2(mod(gl_FragCoord.xy(), 1024.0f));
            return mod(texelFetch(uni_blue_noise, texel, 0).xyz() + G * float(seed), 1.0f);
        }

        vec3 getRandomDirection(int seed) {
            return normalize(getNoise(seed) * 2.0f - 1.0f);
        }

        vec3 getRandomDirectionByNormal(int seed, vec3 normal) {
            vec3 direction = getRandomDirection(seed);
            if (dot(normal, direction) < 0.0f) {
                direction = reflect(direction, normal);
            }
            return direction;
        }*/

        vec3 try_sample_reflection() {
            ivec2 coord = ivec2(gl_FragCoord.xy());
            vec3 normal = texelFetch(uni_normal, coord, 0).xyz();
            vec3 direction = texelFetch(uni_direction, coord, 0).xyz();
            vec3 reflected = reflect(direction, normal);
            vec3 origin = texelFetch(uni_position, coord, 0).xyz() + normal * 1e-3;

            CastResult mainCast = RayCast(shadowObject, origin, reflected, 196);

            if (mainCast.hit)
                return vec3(0);
            return texture(uni_cubemap, reflected).xyz() * texelFetch(uni_material, coord, 0).x;
        }

        void main() {
            if (texelFetch(uni_depth, ivec2(gl_FragCoord.xy()), 0).x > 0.999f) {
                out_reflection = vec4(0);
                return;
            }
            out_reflection = vec4(try_sample_reflection(), 1);
        }
    };

}