// TileGrid.cpp

#include "TileGrid.h"

#include "Scene.h"


TileGrid::TileGrid () {

    // Outer Loop, loop over Columns adjusting every X value 

    for (int TILE_COL_IDX = gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE; 
             TILE_COL_IDX <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE; 
             TILE_COL_IDX++) {

                // Inner loop, loop over each column's Row values (the Y values in each Column)
        
                for (int TILE_ROW_IDX = (gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE); 
                         TILE_ROW_IDX <= gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE; 
                         TILE_ROW_IDX++) {


                    get_tile(TILE_COL_IDX, TILE_ROW_IDX) = {-1,-1};

                }

    }

}









Tile & TileGrid::get_tile (int col, int row) {      

    /** GET_TILE 
     * ----------------------------------------------------------------------------
     * Convert col,row (x,y) coordinates for tiles into a reference we can seat
     * for a tile i guess thatsz in the tilegrid and change it or get it yk
     * 
     * More importantly, we cannot do this raw with [][] because we are working with
     * a raw 2D array and there shouldn't be access tot his, i'm using OOP 
     * encapsulation to avoid this and Force access utilizng my coordinate system. 
     * Here is a rundown of conversions
     * ----------------------------------------------------------------------------
     * 
     *  So if I access:     get_tile(320, 180)    -->       tilegrid[640][360]
     * 
     *  And if I access:    get_tile(-320, -180)  -->       tilegrid[0][0]
     * 
     *  Therfore, access:   get_tile(0,0)    will get you   tilegrid[320][180]
     * 
     *  The linear relationship is now apparent, and we can easily map the conversion to
     *  being that the      get_tile$input.x + 320 -> tilegrid$desired.x 
     *                      get_tile$input.y + 180 -> tilegrid$desired.y
     *                       // Input Side            // Access Side
     * 
     **/ 
    
    

    Tile * tile_ptr = nullptr;

    if (gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE <= col && col <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE &&
        gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE <= row && row <= gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE ) {

            tile_ptr = &tilegrid[col + gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE][row + gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE];
            return *tile_ptr;

    } else {
        
        throw std::runtime_error("ERROR: TRIED TO ACCESS TILE THAT DOES NOT EXIST");

    }


}







const Tile& TileGrid::get_tile(int col, int row) const {

    if (gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE <= col && col <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE &&
        gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE <= row && row <= gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE) {

        return tilegrid[col + gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE][row + gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE];

    } else {
        throw std::runtime_error("ERROR: TRIED TO ACCESS TILE THAT DOES NOT EXIST");
    }
}



Tile & TileGrid::get_tile_world_pos (Vec2 pos) {

    // Get tile by coordinates

    int column;
    int row;

    float position_x = pos.x;
    float position_y = pos.y;

    column = (int)(std::floor(position_x / gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS));
    row    = (int)(std::floor(position_y / gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS));

    Tile & tile = get_tile (column, row);
    
    return tile;
    
}



int TileGrid::translate_world_x_col (float world_x) const {
    return (int)(std::floor(world_x / gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS));
}

int TileGrid::translate_world_y_row (float world_y) const {
    return (int)(std::floor(world_y / gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS));
}

float TileGrid::raycast (const Scene & scene, Vec2 start_wc, Vec2 step, int max_steps) {


    Vec2 ray_position = start_wc;
    float distance = -1.0f;


    auto colltype = [&] (Vec2 pos) -> CollisionType {
        int ray_wcol = translate_world_x_col (pos.x);
        int ray_wrow = translate_world_y_row (pos.y);
        Tile tile_hit = get_tile(ray_wcol, ray_wrow);
        CollisionType tile_colltype = (tile_hit.atlas_idx<0||tile_hit.tile_idx<0) ? CollisionType::COLL_EMPTY : scene.loaded_atlases[tile_hit.atlas_idx].tile_data[tile_hit.tile_idx].collision_data;
        return tile_colltype;
    };


    if (colltype(ray_position) == CollisionType::COLL_FULL_SOLID) {
      // -1.0f
        return distance;

    }

    int step_count = 0;
    
    for (int i = 0; i < max_steps; i++) {


        step_count++;

        ray_position.x += step.x;
        ray_position.y += step.y;

        distance += std::sqrt((step.x*step.x) + (step.y*step.y));

        if (colltype(ray_position) == CollisionType::COLL_FULL_SOLID) {
            break;    
        }
    }

    return distance;

}

