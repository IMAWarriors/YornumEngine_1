// PhysicsBodyMovementSystem.cpp

#include "PhysicsBodyMovementSystem.h"

#include <algorithm>
#include <cmath>


// ============================================================
//  try_move_x
//  Sweeps player horizontally by `delta` pixels, resolves
//  against solid tiles, returns the resolved centre-X.
//  pos   - current centre position (passed by value, local copy)
//  vel   - velocity ref so we can zero on collision
//  input - optional, used only to set wall-push state
// ============================================================
static float try_move_x (Scene & scene, Vec2 pos, comp::PhysicsBody & body, float delta, Vec2 & vel, comp::InputState * input = nullptr) {

    // Nothing to do - bail early
    if (std::abs(delta) < 0.0001f) return pos.x;

    const float tile_size = (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS;
    const float move      = std::abs(delta);
    const int   x_dir    = (delta > 0.0f) ? 1 : -1;

    const float half_w = body.size.x * 0.5f;
    const float half_h = body.size.y * 0.5f;

    // The vertical strip the player's body occupies - same regardless of direction
    // Subtract a tiny epsilon from bottom so a player standing exactly on a tile
    // boundary doesn't bleed into the tile row below during horizontal checks
    int tile_y_min = (int)std::floor((pos.y - half_h)          / tile_size);
    int tile_y_max = (int)std::floor((pos.y + half_h - 0.001f) / tile_size);

    bool  found_collision = false;
    float best_resolve_x  = pos.x + delta;   // default: full move, no collision


    if (x_dir == 1) {

        // ── Moving RIGHT ──────────────────────────────────────────────────────

        const float leading_edge = pos.x + half_w;         // right edge now
        const float target_edge  = leading_edge + move;    // right edge after full move

        int tile_x_min = (int)std::floor(leading_edge           / tile_size);
        int tile_x_max = (int)std::floor((target_edge + 0.001f) / tile_size);

        for (const TileGrid & layer : scene.tile_layers)
        for (int tx = tile_x_min; tx <= tile_x_max; tx++)
        for (int ty = tile_y_min; ty <= tile_y_max; ty++) {

            if (layer.get_tile_coll(scene, tx, ty) != CollisionType::COLL_FULL_SOLID) { continue; }

            float tile_left = tx * tile_size;

            // Skip - tile is behind us or already overlapping us past skin depth
            if (tile_left < leading_edge - body.skin) { continue; }
            // Skip - tile is beyond where we could possibly reach this frame
            if (tile_left > target_edge)              { continue; }

            // Where our centre lands flush against this tile's left edge
            float candidate = tile_left - half_w - body.skin;

            if (!found_collision || candidate < best_resolve_x) {
                best_resolve_x  = candidate;
                found_collision = true;
            }
        }

        if (found_collision) {
            vel.x = 0.0f;
            if (input != nullptr && input->horz_axis > 0) {
                body.wallPush       =  1;
                body.lastWallPush   =  1;
                body.walljumpWindow = 6;
                body.againstWall    = true;
            }
        }

    } else {

        // ── Moving LEFT ───────────────────────────────────────────────────────

        const float leading_edge = pos.x - half_w;         // left edge now
        const float target_edge  = leading_edge - move;    // left edge after full move

        int tile_x_min = (int)std::floor((target_edge - 0.001f) / tile_size);
        int tile_x_max = (int)std::floor(leading_edge            / tile_size);

        for (const TileGrid & layer : scene.tile_layers)
        for (int tx = tile_x_min; tx <= tile_x_max; tx++)
        for (int ty = tile_y_min; ty <= tile_y_max; ty++) {

            if (layer.get_tile_coll(scene, tx, ty) != CollisionType::COLL_FULL_SOLID) { continue; }

            float tile_right = (tx + 1) * tile_size;

            // Skip - tile is behind us or already overlapping us past skin depth
            if (tile_right > leading_edge + body.skin) { continue; }
            // Skip - tile is beyond where we could possibly reach this frame
            if (tile_right < target_edge)              { continue; }

            // Where our centre lands flush against this tile's right edge
            float candidate = tile_right + half_w + body.skin;

            if (!found_collision || candidate > best_resolve_x) {
                best_resolve_x  = candidate;
                found_collision = true;
            }
        }

        if (found_collision) {
            vel.x = 0.0f;
            if (input != nullptr && input->horz_axis < 0) {
                body.wallPush       = -1;
                body.lastWallPush   = -1;
                body.walljumpWindow = 6;
                body.againstWall    = true;
            }
        }
    }

    return best_resolve_x;
}


// ============================================================
//  try_move_y
//  Sweeps player vertically by `delta` pixels, resolves
//  against solid tiles, returns the resolved centre-Y.
//  Also updates body.onSolidGround and body.falling.
// ============================================================
static float try_move_y (Scene & scene, Vec2 pos, comp::PhysicsBody & body, float delta, Vec2 & vel) {

    // Zero-delta path: no movement requested this frame, but we still need to
    // know whether the player is standing on something so that gravity and the
    // jump check in PlayerControllerSystem are correct.
    // We do a tiny 1px downward probe to detect the ground.
    if (std::abs(delta) < 0.0001f) {

        const float tile_size = (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS;
        const float half_w    = body.size.x * 0.5f;
        const float half_h    = body.size.y * 0.5f;
        const float probe     = 1.0f;

        float bottom     = pos.y + half_h;
        float probe_edge = bottom + probe;

        int tile_y_min = (int)std::floor(bottom                    / tile_size);
        int tile_y_max = (int)std::floor(probe_edge                 / tile_size);
        int tile_x_min = (int)std::floor((pos.x - half_w)          / tile_size);
        int tile_x_max = (int)std::floor((pos.x + half_w - 0.001f) / tile_size);

        bool grounded = false;
        for (const TileGrid & layer : scene.tile_layers) {
            for (int tx = tile_x_min; tx <= tile_x_max && !grounded; tx++)
            for (int ty = tile_y_min; ty <= tile_y_max && !grounded; ty++) {
                if (layer.get_tile_coll(scene, tx, ty) == CollisionType::COLL_FULL_SOLID) {
                    float tile_top = ty * tile_size;
                    if (tile_top >= bottom - body.skin && tile_top <= probe_edge) {
                        grounded = true;
                    }
                }
            }
        }

        body.onSolidGround = grounded;

        if (grounded) {
            // IMPORTANT: only reset if not jumping this frame.
            // If PlayerControllerSystem already set vel.y negative (upward jump),
            // do NOT zero it out or the jump gets cancelled before it can move.
            if (vel.y >= 0.0f) {
                vel.y                = 0.0f;
                body.falling         = 0;
                body.vjump_window    = 0;
                body.lastWalljumpDir = 0;
                body.walljumpBuffer  = 0;
                body.againstWall     = false;
            }
        }

        return pos.y;
    }

    // Normal path: player has non-zero vertical velocity this frame
    const float tile_size = (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS;
    const float move      = std::abs(delta);
    const int   y_dir    = (delta > 0.0f) ? 1 : -1;

    const float half_w = body.size.x * 0.5f;
    const float half_h = body.size.y * 0.5f;

    // Horizontal strip is always the same regardless of vertical direction
    int tile_x_min = (int)std::floor((pos.x - half_w)          / tile_size);
    int tile_x_max = (int)std::floor((pos.x + half_w - 0.001f) / tile_size);

    bool  found_collision = false;
    float best_resolve_y  = pos.y + delta;   // default: full move, no collision


    if (y_dir == 1) {

        // ── Moving DOWN (positive Y is visually downward) ─────────────────────

        const float leading_edge = pos.y + half_h;         // bottom edge now
        const float target_edge  = leading_edge + move;    // bottom edge after full move

        int tile_y_min = (int)std::floor(leading_edge           / tile_size);
        int tile_y_max = (int)std::floor((target_edge + 0.001f) / tile_size);

        for (const TileGrid & layer : scene.tile_layers)
        for (int tx = tile_x_min; tx <= tile_x_max; tx++)
        for (int ty = tile_y_min; ty <= tile_y_max; ty++) {

            if (layer.get_tile_coll(scene, tx, ty) != CollisionType::COLL_FULL_SOLID) { continue; }

            float tile_top = ty * tile_size;

            // Skip - tile top is above our current bottom (already inside or behind)
            if (tile_top < leading_edge - body.skin) { continue; }
            // Skip - tile is below where we could possibly reach this frame
            if (tile_top > target_edge)              { continue; }

            // Where our centre lands flush on top of this tile
            float candidate = tile_top - half_h - body.skin;

            if (!found_collision || candidate < best_resolve_y) {
                best_resolve_y  = candidate;
                found_collision = true;
            }
        }

        body.onSolidGround = found_collision;

        if (found_collision) {
            vel.y                = 0.0f;
            body.falling         = 0;
            body.vjump_window    = 0;
            body.lastWalljumpDir = 0;
            body.walljumpBuffer  = 0;
            body.againstWall     = false;
        } else {
            body.falling++;
        }

    } else {

        // ── Moving UP (negative Y is visually upward) ─────────────────────────

        const float leading_edge = pos.y - half_h;          // top edge now
        const float target_edge  = leading_edge - move;     // top edge after full move

        int tile_y_min = (int)std::floor((target_edge - 0.001f) / tile_size);
        int tile_y_max = (int)std::floor(leading_edge            / tile_size);

        for (const TileGrid & layer : scene.tile_layers)
        for (int tx = tile_x_min; tx <= tile_x_max; tx++)
        for (int ty = tile_y_min; ty <= tile_y_max; ty++) {

            if (layer.get_tile_coll(scene, tx, ty) != CollisionType::COLL_FULL_SOLID) { continue; }

            float tile_bottom = (ty + 1) * tile_size;

            // Skip - tile bottom is below our current top (already inside or behind)
            if (tile_bottom > leading_edge + body.skin) { continue; }
            // Skip - tile bottom is above where we could possibly reach this frame
            if (tile_bottom < target_edge)              { continue; }

            // Where our centre lands flush under this tile's bottom edge (head bump)
            float candidate = tile_bottom + half_h + body.skin;

            if (!found_collision || candidate > best_resolve_y) {
                best_resolve_y  = candidate;
                found_collision = true;
            }
        }

        // Moving up: onSolidGround is not touched here.
        // Ground state is only set by the downward branch and the zero-delta probe.
        if (found_collision) {
            vel.y = 0.0f;
        } else {
            body.falling++;
        }
    }

    return best_resolve_y;
}


// ============================================================
//  PhysicsBodyMovementSystem::update
// ============================================================
void PhysicsBodyMovementSystem::update (Registry & registry, float deltatime) {

    for (Entity entity : registry.view<comp::Transform, comp::Velocity, comp::PhysicsBody>()) {

        // Step 1.) Get components
        Vec2 & previous_position = registry.get_component<comp::Transform>(entity).previous_position;
        Vec2 & position          = registry.get_component<comp::Transform>(entity).position;
        Vec2 & velocity          = registry.get_component<comp::Velocity>(entity).magnitude;
        comp::PhysicsBody & body = registry.get_component<comp::PhysicsBody>(entity);

        // Step 2.) Save position before movement for interpolation
        previous_position = position;

        body.onSolidGround = false;

        // Step 3.) Wall-jump window decay
        // Decay the wall-jump window while the player is away from the wall and in the air
        if (body.walljumpWindow > 0 && body.wallPush == 0 && body.falling > 3) {
            body.walljumpWindow--;
        }
        if (body.walljumpWindow <= 0) {
            body.walljumpWindow = 0;
            body.againstWall    = false;
        }

        // Step 4.) Apply gravity
        // NOTE: we intentionally use onSolidGround from the *previous* frame here.
        // try_move_y will update it correctly for the current frame after movement.
        if (!body.onSolidGround && body.gravitous) {

            if (body.wallPush != 0 && velocity.y > 0.0f && body.falling > 5) {
                // Wall-slide: cap fall speed and drift slowly downward
                if (velocity.y > 250.0f) velocity.y = 250.0f;
                velocity.y += ((150.0f - velocity.y) * 0.5f) * deltatime;
            } else {
                // Normal gravity - stronger on the way down for better game-feel
                if (velocity.y < 0.0f) {
                    velocity.y += body.gravity * deltatime;
                } else {
                    velocity.y += body.gravity * 1.5f * deltatime;
                }
            }

            // Terminal velocity cap
            if (velocity.y > 2800.0f) velocity.y = 2800.0f;

        } else if (body.onSolidGround) {
            // Keep vel.y zeroed while grounded so gravity doesn't accumulate between frames.
            // Guard against zeroing a jump that PlayerControllerSystem just set this frame.

            velocity.y   = 0.0f;
            body.falling = 0;
        
        }

        // Step 5.) Reset wall-push flag for this frame
        // (try_move_x will re-set it if we actually hit a wall)
        body.wallPush = 0;

        // Step 6.) Grab optional input pointer for wall detection inside try_move_x
        comp::InputState * inputPtr = nullptr;
        if (registry.has_component<comp::InputState>(entity)) {
            inputPtr = &registry.get_component<comp::InputState>(entity);
        }

        // Step 7.) Sweep X then Y and commit resolved position
        Vec2 temp_pos = position;

        temp_pos.x = try_move_x(scene, temp_pos, body, velocity.x * deltatime, velocity, inputPtr);
        temp_pos.y = try_move_y(scene, temp_pos, body, velocity.y * deltatime, velocity);

        position = temp_pos;
    }
}