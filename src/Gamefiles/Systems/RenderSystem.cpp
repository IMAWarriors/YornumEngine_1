// RenderSystem.cpp

#include "RenderSystem.h"
#include "../../Engine/Core/Rendering/Renderer.h"
#include "../../Engine/Core/Window/Window.h"
#include "../../Engine/Core/Overhead/Config.h"
#include "../../Engine/Core/Overhead/WindowStats.h"
#include "../../Gamefiles/World/Scene.h"
#include "../../Gamefiles/World/Tile.h"
#include "../../Gamefiles/World/Overhead/Gwconst.h"
#include "../../Gamefiles/World/Background.h"

#include "../../Tooling/Debug/DebugManager.h"

#include <algorithm>




void RenderSystem::update (Registry & registry, float deltatime) {



     // "DELTATIME" is technically i guess an alpha here that we have to use to interpolate


    // Set internal renderer camera position according to interpolation bullshit
    for (Entity entity : registry.view<comp::Camera, comp::Transform>()) {

        comp::Transform& transform = registry.get_component<comp::Transform>(entity);
        comp::Camera & camera = registry.get_component<comp::Camera>(entity);

        Vec2 camera_interpolation = {
            transform.previous_position.x + (transform.position.x - transform.previous_position.x) * deltatime,
            transform.previous_position.y + (transform.position.y - transform.previous_position.y) * deltatime
        };

        float zoom_interpolation = camera.previous_zoom + (camera.zoom - camera.previous_zoom) * deltatime;

        renderer.set_camera_position(camera_interpolation);
        renderer.set_camera_zoom(zoom_interpolation);

        break; // assume 1 camera
    }

    const Vec2 camera_position_bg = renderer.get_camera_position();
    const float camera_zoom_bg = renderer.get_camera_zoom();

    if (scene.background.backdrop_image != nullptr) {
        renderer.rdraw_sprite_col(
            *(scene.background.backdrop_image),
            {0.0f, 0.0f, (float)scene.background.backdrop_image->width, (float)scene.background.backdrop_image->height},
            {0.0f, 0.0f, (float)config::GAME_WORLD_WIDTH, (float)config::GAME_WORLD_HEIGHT},
            WHITE
        );
    }

    const float base_screen_tile_w = (float)gwconst::SCREEN_WIDTH_GAMEPIXELS;
    const float base_screen_tile_h = (float)gwconst::SCREEN_HEIGHT_GAMEPIXELS;
    const float world_view_w = config::GAME_WORLD_WIDTH / camera_zoom_bg;
    const float world_view_h = config::GAME_WORLD_HEIGHT / camera_zoom_bg;
    const float half_world_w = world_view_w * 0.5f;
    const float half_world_h = world_view_h * 0.5f;

    for (const ParallaxLayer& layer : scene.background.layers) {
        if (layer.nodes.empty()) {
            continue;
        }

        const float depth = std::max(0.01f, layer.z_dist_offset);
        const float parallax_factor = 1.0f / depth;
        const float parallax_cam_x = (camera_position_bg.x * parallax_factor) + layer.x_dist_offset;
        const float parallax_cam_y = (camera_position_bg.y * parallax_factor) + layer.y_dist_offset;

        const float parallax_left = parallax_cam_x - half_world_w;
        const float parallax_top = parallax_cam_y - half_world_h;

        const int first_col = (int)std::floor(parallax_left / base_screen_tile_w);
        const int first_row = (int)std::floor(parallax_top / base_screen_tile_h);

        const int visible_cols = (int)std::ceil(world_view_w / base_screen_tile_w) + 2;
        const int visible_rows = (int)std::ceil(world_view_h / base_screen_tile_h) + 2;

        for (int row_offset = 0; row_offset < visible_rows; row_offset++) {
            for (int col_offset = 0; col_offset < visible_cols; col_offset++) {
                const int seat_x = first_col + col_offset;
                const int seat_y = first_row + row_offset;

                auto resolved = scene.background.resolve_node_for_tile(layer, seat_x, seat_y);
                if (!resolved.has_value() || resolved->image == nullptr) {
                    continue;
                }

                const float world_x = (seat_x * base_screen_tile_w);
                const float world_y = (seat_y * base_screen_tile_h);
                const float screen_x = (world_x - parallax_left) * camera_zoom_bg;
                const float screen_y = (world_y - parallax_top) * camera_zoom_bg;

                renderer.rdraw_sprite_col(
                    *(resolved->image),
                    {0.0f, 0.0f, (float)resolved->image->width, (float)resolved->image->height},
                    {screen_x, screen_y, base_screen_tile_w * camera_zoom_bg, base_screen_tile_h * camera_zoom_bg},
                    layer.tint
                );
            }
        }
    }







    














    // Going to try to draw tiles here first I guess, assuming the default layer of tiles is BEHIND the play

    // DRAWING TILES, UTILIZING THE SCENE, NOT FUCKING ENTITIES BECAUSE THATS STUPID
    
    float current_tile_animation_frame = 0.0f;
    for (Entity entity : registry.view<tag::EngineManager, comp::WorldAnimationState>()) {
        current_tile_animation_frame = registry.get_component<comp::WorldAnimationState>(entity).current_tile_animation_frame;
        break;
    }

    if (scene.loaded_atlases.size() > 0 && scene.tile_layers.size() > 0) {

        Vec2 camera_position = renderer.get_camera_position();      // Center of camera in world space
        float camera_zoom = renderer.get_camera_zoom();
        const float half_world_width = (config::GAME_WORLD_WIDTH * 0.5f) / camera_zoom;
        const float half_world_height = (config::GAME_WORLD_HEIGHT * 0.5f) / camera_zoom;
        const float world_left = camera_position.x - half_world_width;
        const float world_top = camera_position.y - half_world_height;

        int first_column = (int)(std::floor(world_left / gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS));
        int first_row    = (int)(std::floor(world_top / gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS));

        // Get first place we will show X and Y rendering physically on the screen (little past top left corner)
        // by utilizing the first column and row and dividing camera position based on where we wknow howt o be the row
        float first_tile_screen_x = ((float)(first_column * gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS) - world_left) * camera_zoom;
        float first_tile_screen_y = ((float)(first_row * gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS) - world_top) * camera_zoom;

        const float tile_step = (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS * camera_zoom;

        int visible_cols = (int)std::ceil((config::GAME_WORLD_WIDTH / camera_zoom) / gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS) + 2;
        int visible_rows = (int)std::ceil((config::GAME_WORLD_HEIGHT / camera_zoom) / gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS) + 2;

        // ========================================================================================================
        // MAIN TILESET LAYERS DRAWINGS SYSTEM
        // ------------
        // This is where we draw each tile grid layer
        //
        // ========================================================================================================

        int ilayer = 0;

        for (TileGrid & layer : scene.tile_layers) {

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

            

            //                                 EDITOR ONLY CODE 
            // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

            bool mouseHover = false;
            int hoverWorldCol = 0;
            int hoverWorldRow = 0;

            if (renderSceneEditorUI) {
        
                Vec2 mouse_position = winstats::ScreenMousePosition();

                float worldMouseX = camera_position.x + ((mouse_position.x - (config::GAME_WORLD_WIDTH * 0.5f)) / camera_zoom);
                float worldMouseY = camera_position.y + ((mouse_position.y - (config::GAME_WORLD_HEIGHT * 0.5f)) / camera_zoom);

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

                    float raw_left   = first_tile_screen_x + (col_offset * tile_step);
                    float raw_right  = first_tile_screen_x + ((col_offset + 1) * tile_step);
                    float raw_top    = first_tile_screen_y + (row_offset * tile_step);
                    float raw_bottom = first_tile_screen_y + ((row_offset + 1) * tile_step);

                    float screen_x = std::round(raw_left);
                    float screen_y = std::round(raw_top);
                    float screen_w = std::round(raw_right) - screen_x;
                    float screen_h = std::round(raw_bottom) - screen_y;



                    // Failsafe asks: Are our columns and row of the world we are trying to draw in bounds? If not, move onto next tile of the loop
                    if (world_column < gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE || world_column > gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE) { continue; }
                    if (world_row < gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE    || world_row > gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE)    { continue; }

                    // If the world area we are accessing is in normal grid bounds, then draw
                    Tile tile_to_draw = layer.get_tile(world_column, world_row);
                    bool is_empty_tile = false;

                    // Lets actually just put a rectangle or smth around where the mouse is ig


                    

                    size_t atlasMaxIndex = 0;
                    if (tile_to_draw.atlas_idx >= 0 && tile_to_draw.tile_idx >= 0) {
                        if (!scene.is_valid_atlas_index(tile_to_draw.atlas_idx)) {
                            is_empty_tile = true;
                        }

                        if (!is_empty_tile) {
                            tile_to_draw.tile_idx = scene.normalize_tile_to_animation_parent(tile_to_draw.atlas_idx, tile_to_draw.tile_idx);
                            if (tile_to_draw.tile_idx < 0) {
                                is_empty_tile = true;
                            }
                        }

                        if (!is_empty_tile) {
                            atlasMaxIndex = scene.loaded_atlases[tile_to_draw.atlas_idx].getTileIdx(
                            scene.loaded_atlases[tile_to_draw.atlas_idx].tiles_per_row - 1,
                            scene.loaded_atlases[tile_to_draw.atlas_idx].tiles_per_col - 1);

                            if (tile_to_draw.tile_idx > atlasMaxIndex) {
                                is_empty_tile = true;
                                atlasMaxIndex = 9999;
                            }
                            
                        }
                    } else {
                        is_empty_tile = true;
                        atlasMaxIndex = 9999;
                    }

                
                    // Quick little check
                    // If we get to this point, it means that the tile is also a valid tile in the index so we're good to draw


                    if (!is_empty_tile) {


                        if (!scene.EDITOR_ONLY_ONION_LAYER_MODE) {
                            renderer.rdraw_sprite(*(scene.loaded_atlases[tile_to_draw.atlas_idx].image_sheet_source),
                                scene.loaded_atlases[tile_to_draw.atlas_idx].getRect(current_tile_animation_frame, tile_to_draw.tile_idx),
                                {screen_x,
                                    screen_y,
                                    screen_w,
                                    screen_h}
                                );
                        } else {

                            if (ilayer == scene.EDITOR_ONLY_SELECTED_LAYER) {

                                renderer.rdraw_sprite(*(scene.loaded_atlases[tile_to_draw.atlas_idx].image_sheet_source),
                                scene.loaded_atlases[tile_to_draw.atlas_idx].getRect(current_tile_animation_frame, tile_to_draw.tile_idx),
                                {screen_x,
                                    screen_y,
                                    screen_w,
                                    screen_h}
                                );

                            } else {

                                // Draw ghostly if onion layer mode is on and this is not the main layer
                                renderer.rdraw_sprite_col(*(scene.loaded_atlases[tile_to_draw.atlas_idx].image_sheet_source),
                                scene.loaded_atlases[tile_to_draw.atlas_idx].getRect(current_tile_animation_frame, tile_to_draw.tile_idx),
                                {screen_x,
                                    screen_y,
                                    screen_w,
                                    screen_h}, 
                                    {255, 255, 255, 145}
                                );
                            }
                        }


                        

                            
                            if (G_DEBUGGER.show == true) {
                                if (layer.get_tile_coll(scene, world_column, world_row) == CollisionType::COLL_FULL_SOLID) {
                                    Rectangle thisTileDestination = {
                                        screen_x,
                                        screen_y,
                                        (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS * camera_zoom,
                                        (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS * camera_zoom
                                    };
                                    DrawRectangleLinesEx(thisTileDestination, 1.0f, RED);
                                }

                            }




                    }


                    //                                 EDITOR ONLY CODE 
                    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                    // renderSceneEditorUI check is reduntant, but reminds that this is ediotr only code
                    
                    
                    if (renderSceneEditorUI && mouseHover && !scene.EDITOR_ONLY_ACTIVE_TAEDITOR && !scene.EDITOR_ONLY_ACTIVE_BACKGROUND_EDITOR && !scene.EDITOR_ONLY_BACKGROUND_TAB_SELECTED && !scene.uiCapturesMouse) {

                        bool hoveringTileGrid = false;

                        Vec2 mousepos = winstats::ScreenMousePosition();
                        Rectangle mouserect = {mousepos.x - 5.0f, mousepos.y - 5.0f, 5.0f, 5.0f};

                        if (mousepos.y < 520.0f) {
                            hoveringTileGrid = true;
                        }

                        DrawRectangleLinesEx(mouserect, 3.0f, YELLOW);

                        if (hoveringTileGrid) {
                            if (world_column == hoverWorldCol && world_row == hoverWorldRow) {
                                Rectangle thisTileDestination = {
                                    screen_x,
                                    screen_y,
                                    (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS * camera_zoom,
                                    (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS * camera_zoom
                                };
                                
                                if (scene.EDITOR_ONLY_SELECTED_ATLAS >= 0 && scene.EDITOR_ONLY_SELECTED_PALLET_TILE >= 0) {
                                     int preview_tile = scene.normalize_tile_to_animation_parent(scene.EDITOR_ONLY_SELECTED_ATLAS, scene.EDITOR_ONLY_SELECTED_PALLET_TILE);
                                    renderer.rdraw_sprite_col(*(scene.loaded_atlases[scene.EDITOR_ONLY_SELECTED_ATLAS].image_sheet_source), 
                                        scene.loaded_atlases[scene.EDITOR_ONLY_SELECTED_ATLAS].getRect(current_tile_animation_frame, preview_tile), thisTileDestination, {255,255,255,185}
                                    );
                                }

                                DrawRectangleLinesEx(thisTileDestination, 3.0f, RED);
                            }


                            if (scene.EDITOR_ONLY_SELECTED_LAYER >= 0 && scene.EDITOR_ONLY_SELECTED_LAYER == ilayer) {
                            
                                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && world_column == hoverWorldCol && world_row == hoverWorldRow) {
                                    if (!scene.is_valid_atlas_index(scene.EDITOR_ONLY_SELECTED_ATLAS) || scene.EDITOR_ONLY_SELECTED_PALLET_TILE < 0) {
                                        layer.get_tile(world_column, world_row) = {-1,-1};
                                    } else {
                                        int selected_parent_tile = scene.normalize_tile_to_animation_parent(scene.EDITOR_ONLY_SELECTED_ATLAS, scene.EDITOR_ONLY_SELECTED_PALLET_TILE);
                                        layer.get_tile(world_column, world_row) = {scene.EDITOR_ONLY_SELECTED_ATLAS, selected_parent_tile};
                                    }
                                }

                            }

                        }
                    }

                    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


                   
                   
                    

                
                }

                
            }

            ilayer++;

        }

        
    }


    
    if (G_DEBUGGER.show == true) {

        static int active_corner = -1;      // 0–3 for player zone, 4–7 for clamp zone
        static CameraClamp* active_clamp = nullptr;


        for (CameraClamp & clamp : scene.active_clamps) {

            Vec2 cam = renderer.get_camera_position();
            float zoom = renderer.get_camera_zoom();

            float half_w = (config::GAME_WORLD_WIDTH * 0.5f) / zoom;
            float half_h = (config::GAME_WORLD_HEIGHT * 0.5f) / zoom;

            float cam_left   = cam.x - half_w;
            float cam_right  = cam.x + half_w;
            float cam_top    = cam.y - half_h;
            float cam_bottom = cam.y + half_h;

            if (clamp.player_zone_top_left.x > cam_right) {
                continue;
            }

            if (clamp.player_zone_bottom_right.x < cam_left) {
                continue;
            }

            if (clamp.player_zone_top_left.y > cam_bottom) {
                continue;
            }

            if (clamp.player_zone_bottom_right.y < cam_top) {
                continue;
            }

            float pz_width     = clamp.player_zone_bottom_right.x - clamp.player_zone_top_left.x;
            float pz_height    = clamp.player_zone_bottom_right.y - clamp.player_zone_top_left.y;

            float cc_width     = clamp.clamp_bottom_right.x - clamp.clamp_top_left.x;
            float cc_height    = clamp.clamp_bottom_right.y - clamp.clamp_top_left.y;

            Color pz_color = ORANGE;
            Color cc_color = YELLOW;

            renderer.rdraw_wfrect(clamp.player_zone_top_left.x, clamp.player_zone_top_left.y, pz_width, pz_height, pz_color, 3.0f);
            renderer.rdraw_wfrect(clamp.clamp_top_left.x, clamp.clamp_top_left.y, cc_width, cc_height, cc_color, 3.0f);


            

            Vec2 mouse_position = {-9999.0f,9999.0f};

            for (Entity entity : registry.view<tag::EngineManager>()) {

                const auto & mouse = registry.get_component<comp::MouseTracker>(entity);
                mouse_position = mouse.screen_mouse_position;     
            }

            auto dist = [&](Vec2 p1, Vec2 p2) -> float {
                return std::sqrt(((p2.x - p1.x) * (p2.x - p1.x)) + ((p2.y - p1.y) * (p2.y - p1.y)));
            };

            // Here --> ... Is there any way to make these nodes draggablle without introducing state and doing
            // a bunch of that?

            Vec2 corners[8] = {
                    clamp.player_zone_top_left,
                    {clamp.player_zone_bottom_right.x, clamp.player_zone_top_left.y},
                    clamp.player_zone_bottom_right,
                    {clamp.player_zone_top_left.x, clamp.player_zone_bottom_right.y},

                    clamp.clamp_top_left,
                    {clamp.clamp_bottom_right.x, clamp.clamp_top_left.y},
                    clamp.clamp_bottom_right,
                    {clamp.clamp_top_left.x, clamp.clamp_bottom_right.y}
                };

                Vec2 mouse_world = {
                    cam.x + ((mouse_position.x - config::GAME_WORLD_WIDTH * 0.5f) / zoom),
                    cam.y + ((mouse_position.y - config::GAME_WORLD_HEIGHT * 0.5f) / zoom)
                };

                float min_dist = 15.0f / zoom; // scale with zoom
                int hovered = -1;

                for (int i = 0; i < 8; i++) {
                    if (dist(mouse_world, corners[i]) < min_dist) {
                        hovered = i;
                        break;
                    }
                }

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hovered != -1) {
                    active_corner = hovered;
                    active_clamp = &clamp;
                }

                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                    active_corner = -1;
                    active_clamp = nullptr;
                }

                if (active_clamp == &clamp && active_corner != -1) {

                    Vec2* tl;
                    Vec2* br;

                    if (active_corner < 4) {
                        tl = &clamp.player_zone_top_left;
                        br = &clamp.player_zone_bottom_right;
                    } else {
                        tl = &clamp.clamp_top_left;
                        br = &clamp.clamp_bottom_right;
                    }

                    int c = active_corner % 4;

                    if (c == 0) { // top-left
                        tl->x = mouse_world.x;
                        tl->y = mouse_world.y;
                    }
                    else if (c == 1) { // top-right
                        br->x = mouse_world.x;
                        tl->y = mouse_world.y;
                    }
                    else if (c == 2) { // bottom-right
                        br->x = mouse_world.x;
                        br->y = mouse_world.y;
                    }
                    else if (c == 3) { // bottom-left
                        tl->x = mouse_world.x;
                        br->y = mouse_world.y;
                    }
                }


                for (int i = 0; i < 8; i++) {
                    renderer.rdraw_wfrect(
                        corners[i].x - 4,
                        corners[i].y - 4,
                        8, 8,
                        (i == hovered ? RED : BLUE),
                        2.0f
                    );
                }



        }
    }








    // STEP DRAW LOOP: Draw all PhysicsBody hitboxes

    for (Entity entity : registry.view<comp::Transform, comp::PhysicsBody>()) {      // For each iteration of Entity


            comp::Transform & transform     = registry.get_component<comp::Transform>(entity);
            comp::PhysicsBody & body        = registry.get_component<comp::PhysicsBody>(entity);


        


            if (body.render_hitbox) {
                Color col_body;
                if (body.inColl) {
                    col_body = Color({0, 255, 255, 110});
                } else {
                    col_body = Color({0, 255, 0, 110});
                }

                Color col_skin;
                if (body.innerSkinInColl) {
                    col_skin = Color({255, 0, 255, 180});
                } else {
                    col_skin = Color({255, 0, 0, 180});
                }

                

                Vec2 top_left;
                Vec2 bottom_right;
                
                top_left.x = transform.position.x - (body.size.x / 2.0f);
                top_left.y = transform.position.y - (body.size.y / 2.0f);
                bottom_right.x = transform.position.x + (body.size.x / 2.0f);
                bottom_right.y = transform.position.y + (body.size.y / 2.0f);

                renderer.rdraw_rect(
                    top_left.x + (body.skin), 
                    top_left.y + (body.skin), 
                    body.size.x - (body.skin*2.0f), 
                    body.size.y - (body.skin*2.0f), 
                    col_skin
                );

                renderer.rdraw_rect(
                    top_left.x, 
                    top_left.y, 
                    body.size.x, 
                    body.size.y, 
                    col_body
                );

                



            }


    }

}


