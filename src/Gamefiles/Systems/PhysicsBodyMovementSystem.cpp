#include "PhysicsBodyMovementSystem.h"

#include <algorithm>
#include <cmath>


static bool inside_collision (Scene & scene, Vec2 position, comp::PhysicsBody body, bool with_skin) {

    float skin = body.skin;

    float width = body.size.x;
    float height = body.size.y;

    float half_w = width / 2.0f;
    float half_h = height / 2.0f;

    bool coll_skin_UL, coll_skin_UR, coll_skin_BL, coll_skin_BR;
    bool coll_body_UL, coll_body_UR, coll_body_BL, coll_body_BR;
    Vec2 skin_UL, skin_UR, skin_BL, skin_BR;
    Vec2 body_UL, body_UR, body_BL, body_BR;


    bool coll_skin = false;
    bool coll_body = false;
    
    for (const TileGrid & layer : scene.tile_layers) {
        skin_UL = {    (position.x - half_w) + skin, (position.y - half_h) + skin};
        skin_UR = {    (position.x + half_w) - skin, (position.y - half_h) + skin};
        skin_BL = {    (position.x - half_w) + skin, (position.y + half_h) - skin};
        skin_BR = {    (position.x + half_w) - skin, (position.y + half_h) - skin};
        body_UL = {    (position.x - half_w), (position.y - half_h) };
        body_UR = {    (position.x + half_w), (position.y - half_h) };
        body_BL = {    (position.x - half_w), (position.y + half_h) };
        body_BR = {    (position.x + half_w), (position.y + half_h) };

        coll_skin_UL = (layer.get_tile_coll_pos(scene, skin_UL) == CollisionType::COLL_FULL_SOLID);
        coll_skin_UR = (layer.get_tile_coll_pos(scene, skin_UR) == CollisionType::COLL_FULL_SOLID);
        coll_skin_BL = (layer.get_tile_coll_pos(scene, skin_BL) == CollisionType::COLL_FULL_SOLID);
        coll_skin_BR = (layer.get_tile_coll_pos(scene, skin_BR) == CollisionType::COLL_FULL_SOLID);
        coll_body_UL = (layer.get_tile_coll_pos(scene, body_UL) == CollisionType::COLL_FULL_SOLID);
        coll_body_UR = (layer.get_tile_coll_pos(scene, body_UR) == CollisionType::COLL_FULL_SOLID);
        coll_body_BL = (layer.get_tile_coll_pos(scene, body_BL) == CollisionType::COLL_FULL_SOLID);
        coll_body_BR = (layer.get_tile_coll_pos(scene, body_BR) == CollisionType::COLL_FULL_SOLID);

        coll_skin = (coll_skin_UL || coll_skin_UR || coll_skin_BL || coll_skin_BR);
        coll_body = (coll_body_UL || coll_body_UR || coll_body_BL || coll_body_BR);

        if (with_skin) {
            if (coll_skin) {
                return coll_skin;
            }
        } else {
            if (coll_body) {
                return coll_body;
            }
        }
    }



    if (with_skin) {
        return coll_skin;
    } else {
        return coll_body;
    }
}




