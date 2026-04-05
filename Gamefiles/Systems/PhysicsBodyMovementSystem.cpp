// PhysicsBodyMovementSystem.cpp

#include "PhysicsBodyMovementSystem.h"

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
        int direction_y = (velocity.magnitude.y > 0.0f) ? 1 : -1;

        const int HORZ_RAY_SPACING = 10;
        const int HORZ_RAY_COUNT = std::max(2, (int)std::ceil(body.size.y / (float)HORZ_RAY_SPACING));
        std::vector<float> horz_ray_dists(HORZ_RAY_COUNT, std::fabs(to_move.x));

        const int VERT_RAY_SPACING = 10;
        const int VERT_RAY_COUNT = std::max(2, (int)std::ceil(body.size.x / (float)VERT_RAY_SPACING));
        std::vector<float> vert_ray_dists(VERT_RAY_COUNT, std::fabs(to_move.y));


        for (TileGrid & layer : scene.tile_layers) {

            // HORIZONTAL COLLISION

            // Cast horizontal rays

            if (direction_x > 0) {
                for (int i = 0; i < HORZ_RAY_COUNT; i++) {
                    float ray_y = (transform.position.y - body.size.y / 2.0f) + ((body.size.y * i) / (float)(HORZ_RAY_COUNT - 1));
                    horz_ray_dists[i] = layer.raycast(scene, {transform.position.x + body.size.x / 2.0f, ray_y}, {1.0f, 0.0f}, (int)std::ceil(std::fabs(to_move.x)));
                }
                
            } else if (direction_x < 0) {
                for (int i = 0; i < HORZ_RAY_COUNT; i++) {
                    float ray_y = (transform.position.y - body.size.y / 2.0f) + ((body.size.y * i) / (float)(HORZ_RAY_COUNT - 1));
                    horz_ray_dists[i] = layer.raycast(scene, {transform.position.x - body.size.x / 2.0f, ray_y}, {-1.0f, 0.0f}, (int)std::ceil(std::fabs(to_move.x)));
                }

            } else {
                break;
            }

            // Set move to horizontal ray calc dist min clamp to collision

            for (float dist : horz_ray_dists) {
                if (dist < std::fabs(to_move.x)) {
                    // We cannot move full desired distance for some reason

                    if (dist == -1) {
                        // We are inside of a wall
                        to_move.x = 0.0f;
                        break;

                    } else {
                        // We are not inside of a wall, we just can't move our full desired distance

                        int move_axis = (to_move.x > 0) ? 1 : -1;
                        to_move.x = (dist - body.skin) * move_axis;
                    }
                    
                    velocity.magnitude.x = 0;
                }
            }
        


            // VERTICAL COLLISION

            // Cast vertical rays

            if (direction_y > 0) {
                for (int i = 0; i < VERT_RAY_COUNT; i++) {
                    float ray_x = (transform.position.x - body.size.x / 2.0f) + ((body.size.x * i) / (float)(VERT_RAY_COUNT - 1));
                    vert_ray_dists[i] = layer.raycast(scene, {ray_x, transform.position.x + body.size.x / 2.0f}, {0.0f, 1.0f}, (int)std::ceil(std::fabs(to_move.y)));
                }
                
            } else if (direction_x < 0) {
                for (int i = 0; i < VERT_RAY_COUNT; i++) {
                    float ray_x = (transform.position.x - body.size.y / 2.0f) + ((body.size.x * i) / (float)(VERT_RAY_COUNT - 1));
                    vert_ray_dists[i] = layer.raycast(scene, {ray_x, transform.position.x - body.size.x / 2.0f}, {0.0f, -1.0f}, (int)std::ceil(std::fabs(to_move.y)));
                }

            } else {
                break;
            }

            // Set move to vertical ray calc dist min clamp to collision

            for (float dist : vert_ray_dists) {
                if (dist < std::fabs(to_move.y)) {
                    // We cannot move full desired distance for some reason

                    if (dist == -1) {
                        // We are inside of a wall
                        to_move.y = 0.0f;
                        break;

                    } else {
                        // We are not inside of a wall, we just can't move our full desired distance

                        int move_axis = (to_move.y > 0) ? 1 : -1;
                        to_move.y = (dist - body.skin) * move_axis;
                    }
                    
                    velocity.magnitude.y = 0;
                }
            }


        
        
        
        
        
        
        }
        






        transform.position.x = transform.position.x + to_move.x;
        transform.position.y = transform.position.y + to_move.y;


    }

}