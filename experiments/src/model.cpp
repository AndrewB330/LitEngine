#include <glm/gtx/norm.hpp>
#include "model.hpp"

constexpr float radius_cube = Particle::k_radius * Particle::k_radius * Particle::k_radius;

ivec3 pos_to_grid_pos(vec3 pos) {
    return ivec3(floor(pos / (Particle::k_radius * 2))) + World::k_grid_size / 2;
}

bool check_grid_pos(ivec3 grid_pos) {
    return !(any(greaterThanEqual(grid_pos, ivec3(World::k_grid_size))) || any(lessThan(grid_pos, ivec3(0))));
}

void World::update(float delta_time) {
    // Integrate
    for (auto &p : particles) {
        p.position += (p.velocity + p.velocity_pseudo) * delta_time;
        p.velocity += gravity * delta_time;
        p.velocity_pseudo = vec3(); // clear pseudo velocity
    }

    for (auto &a : grid)
        for (auto &b : a)
            for (auto &c : b)
                c.clear();

    /*for (auto &p : particles) {
        ivec3 grid_pos = pos_to_grid_pos(p.position);
        if (!check_grid_pos(grid_pos))
            continue;
        grid[grid_pos.x][grid_pos.y][grid_pos.z].push_back(&p);
    }

    for (auto &p1 : particles) {
        ivec3 grid_pos = pos_to_grid_pos(p1.position);
        if (!check_grid_pos(grid_pos)) continue;
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dz = -1; dz <= 1; dz++) {
                    ivec3 next_grid_pos = grid_pos + ivec3(dx, dy, dz);
                    if (!check_grid_pos(next_grid_pos)) continue;
                    for (auto &p2 : grid[next_grid_pos.x][next_grid_pos.y][next_grid_pos.y]) {
                        //if (dx == 0 && dy == 0 && dz == 0 && &p1 >= p2) continue;
                        solve(&p1, p2, delta_time);
                    }
                }
            }
        }
    }*/

    // Resolve collisions
    for (auto &p1 : particles) {
        for (auto &p2 : particles) {
            if (&p1 > &p2) continue;
            //solve(&p1, &p2, delta_time);
        }
    }

    for (auto &p : particles) {
        for (auto &b : boxes) {
            solve(&b, &p, delta_time);
        }
    }

    for (auto &joint : joints) {
        float cur_length = length(joint.p1->position - joint.p2->position);
        vec3 direction = normalize(joint.p2->position - joint.p1->position);
        float difference = clamp(joint.length - cur_length, -0.8f, 0.8f);
        float velocity_projected = dot(direction, joint.p1->velocity - joint.p2->velocity);
        float force = difference * joint.spring + velocity_projected * joint.damping * 0.1f;

        joint.p1->velocity -= force * delta_time * direction / radius_cube;
        joint.p2->velocity += force * delta_time * direction / radius_cube;
    }
}

void World::solve(Particle *p1, Particle *p2, float delta_time) {
    auto collision = find_collision(p1, p2);
    if (!collision.has_value()) return;
    float velocity_projected = dot(collision->normal, p1->velocity - p2->velocity);
    float impulse = velocity_projected * 0.5f;
    if (impulse < 0.0f) return;

    p1->velocity -= impulse * collision->normal;
    p2->velocity += impulse * collision->normal;

    p1->velocity_pseudo -= collision->normal * collision->depth / delta_time * 0.1f;
    p2->velocity_pseudo += collision->normal * collision->depth / delta_time * 0.1f;
}

void World::solve(Box *box, Particle *p, float delta_time) {
    auto collision = find_collision(box, p);
    if (!collision.has_value()) return;
    if (dot(p->velocity, collision->normal) > 0.0f) return;
    vec3 u = normalize(cross(collision->normal, p->velocity));
    vec3 v = cross(u, collision->normal);
    p->velocity -= 1.0f * dot(p->velocity, collision->normal) * collision->normal; // todo: constant
    p->velocity -= 0.1f * dot(p->velocity, u) * u + 0.1f * dot(p->velocity, v) * v;
    p->velocity_pseudo += collision->normal * collision->depth / delta_time * 0.1f; // todo: constant
}

std::optional<Collision> find_collision(const Box *box, const Particle *particle) {
    // Fast check (against box bounding sphere)
    float dist_sqr = distance2(box->position, particle->position);
    float radius_sum = length(box->size) + Particle::k_radius;
    if (dist_sqr > radius_sum * radius_sum)
        return std::nullopt;

    auto rotation_inv = quat(-box->rotation.w, box->rotation.x, box->rotation.y, box->rotation.z);
    vec3 particle_in_box_space = glm::rotate(rotation_inv, particle->position - box->position);
    vec3 nearest_in_box_space = glm::clamp(particle_in_box_space, -box->size * 0.5f, box->size * 0.5f);
    float dist = distance(particle_in_box_space, nearest_in_box_space);
    if (dist > Particle::k_radius || dist < 0.001f)
        return std::nullopt;

    vec3 nearest = glm::rotate(box->rotation, nearest_in_box_space) + box->position;

    Collision collision;
    collision.depth = Particle::k_radius - dist;
    collision.normal = (particle->position - nearest) / dist;
    collision.point = nearest;
    return collision;

}

std::optional<Collision> find_collision(const Particle *p1, const Particle *p2) {
    if (p1 == p2)
        return std::nullopt;

    // Fast check
    float dist_sqr = distance2(p1->position, p2->position);
    constexpr float double_radius = Particle::k_radius * 2;
    if (dist_sqr > double_radius * double_radius || dist_sqr < 0.00001f)
        return std::nullopt;

    float dist = sqrtf(dist_sqr);
    Collision collision;
    collision.depth = double_radius - dist;
    collision.normal = (p2->position - p1->position) / dist;
    collision.point = collision.normal * (Particle::k_radius - collision.depth * 0.5f) + p1->position;
    return collision;
}
