#pragma once

#include <lit/rendering/cpp_shader.hpp>
#include <glm/vector_relational.hpp>
#include <glm/glm.hpp>

namespace lit::voxels::shaders {

    using namespace glm;
    using namespace rendering;

    shader_vert(RayTracingVert) {
        shader_in(location = 0, vec3, in_vert);

        shader_out(location = 0, vec3, out_pos);

        uniform(mat4, ProjectionMatrix);
        uniform(mat4, ViewMatrix);
        uniform(mat4, ModelMatrix);
        uniform(ivec3, VoxelObjectDims);

        void main() {
            gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(VoxelObjectDims * in_vert, 1);
            out_pos = VoxelObjectDims * in_vert;
        }
    };

    shader_frag(RayTracingFrag) {
        shader_in(location = 0, vec3, in_pos);

        shader_out(location = 0, vec4, out_color, gl::Attachment::RGBA);
        shader_out(location = 1, vec3, out_material, gl::Attachment::RGB);
        shader_out(location = 2, vec3, out_normal, gl::Attachment::RGB16);
        shader_out(location = 3, vec3, out_frame_offset, gl::Attachment::RGB16);
        shader_out(location = 4, vec3, out_shadow_position, gl::Attachment::RGB32);
        shader_out(location = 5, vec3, out_direction, gl::Attachment::RGB16);
        shader_out(depth_any, float, gl_FragDepth, gl::Attachment::DepthComponent);

        struct VoxelObjectData : UniformHolder {
            uniform_field(usampler3D, lodSampler);
            uniform_field(usampler3D, dataSampler);
            uniform_field(float, voxelSize);
            uniform_field(ivec3, dims);
            uniform_field(int, maxLod);
            uniform_field(mat4, transform);
            uniform_field(mat4, transformInv);
        };

        uniform_struct(VoxelObjectData, currentObject);

        uniform(mat4, shadowTransformInv);

        uniform(mat4, ModelViewMatrixInverse);

        uniform(mat4, ProjectionMatrixPrev);
        uniform(mat4, ViewMatrixPrev);
        uniform(mat4, ModelMatrixPrev);
        uniform(vec2, ViewportSize);

        uniform(float, zFar);

        uniform(sampler2D, uni_palette);

        struct CastResult {
            uint data;
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

        bool HasVoxel(VoxelObjectData object, int level, ivec3 cell) {
            cell >>= level;
            int bit = (cell.x & 1) | ((cell.y & 1) << 1) | ((cell.z & 1) << 2);
            return bool((texelFetch(object.lodSampler, cell >> 1, level).r >> bit) & 1);
        }

        bool HasVoxelFast(VoxelObjectData object, int level, ivec3 cell) {
            return texelFetch(object.lodSampler, cell >> level, level - 1).r > 0;
        }

        uint GetVoxel(VoxelObjectData object, ivec3 cell) {
            return texelFetch(object.dataSampler, cell, 0).r;
        }

        int LodMask(int lod) {
            return ~((1 << lod) - 1);
        }

        bool CanLodUp(ivec3 cell, int lod, int maxLod) {
            return lod < maxLod && any(bvec3((cell & LodMask(lod)) == cell));
        }

        CastResult RayCast(VoxelObjectData object, vec3 ray_origin, vec3 ray_direction, int max_iterations) {
            // ray_direction should be positive, inverse axes if needed
            ivec3 signs = ivec3(sign(ray_direction + 1e-4f));
            ivec3 axes_inversed = (1 - signs) >> 1;

            ray_origin = ApplyInverse(ray_origin, object.dims, axes_inversed);
            ray_direction = normalize(abs(ray_direction) + 1e-6f);

            vec3 ray_direction_inversed = 1.0f / ray_direction; // to speed up division
            vec3 time = vec3(0); // time when we can hit a plane (Y-0-Z, X-0-Z, Y-0-X planes)
            vec3 shifted_ray_origin = ray_origin; // ray origin is shifted each step to reduce floating point errors
            ivec3 cell = ivec3(floor(ray_origin));
            int lod = object.maxLod;
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
            res.data = GetVoxel(object, res.cell);
            res.depth = dot(shifted_ray_origin - ray_origin, ray_direction);
            res.pos = ApplyInverse(shifted_ray_origin, object.dims, axes_inversed);
            res.normal = (iteration > 0 ?
                          ivec3(step(time.xyz(), time.yzx()) * step(time.xyz(), time.zxy())) :
                          ivec3(step(ray_origin.xyz(), ray_origin.yzx()) * step(ray_origin.xyz(), ray_origin.zxy())))
                         * (2 * axes_inversed - 1);

            return res;
        }

        void main() {
            vec3 eye_position = (ModelViewMatrixInverse * vec4(0, 0, 0, 1)).xyz();
            vec3 ray_origin = in_pos;
            vec3 ray_direction = normalize(in_pos - eye_position);
            float initial_depth = length(ray_origin - eye_position);

            bool greater = all(greaterThanEqual(eye_position, vec3(-0.1f)));
            bool less = all(lessThanEqual(eye_position, currentObject.dims + vec3(0.1f)));
            if (greater && less) {
                ray_origin = eye_position;
                initial_depth = 0;
            } else {
                ray_origin += ray_direction * 1e-4f; // move ray origin a bit forward
            }

            CastResult mainCast = RayCast(currentObject, ray_origin, ray_direction, 256);

            if (!mainCast.hit) {
                out_normal = vec3(0);
                out_color = vec4(0);
                out_frame_offset = vec3(0);
                out_shadow_position = vec3(0);
                gl_FragDepth = 0.999999f;
                return;
            }

            out_normal = normalize(currentObject.transform * vec4(mainCast.normal, 0)).xyz();
            out_color = texelFetch(uni_palette, ivec2(mainCast.data - 1, 0), 0);
            gl_FragDepth = ((initial_depth + mainCast.depth) * currentObject.voxelSize) / zFar;

            if (mainCast.data == 73) {
                out_material = vec3(0.8, 0, 0);
            } else {
                out_material = vec3(0.05, 0, 0);
            }

            out_direction = (currentObject.transform * vec4(ray_direction, 0)).xyz();

            vec4 old_pos = ProjectionMatrixPrev * ViewMatrixPrev * ModelMatrixPrev * vec4(mainCast.pos, 1);
            out_frame_offset = vec3((old_pos.xy() / old_pos.w + 1.0f) * 0.5f - gl_FragCoord.xy() / ViewportSize, 0);

            out_shadow_position = (shadowTransformInv * currentObject.transform * vec4(mainCast.pos, 1)).xyz();
        }
    };

}