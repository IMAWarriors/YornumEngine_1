// PhysicsBodyMovementSystem.cpp

#include "PhysicsBodyMovementSystem.h"

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

        static const int HORZ_RAY_SPACING = 10;
        static const int HORZ_RAY_COUNT = std::max(2, (int)(body.size.y / HORZ_RAY_SPACING));

        float horz_ray_dists[HORZ_RAY_COUNT+1];

        


        for (TileGrid & layer : scene.tile_layers) {
            if (direction_x > 0) {
                for (int i = 0; i < HORZ_RAY_COUNT-1; i++) {
                    horz_ray_dists[i] = layer.raycast(scene, {transform.position.x + body.size.x / 2, (transform.position.y - body.size.y / 2) + i * HORZ_RAY_SPACING}, {1.0, 0}, (int)std::ceil(std::fabs(to_move.x)));
                }
                horz_ray_dists[HORZ_RAY_COUNT] = layer.raycast(scene, {transform.position.x + body.size.x / 2, transform.position.y + body.size.y / 2}, {1.0, 0}, (int)std::ceil(std::fabs(to_move.x)));
            } else if (direction_x < 0) {
                for (int i = 0; i < HORZ_RAY_COUNT-1; i++) {
                    horz_ray_dists[i] = layer.raycast(scene, {transform.position.x - body.size.x / 2, (transform.position.y - body.size.y / 2) + i * HORZ_RAY_SPACING}, {-1.0, 0}, (int)std::ceil(std::fabs(to_move.x)));
                }
                horz_ray_dists[HORZ_RAY_COUNT] = layer.raycast(scene, {transform.position.x - body.size.x / 2, transform.position.y + body.size.y / 2}, {-1.0, 0}, (int)std::ceil(std::fabs(to_move.x)));
            } else {


                break;
            }

            bool in_wall = false;

            for (float dist : horz_ray_dists) {
                if (dist < std::fabs(to_move.x)) {
                    // We cannot move full desired distance for some reason

                    if (dist == -1) {
                        // We are inside of a wall
                        in_wall = true;

                        auto is_solid = [&](Vec2 pos) {
                            int col = layer.translate_world_x_col(pos.x);
                            int row = layer.translate_world_y_row(pos.y);
                            Tile tile = layer.get_tile(col, row);

                            if (tile.atlas_idx < 0 || tile.tile_idx < 0) return false;

                            return scene.loaded_atlases[tile.atlas_idx]
                                .tile_data[tile.tile_idx]
                                .collision_data == CollisionType::COLL_FULL_SOLID;
                        };

                        // Check center of body
                        Vec2 center = transform.position;

                        // If inside wall → push out horizontally
                        if (is_solid(center)) {
                            float push_dist = 0.0f;
                            const float MAX_PUSH = body.size.x;

                            // Try pushing right first
                            while (push_dist < MAX_PUSH) {
                                center.x += 1.0f;
                                if (!is_solid(center)) {
                                    transform.position.x = center.x;
                                    break;
                                }
                                push_dist += 1.0f;
                            }

                            // If still stuck, try left
                            if (push_dist >= MAX_PUSH) {
                                center = transform.position;
                                push_dist = 0.0f;

                                while (push_dist < MAX_PUSH) {
                                    center.x -= 1.0f;
                                    if (!is_solid(center)) {
                                        transform.position.x = center.x;
                                        break;
                                    }
                                    push_dist += 1.0f;
                                }
                            }
                        }



                        break;

                    } else {
                        // We are not inside of a wall, we just can't move our full desired distance

                        int move_axis = (to_move.x > 0) ? 1 : -1;
                        to_move.x = (dist - body.skin) * move_axis;
                    }
                    
                    velocity.magnitude.x = 0;
                }
            }

            


        }
        






        transform.position.x = transform.position.x + to_move.x;
        transform.position.y = transform.position.y + to_move.y;


    }

}