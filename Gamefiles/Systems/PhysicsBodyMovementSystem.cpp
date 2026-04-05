// PhysicsBodyMovementSystem.cpp

#include "PhysicsBodyMovementSystem.h"

#include <algorithm>
#include <vector>

void PhysicsBodyMovementSystem::update (Registry & registry, float deltatime) {


    for (Entity entity : registry.view<comp::Transform, comp::Velocity, comp::PhysicsBody>()) {      // For each iteration of Entity


        // To get to this point, we have confirmed this entity has both VELOCITY and TRANSFORM, so should be valid to get component
        // Go ahead and grab struct references to the components we want to manipulate so we can edit them directly

        comp::Transform & transform = registry.get_component<comp::Transform>(entity);
        comp::Velocity & velocity = registry.get_component<comp::Velocity>(entity);
        comp::PhysicsBody & body = registry.get_component<comp::PhysicsBody>(entity);
        transform.previous_position = transform.position;
        
        // Transform

        Vec2 to_move = {velocity.magnitude.x * deltatime, velocity.magnitude.y * deltatime};

        int direction_x = (velocity.magnitude.x > 0.0f) ? 1 : (velocity.magnitude.x < 0.0f) ? -1 : 0;
        int direction_y = (velocity.magnitude.y > 0.0f) ? 1 : (velocity.magnitude.y < 0.0f) ? -1 : 0;

        const int HORZ_RAY_SPACING = 10;
        const int HORZ_RAY_COUNT = std::max(2, (int)std::ceil(body.size.y / (float)HORZ_RAY_SPACING));
        std::vector<float> horz_ray_dists(HORZ_RAY_COUNT, std::fabs(to_move.x));

        const int VERT_RAY_SPACING = 10;
        const int VERT_RAY_COUNT = std::max(2, (int)std::ceil(body.size.x / (float)VERT_RAY_SPACING));
        std::vector<float> vert_ray_dists(VERT_RAY_COUNT, std::fabs(to_move.y));


        for (TileGrid & layer : scene.tile_layers) {

            if (direction_x == 0) {
                continue;
            }

            for (int i = 0; i < HORZ_RAY_COUNT; i++) {
                float ray_y = (transform.position.y - body.size.y / 2.0f) + ((body.size.y * i) / (float)(HORZ_RAY_COUNT - 1));
                Vec2 ray_start = (direction_x > 0)
                    ? Vec2{transform.position.x + body.size.x / 2.0f, ray_y}
                    : Vec2{transform.position.x - body.size.x / 2.0f, ray_y};
                Vec2 ray_step = (direction_x > 0) ? Vec2{1.0f, 0.0f} : Vec2{-1.0f, 0.0f};
                horz_ray_dists[i] = layer.raycast(scene, ray_start, ray_step, (int)std::ceil(std::fabs(to_move.x)));
                
            }

            float clamped_dist = std::fabs(to_move.x);
            bool inside_wall = false;

            for (float dist : horz_ray_dists) {
                if (dist == -1.0f) {
                    inside_wall = true;
                    break;
                }
                clamped_dist = std::min(clamped_dist, dist);
            }
        


            if (inside_wall || clamped_dist < std::fabs(to_move.x)) {
                float safe_dist = inside_wall ? 0.0f : std::max(0.0f, clamped_dist - body.skin);
                to_move.x = safe_dist * (float)direction_x;
                velocity.magnitude.x = 0.0f;
            }
        }

        transform.position.x = transform.position.x + to_move.x;

        // Resolve Y after X so the body can slide along blocking walls.
        for (TileGrid & layer : scene.tile_layers) {
            if (direction_y == 0) {
                continue;
            }

            for (int i = 0; i < VERT_RAY_COUNT; i++) {
                float ray_x = (transform.position.x - body.size.x / 2.0f) + ((body.size.x * i) / (float)(VERT_RAY_COUNT - 1));
                Vec2 ray_start = (direction_y > 0)
                    ? Vec2{ray_x, transform.position.y + body.size.y / 2.0f}
                    : Vec2{ray_x, transform.position.y - body.size.y / 2.0f};
                Vec2 ray_step = (direction_y > 0) ? Vec2{0.0f, 1.0f} : Vec2{0.0f, -1.0f};
                vert_ray_dists[i] = layer.raycast(scene, ray_start, ray_step, (int)std::ceil(std::fabs(to_move.y)));
                
            }

            float clamped_dist = std::fabs(to_move.y);
            bool inside_wall = false;

            for (float dist : vert_ray_dists) {
                if (dist == -1.0f) {
                    inside_wall = true;
                    break;
                }
                clamped_dist = std::min(clamped_dist, dist);
            }

            if (inside_wall || clamped_dist < std::fabs(to_move.y)) {
                float safe_dist = inside_wall ? 0.0f : std::max(0.0f, clamped_dist - body.skin);
                to_move.y = safe_dist * (float)direction_y;
                velocity.magnitude.y = 0.0f;
            }
        }
        
        transform.position.y = transform.position.y + to_move.y;


    }

}