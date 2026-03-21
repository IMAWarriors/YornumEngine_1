// RenderSystem.cpp

#include "RenderSystem.h"
#include "../../Engine/Core/Rendering/Renderer.h"
#include "../../Gamefiles/World/Scene.h"
#include "../../Gamefiles/World/Tile.h"
#include "../../Gamefiles/World/Overhead/Gwconst.h"



void RenderSystem::update (Registry & registry, float deltatime) {



     // "DELTATIME" is technically i guess an alpha here that we have to use to interpolate

    // Set internal renderer camera position according to interpolation bullshit
    for (Entity entity : registry.view<comp::Camera, comp::Transform>()) {

        comp::Transform& transform = registry.get_component<comp::Transform>(entity);

        Vec2 camera_interpolation = {
            transform.previous_position.x + (transform.position.x - transform.previous_position.x) * deltatime,
            transform.previous_position.y + (transform.position.y - transform.previous_position.y) * deltatime
        };

        renderer.set_camera_position(camera_interpolation);

        break; // assume 1 camera
    }





    // Going to try to draw tiles here first I guess, assuming the default layer of tiles is BEHIND the player

    // DRAWING TILES, UTILIZING THE SCENE, NOT FUCKING ENTITIES BECAUSE THATS STUPID
    
    if (scene.loaded_atlases.size() > 0 && scene.tile_layers.size() > 0) {

        Vec2 camera_position = renderer.get_camera_position();      // Top Left of Camera

        int camera_mod_x_offset = (int)(camera_position.x) % scene.loaded_atlases[0].tile_size;
        int camera_mod_y_offset = (int)(camera_position.y) % scene.loaded_atlases[0].tile_size;



        // ========================================================================================================
        // MAIN TILESET LAYERS DRAWINGS SYSTEM
        // ------------
        // This is where we draw each tile grid layer
        //
        // ========================================================================================================

        for (TileGrid layer : scene.tile_layers) {

            // Pretty much all of this is unadjusted for CAMERA ZOOM

            int start_x = (int)(camera_position.x);
            int start_y = (int)(camera_position.y);

            int end_x = start_x + config::GAME_WORLD_WIDTH;
            int end_y = start_y + config::GAME_WORLD_HEIGHT;

            start_x = start_x - camera_mod_x_offset;
            start_y = start_y - camera_mod_y_offset;

            int drawcursor_x = start_x;
            int drawcursor_y = start_y;

            
            auto iterate_drawcursor_x = [&] (int tilesize) {
                // UNADJUSTED FOR CAMERA ZOOM
                drawcursor_x += tilesize;
            };

            auto iterate_drawcursor_y = [&] (int tilesize) {
                // UNADJUSTED FOR CAMERA ZOOM
                drawcursor_y += tilesize;
                drawcursor_x = start_x;
            };

            // ==============================================================
            //   Main Actual Draw Loop
            // ---------------------------------------
            //  -> Uses a cursor based system of cursor x and y draw from the top left corner of where the first tile should
            // be drawn, calculated at beginning using start x and y.
            //
            //  -> Uses inline defined iterators to change the cursors by the tilesize amount
            //
            //  -> TRies to draw according to atlas and whatever layers are there 
            //
            // ==============================================================

            // Outer loop, draw until the cursor x gets to the end past end x which is like the end of the screen ig like past where we care abt rendering right
            while (drawcursor_y < end_y) {

                while (drawcursor_x < end_x) {

                    // Remember layer is just a tile_grid 
                    int world_column = layer.translate_world_x_col(drawcursor_x);
                    int world_row    = layer.translate_world_y_row(drawcursor_y);

                    // IF statement: to protect from drawing tiles outside of defined grid
                    
                    if ( (gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE <= world_column && world_column <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE) &&
                         (gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE <= world_row && world_row <= gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE ) ) {

                            // If the world area we are accessing is in normal grid bounds, then draw, if not uh idk...

                        Tile tile_to_draw = layer.get_tile(world_column, world_row);

                        size_t atlasMaxIndex = scene.loaded_atlases[tile_to_draw.atlas_idx].getTileIdx(
                            scene.loaded_atlases[tile_to_draw.atlas_idx].tiles_per_row - 1, 
                            scene.loaded_atlases[tile_to_draw.atlas_idx].tiles_per_col - 1);


                        if (0 <= tile_to_draw.tile_idx && tile_to_draw.tile_idx <= atlasMaxIndex) {  // Just means to not draw anything if there is a tile idx tryna be gotten not in teh atlas                                                                        
                            renderer.rdraw_sprite(scene.loaded_atlases[tile_to_draw.atlas_idx].image_sheet_source, 
                                scene.loaded_atlases[tile_to_draw.atlas_idx].getRect(tile_to_draw.tile_idx), 
                                {(float)drawcursor_x - camera_position.x, (float)drawcursor_y - camera_position.y, (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS, (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS});
                        }

                    }

                    iterate_drawcursor_x(gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS);
                }

                iterate_drawcursor_y(gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS);
                
            }
        }

        
    }











    // STEPS HERE:
    //
    // -> Calculate basic Camera setup and whre eit should go i guess
    //
    // -> Draw all the entities with a Circle Renderer
    //



   


    // Draw all Circles

    for (Entity entity : registry.view<comp::Transform, comp::CircleRenderer>()) {      // For each iteration of Entity


            comp::Transform & transform     = registry.get_component<comp::Transform>(entity);
            comp::CircleRenderer & circle = registry.get_component<comp::CircleRenderer>(entity);


            unsigned char _r = circle.color.r;
            unsigned char _g = circle.color.g;
            unsigned char _b = circle.color.b;
            unsigned char _a = circle.color.a;


            Color col = Color({_r, _g, _b, _a});

            Vec2 interpolation = {transform.previous_position.x + ((transform.position.x-transform.previous_position.x) * deltatime), transform.previous_position.y + ((transform.position.y-transform.previous_position.y) * deltatime)};

            renderer.rdraw_circle(interpolation.x, interpolation.y, circle.radius, col);


    }

}


