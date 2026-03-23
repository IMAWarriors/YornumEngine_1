// RenderSystem.cpp

#include "RenderSystem.h"
#include "../../Engine/Core/Rendering/Renderer.h"
#include "../../Engine/Core/Overhead/Config.h"
#include "../../Engine/Core/Overhead/WindowStats.h"
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

        int first_column = (int)(std::floor(camera_position.x / gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS));
        int first_row    = (int)(std::floor(camera_position.y / gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS));

        // Get first place we will show X and Y rendering physically on the screen (little past top left corner)
        // by utilizing the first column and row and dividing camera position based on where we wknow howt o be the row
        float first_tile_screen_x = (float)(first_column * gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS) - camera_position.x;
        float first_tile_screen_y = (float)(first_row * gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS) - camera_position.y;

        int visible_cols = config::GAME_WORLD_WIDTH / gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS + 2;
        int visible_rows = config::GAME_WORLD_HEIGHT / gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS + 2;

        // ========================================================================================================
        // MAIN TILESET LAYERS DRAWINGS SYSTEM
        // ------------
        // This is where we draw each tile grid layer
        //
        // ========================================================================================================

        for (TileGrid layer : scene.tile_layers) {

            // Pretty much all of this is unadjusted for CAMERA ZOOM


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

            // Before commencing main loop, if we are in editor mode,
            // Lets check to see what tile would be highlighted in the current viewport

            bool mouseHover = false;
            int hoverWorldCol = 0;
            int hoverWorldRow = 0;

            //                                 EDITOR ONLY CODE 
            // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            if (renderSceneEditorUI) {
        
                Vec2 mouse_position = winstats::ScreenMousePosition();


                float worldMouseX = mouse_position.x + camera_position.x;
                float worldMouseY = mouse_position.y + camera_position.y;

                hoverWorldCol = (int)(std::floor(worldMouseX/(gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS)));
                hoverWorldRow = (int)(std::floor(worldMouseY/(gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS)));

                mouseHover = true;

            }
            // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

            // Outer loop, draw until the cursor x gets to the end past end x which is like the end of the screen ig like past where we care abt rendering right
            for (int row_offset = 0; row_offset < visible_rows;  row_offset++) {

                for (int col_offset = 0; col_offset < visible_cols; col_offset++) {
                    // Continues are safe now cuz we have built in iterators

                    // Remember layer is just a tile_grid 
                    int world_column = first_column + col_offset;
                    int world_row    = first_row + row_offset;

                    float screen_x = first_tile_screen_x + (col_offset * gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS);
                    float screen_y = first_tile_screen_y + (row_offset * gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS);



                    // Failsafe asks: Are our columns and row of the world we are trying to draw in bounds? If not, move onto next tile of the loop
                    if (world_column < gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE || world_column > gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE) { continue; }
                    if (world_row < gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE    || world_row > gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE)    { continue; }

                    // If the world area we are accessing is in normal grid bounds, then draw
                    Tile tile_to_draw = layer.get_tile(world_column, world_row);

                    // Lets actually just put a rectangle or smth around where the mouse is ig




                    size_t atlasMaxIndex = scene.loaded_atlases[tile_to_draw.atlas_idx].getTileIdx(
                        scene.loaded_atlases[tile_to_draw.atlas_idx].tiles_per_row - 1, 
                        scene.loaded_atlases[tile_to_draw.atlas_idx].tiles_per_col - 1);


                        // Draw tile
                    if (0 <= tile_to_draw.tile_idx && tile_to_draw.tile_idx <= atlasMaxIndex) {  // Just means to not draw anything if there is a tile idx tryna be gotten not in teh atlas                                                                        
                        
                        // Quick little check
                        // If we get to this point, it means that the tile is also a valid tile in the index so we're good to draw

                        renderer.rdraw_sprite(*(scene.loaded_atlases[tile_to_draw.atlas_idx].image_sheet_source), 
                            scene.loaded_atlases[tile_to_draw.atlas_idx].getRect(tile_to_draw.tile_idx), 
                               {screen_x, 
                                screen_y, 
                                (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS, 
                                (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS}
                            );

                       
                        
                        if (mouseHover) {

                            Vec2 mousepos = winstats::ScreenMousePosition();
                            Rectangle mouserect = {mousepos.x - 5.0f, mousepos.y - 5.0f, 5.0f, 5.0f};

                            DrawRectangleLinesEx(mouserect, 3.0f, YELLOW);

                            if (world_column == hoverWorldCol && world_row == hoverWorldRow) {
                                Rectangle thisTileDestination = {screen_x, screen_y, (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS, (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS};
                                DrawRectangleLinesEx(thisTileDestination, 3.0f, RED);

                            }
                        }
                   
                   
                    }

                
                }

                
            }
        }

        
    }







    // STEP DRAW LOOP: Draw all Circles

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


