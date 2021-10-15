#pragma once
#define GLM_FORCE_SWIZZLE

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vector>
#include <optional>

using namespace glm;

struct Particle {
    vec3 position = vec3();
    vec3 velocity = vec3();
    vec3 velocity_pseudo = vec3();

    static constexpr float k_radius = 0.025f;
};

struct Box {
    vec3 size = vec3(0.1f, 0.1f, 0.1f);
    vec3 position = vec3();
    quat rotation = quat(1, 0, 0, 0);
};

struct Joint {
    Particle * p1;
    Particle * p2;
    float length = 1.0f;
    float spring = 1.0f;
    float damping = 1.0f;
};

struct World {
    vec3 gravity = vec3(0, -9.8, 0);

    std::vector<Particle> particles;
    std::vector<Joint> joints;
    std::vector<Box> boxes;

    void update(float delta_time);

    void solve(Particle * p1, Particle * p2, float delta_time);

    void solve(Box * box, Particle * p, float delta_time);

    static constexpr int k_grid_size = 100;
    std::vector<Particle*> grid[k_grid_size][k_grid_size][k_grid_size];
};

struct Collision {
    vec3 point = vec3();
    vec3 normal = vec3();
    float depth = 0.0;
};

std::optional<Collision> find_collision(const Box * box, const Particle * particle);

std::optional<Collision> find_collision(const Particle * p1, const Particle * p2);
