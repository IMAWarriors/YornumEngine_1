// PhysicsBodyMovementSystem.cpp

#include "PhysicsBodyMovementSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

static bool is_tiletype_collision (CollisionType colltype, bool include_semisol = false) {
    return colltype == CollisionType::COLL_FULL_SOLID ||
           (include_semisol && colltype == CollisionType::COLL_FULL_SEMISOLID);
}

static int get_tiletype_slope_dir (CollisionType colltype) {
    if (colltype == CollisionType::COLL_PSLOPE1_SOLID) return 1;
    if (colltype == CollisionType::COLL_NSLOPE1_SOLID) return -1;
    return 0;
}

static bool is_solid_or_slope (CollisionType colltype, bool include_semisol = false) {
    return is_tiletype_collision(colltype, include_semisol) || get_tiletype_slope_dir(colltype) != 0;
}

static float slope_surface_y (int tx, int ty, float sample_x, int slope_dir) {
    const float tile_size = (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS;
    const float local_x = std::clamp(sample_x - (tx * tile_size), 0.0f, tile_size);

    if (slope_dir > 0) {
        // Positive slope debug triangle rises to the right: bottom-left -> top-right.
        return (ty * tile_size) + (tile_size - local_x);
    }

    // Negative slope descends to the right: top-left -> bottom-right.
    return (ty * tile_size) + local_x;
}

static bool find_floor_at_x (Scene & scene, float sample_x, float bottom, float probe_down, float probe_up, float & out_floor_y) {
    const float tile_size = (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS;
    const int tx = (int)std::floor(sample_x / tile_size);
    const int tile_y_min = (int)std::floor((bottom - probe_up) / tile_size) - 1;
    const int tile_y_max = (int)std::floor((bottom + probe_down) / tile_size) + 1;

    bool found = false;
    float best_y = std::numeric_limits<float>::max();

    for (const TileGrid & layer : scene.tile_layers) {
        for (int ty = tile_y_min; ty <= tile_y_max; ++ty) {
            CollisionType coll = layer.get_tile_coll(scene, tx, ty);
            if (!is_solid_or_slope(coll, true)) continue;
            if (coll == CollisionType::COLL_FULL_SEMISOLID && IsKeyDown(KEY_M)) continue;

            float surface_y = ty * tile_size;
            const int slope_dir = get_tiletype_slope_dir(coll);
            if (slope_dir != 0) {
                surface_y = slope_surface_y(tx, ty, sample_x, slope_dir);
            }

    if (surface_y < bottom - probe_up || surface_y > bottom + probe_down) continue;

            if (!found || surface_y < best_y) {
                best_y = surface_y;
                found = true;
            }
        }
    }

    if (found) out_floor_y = best_y;
    return found;
}

static bool find_body_floor (Scene & scene, Vec2 pos, comp::PhysicsBody & body, float probe_down, float probe_up, float & out_floor_y) {
    const float half_w = body.size.x * 0.5f;
    const float half_h = body.size.y * 0.5f;
    const float bottom = pos.y + half_h;

    const float samples[] = {
        pos.x - half_w + body.skin + 1.0f,
        pos.x,
        pos.x + half_w - body.skin - 1.0f
    };

    bool found = false;
    float best_y = std::numeric_limits<float>::max();

    for (float sample_x : samples) {
        float floor_y = 0.0f;
        if (find_floor_at_x(scene, sample_x, bottom, probe_down, probe_up, floor_y)) {
            if (!found || floor_y < best_y) {
                best_y = floor_y;
                found = true;
            }
        }
    }

    if (found) out_floor_y = best_y;
    return found;
}

static void set_grounded (comp::PhysicsBody & body, Vec2 & vel) {
    if (vel.y >= 0.0f) vel.y = 0.0f;
    body.onSolidGround  = true;
    body.falling        = 0;
    body.vjump_window   = 0;
    body.lastWalljumpDir = 0;
    body.walljumpBuffer = 0;
    body.againstWall    = false;
}

static float try_move_x (Scene & scene, Vec2 pos, comp::PhysicsBody & body, float delta, Vec2 & vel, bool was_grounded, comp::InputState * input = nullptr) {
    if (std::abs(delta) < 0.0001f) return pos.x;

    const float tile_size = (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS;
    const float move = std::abs(delta);
    const int x_dir = (delta > 0.0f) ? 1 : -1;
    const float half_w = body.size.x * 0.5f;
    const float half_h = body.size.y * 0.5f;

    int tile_y_min = (int)std::floor((pos.y - half_h) / tile_size);
    int tile_y_max = (int)std::floor((pos.y + half_h - 0.001f) / tile_size);

    bool found_collision = false;
    float best_resolve_x = pos.x + delta;

    if (x_dir == 1) {
        const float leading_edge = pos.x + half_w;
        const float target_edge = leading_edge + move;
        int tile_x_min = (int)std::floor(leading_edge / tile_size);
        int tile_x_max = (int)std::floor((target_edge + 0.001f) / tile_size);

        for (const TileGrid & layer : scene.tile_layers)
        for (int tx = tile_x_min; tx <= tile_x_max; ++tx)
        for (int ty = tile_y_min; ty <= tile_y_max; ++ty) {
            CollisionType coll = layer.get_tile_coll(scene, tx, ty);
            const int slope_dir = get_tiletype_slope_dir(coll);
            if (!is_tiletype_collision(coll, false) && slope_dir >= 0) continue;
            float tile_left = tx * tile_size;
            if (tile_left < leading_edge - body.skin || tile_left > target_edge) continue;
            
            const float bottom = pos.y + half_h;
            const float tile_top = ty * tile_size;
            const float step_up = std::max(move + body.skin + 2.0f, 4.0f);
            if (is_tiletype_collision(coll, false) && was_grounded && tile_top >= bottom - step_up && tile_top <= bottom + body.skin) continue;

            float candidate = tile_left - half_w - body.skin;
            if (!found_collision || candidate < best_resolve_x) { best_resolve_x = candidate; found_collision = true; }
        }
    } else {
        const float leading_edge = pos.x - half_w;
        const float target_edge = leading_edge - move;
        int tile_x_min = (int)std::floor((target_edge - 0.001f) / tile_size);
        int tile_x_max = (int)std::floor(leading_edge / tile_size);

        for (const TileGrid & layer : scene.tile_layers)
        for (int tx = tile_x_min; tx <= tile_x_max; ++tx)
        for (int ty = tile_y_min; ty <= tile_y_max; ++ty) {
            CollisionType coll = layer.get_tile_coll(scene, tx, ty);
            const int slope_dir = get_tiletype_slope_dir(coll);
            if (!is_tiletype_collision(coll, false) && slope_dir <= 0) continue;
            float tile_right = (tx + 1) * tile_size;
            if (tile_right > leading_edge + body.skin || tile_right < target_edge) continue;
            
            const float bottom = pos.y + half_h;
            const float tile_top = ty * tile_size;
            const float step_up = std::max(move + body.skin + 2.0f, 4.0f);
            if (is_tiletype_collision(coll, false) && was_grounded && tile_top >= bottom - step_up && tile_top <= bottom + body.skin) continue;

            float candidate = tile_right + half_w + body.skin;
            if (!found_collision || candidate > best_resolve_x) { best_resolve_x = candidate; found_collision = true; }
        }
    }
    if (found_collision) {
        vel.x = 0.0f;
        if (input != nullptr && input->horz_axis == x_dir) {
            body.wallPush = x_dir;
            body.lastWallPush = x_dir;
            body.walljumpWindow = 6;
            body.againstWall = true;
        }
    }

    return best_resolve_x;
}


static float try_move_y (Scene & scene, Vec2 pos, comp::PhysicsBody & body, float delta, Vec2 & vel) {
    const float tile_size = (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS;
    const float half_h = body.size.y * 0.5f;
    
    if (std::abs(delta) < 0.0001f) {
        float floor_y = 0.0f;
        if (find_body_floor(scene, pos, body, 1.5f + body.skin, body.skin, floor_y)) {
            set_grounded(body, vel);
        } else {
            body.onSolidGround = false;
        }
        return pos.y;
    }

    float target_y = pos.y + delta;
    
        if (delta > 0.0f) {
        float floor_y = 0.0f;
        if (find_body_floor(scene, pos, body, delta + body.skin, body.skin, floor_y)) {
            target_y = floor_y - half_h - body.skin;
            set_grounded(body, vel);
        } else {
            body.onSolidGround = false;
            body.falling++;
        }
    } else {
        const float half_w = body.size.x * 0.5f;
        const float top = pos.y - half_h;
        const float target_top = top + delta;
        int tile_y_min = (int)std::floor((target_top - 0.001f) / tile_size);
        int tile_y_max = (int)std::floor(top / tile_size);
        int tile_x_min = (int)std::floor((pos.x - half_w) / tile_size);
        int tile_x_max = (int)std::floor((pos.x + half_w - 0.001f) / tile_size);
        
        bool found_collision = false;
        float best_y = target_y;
        for (const TileGrid & layer : scene.tile_layers)
        for (int tx = tile_x_min; tx <= tile_x_max; ++tx)
        for (int ty = tile_y_min; ty <= tile_y_max; ++ty) {
            if (!is_tiletype_collision(layer.get_tile_coll(scene, tx, ty), false)) continue;
            float tile_bottom = (ty + 1) * tile_size;
            if (tile_bottom > top + body.skin || tile_bottom < target_top) continue;
            float candidate = tile_bottom + half_h + body.skin;
            if (!found_collision || candidate > best_y) { best_y = candidate; found_collision = true; }
        }

        if (found_collision) {
            target_y = best_y;
            vel.y = 0.0f;
            body.vjump_window = 0;
        } else {
            body.onSolidGround = false;
            body.falling++;
        }
    }

    return target_y;
}

void PhysicsBodyMovementSystem::update (Registry & registry, float deltatime) {
    for (Entity entity : registry.view<comp::Transform, comp::Velocity, comp::PhysicsBody>()) {
        Vec2 & previous_position = registry.get_component<comp::Transform>(entity).previous_position;
        Vec2 & position = registry.get_component<comp::Transform>(entity).position;
        Vec2 & velocity = registry.get_component<comp::Velocity>(entity).magnitude;
        comp::PhysicsBody & body = registry.get_component<comp::PhysicsBody>(entity);

        previous_position = position;
        const bool was_grounded = body.onSolidGround;
        body.onSolidGround = false;

        if (body.walljumpWindow > 0 && body.wallPush == 0 && body.falling > 3) body.walljumpWindow--;
        if (body.walljumpWindow <= 0) { body.walljumpWindow = 0; body.againstWall = false; }

        if (!was_grounded && body.gravitous) {
            if (body.wallPush != 0 && velocity.y > 0.0f && body.falling > 5) {
                if (velocity.y > 250.0f) velocity.y = 250.0f;
                velocity.y += ((150.0f - velocity.y) * 0.5f) * deltatime;
            } else if (velocity.y < 0.0f) {
                velocity.y += body.gravity * deltatime;
            } else {
                velocity.y += body.gravity * 2.0f * deltatime;
            }
            if (velocity.y > 2800.0f) velocity.y = 2800.0f;
        } else if (was_grounded && velocity.y > 0.0f) {
            velocity.y = 0.0f;
            body.falling = 0;
        }

        body.wallPush = 0;

        comp::InputState * inputPtr = nullptr;
        if (registry.has_component<comp::InputState>(entity)) inputPtr = &registry.get_component<comp::InputState>(entity);

        Vec2 temp_pos = position;
        const float dx = velocity.x * deltatime;
        temp_pos.x = try_move_x(scene, temp_pos, body, dx, velocity, was_grounded, inputPtr);

        if (was_grounded && velocity.y >= 0.0f) {
            const float half_h = body.size.y * 0.5f;
            float floor_y = 0.0f;
            const float snap_down = std::max(std::abs(dx) + body.skin + 2.0f, 4.0f);
            const float snap_up = std::max(std::abs(dx) + body.skin + 2.0f, 4.0f);
            if (find_body_floor(scene, temp_pos, body, snap_down, snap_up, floor_y)) {
                temp_pos.y = floor_y - half_h - body.skin;
                set_grounded(body, velocity);
            } else {
                temp_pos.y = try_move_y(scene, temp_pos, body, velocity.y * deltatime, velocity);
            }
        } else {
            temp_pos.y = try_move_y(scene, temp_pos, body, velocity.y * deltatime, velocity);
        }

        position = temp_pos;
    }
}