void PhysicsBodyMovementSystem::update (Registry & registry, float deltatime) {



    for (Entity entity : registry.view<comp::Transform, comp::Velocity, comp::PhysicsBody>()) {


        // Step 1.) Initialize Variables

        comp::Transform & transform = registry.get_component<comp::Transform>(entity);
        comp::Velocity & velocity = registry.get_component<comp::Velocity>(entity);
        comp::PhysicsBody & body = registry.get_component<comp::PhysicsBody>(entity);

        transform.previous_position = transform.position;
        float tile_size = gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS;

        Vec2 new_position = transform.position;

        bool body_colliding = false;
        bool skin_colliding = false;




        // Handle horizontal state (direction and stuff)
        // ***************************

        int x_dir = 0;
        x_dir = (velocity.magnitude.x > 0) ? 1 : (velocity.magnitude.x < 0) ? -1 : x_dir;




        // Handle grounding state / vertical positioning from last frame
        // before doing collision check
        // ***************************

        const float GROUNDED_CHECK_DIST = 0.5f;

        bool previous_onSolidGround = inside_collision(scene, {transform.position.x, transform.position.y + GROUNDED_CHECK_DIST}, body, false);

        body.onSolidGround = previous_onSolidGround;

        if (!(previous_onSolidGround && velocity.magnitude.y >= 0.0f)) {
            velocity.magnitude.y += body.gravity * deltatime;
        } else {
            velocity.magnitude.y = 0.0f;
        }

        if (velocity.magnitude.y > 2800.0f) {
            velocity.magnitude.y = 2800.0f;
        }

        int y_dir = 0;
        y_dir = (velocity.magnitude.y > 0) ? 1 : (velocity.magnitude.y < 0) ? -1 : y_dir;
        
        new_position = { transform.position.x + (velocity.magnitude.x * deltatime), transform.position.y + (velocity.magnitude.y * deltatime) };

        // =================================================================================================

        // Step 2.) Test collision along desired X movement

        body_colliding = inside_collision(scene, {new_position.x, transform.position.y}, body, false);
        skin_colliding = inside_collision(scene, {new_position.x, transform.position.y}, body, true);

        if (skin_colliding) {
            // If there was a collision detected along X:
            //      1.) if {Move X} > 0 : Clamp player to the outside of the LEFT of tile 
            //          if {Move X} < 0 : Clamp player to the outside of the RIGHT of the tile
            //
            velocity.magnitude.x = 0;
            int in_tile_col = scene.translate_world_x_col(new_position.x);
            int in_tile_row = scene.translate_world_y_row(transform.position.y);

            if (x_dir > 0) {
                // Snap player to flush edge of tile
                new_position.x = scene.get_tile_pos_UL(in_tile_col + 1, in_tile_row).x - ((body.size.x / 2.0f));
                body_colliding = inside_collision(scene, {new_position.x, transform.position.y}, body, false);
                skin_colliding = inside_collision(scene, {new_position.x, transform.position.y}, body, true);


                while (body_colliding) {
                    new_position.x -= 1.0f; //scene.get_tilesize();
                    body_colliding = inside_collision(scene, {new_position.x, transform.position.y}, body, false);
                }

            } else {

                // Snap player to flush edge of tile
                new_position.x = scene.get_tile_pos_UL(in_tile_col, in_tile_row).x + ((body.size.x / 2.0f));
                body_colliding = inside_collision(scene, {new_position.x, transform.position.y}, body, false);
                skin_colliding = inside_collision(scene, {new_position.x, transform.position.y}, body, true);

                while (body_colliding) {
                    new_position.x += 1.0f;  // scene.get_tilesize();
                    body_colliding = inside_collision(scene, {new_position.x, transform.position.y}, body, false);
                }
            }
        } 

        

        // Step 3.) Test collision along desired Y movement

        body_colliding = inside_collision(scene, {new_position.x, new_position.y}, body, false);
        skin_colliding = inside_collision(scene, {new_position.x, new_position.y}, body, true);

        if (skin_colliding) {
            // If there was a collision detected along Y:
            //      1.) if {Move Y} > 0 : Clamp player to the outside of the TOP of tile 
            //          if {Move Y} < 0 : Clamp player to the outside of the BOTTOM of the tile
            //
            velocity.magnitude.y = 0;
            int in_tile_col = scene.translate_world_x_col(new_position.x);
            int in_tile_row = scene.translate_world_y_row(new_position.y);

            if (y_dir > 0) {
                // Snap player to flush edge of tile
                new_position.y = scene.get_tile_pos_UL(in_tile_col, in_tile_row + 1).y - ((body.size.y / 2.0f));
                body_colliding = inside_collision(scene, {new_position.x, new_position.y}, body, false);
                skin_colliding = inside_collision(scene, {new_position.x, new_position.y}, body, true);

                while (body_colliding) {
                    new_position.y -= 1.0f; // scene.get_tilesize();
                    body_colliding = inside_collision(scene, {new_position.x, new_position.y}, body, false);
                }

            } else {
                // Snap player to flush edge of tile
                new_position.y = scene.get_tile_pos_UL(in_tile_col, in_tile_row).y + ((body.size.y / 2.0f));
                body_colliding = inside_collision(scene, {new_position.x, new_position.y}, body, false);
                skin_colliding = inside_collision(scene, {new_position.x, new_position.y}, body, true);

                while (body_colliding) {
                    new_position.y += 1.0f; // scene.get_tilesize();
                    body_colliding = inside_collision(scene, {new_position.x, new_position.y}, body, false);
                }

                body.onSolidGround = true;

            }
        } else {
            body.onSolidGround = false;
        }

       
        transform.position.x = new_position.x;
        transform.position.y = new_position.y;




        body.inColl             = inside_collision(scene, new_position, body, false);
        body.innerSkinInColl    = inside_collision(scene, new_position, body, true);

        
    }
}