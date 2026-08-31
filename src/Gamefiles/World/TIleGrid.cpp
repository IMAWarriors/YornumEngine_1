// TileGrid.cpp

#include "TileGrid.h"

#include "Scene.h"


TileGrid::TileGrid () : tilegrid(gwconst::WORLD_TILEGRID_WIDTH * gwconst::WORLD_TILEGRID_HEIGHT) {

}









Tile & TileGrid::get_tile (int col, int row) {      

    if (gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE <= col && col <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE &&
        gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE <= row && row <= gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE ) {

            const int grid_col = col - gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE;
            const int grid_row = row - gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE;
            return tilegrid[grid_col * gwconst::WORLD_TILEGRID_HEIGHT + grid_row];

    } else {
        
        throw std::runtime_error("ERROR: TRIED TO ACCESS TILE THAT DOES NOT EXIST");

    }


}







const Tile& TileGrid::get_tile(int col, int row) const {

    if (gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE <= col && col <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE &&
        gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE <= row && row <= gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE) {

        const int grid_col = col - gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE;
        const int grid_row = row - gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE;
        return tilegrid[grid_col * gwconst::WORLD_TILEGRID_HEIGHT + grid_row];

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




CollisionType TileGrid::get_tile_coll (Scene & scene, int col, int row) const {

    if (col < gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE || col > gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE ||
        row < gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE || row > gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE) {
        return CollisionType::COLL_FULL_SOLID;
    }

    const Tile tile_hit = get_tile(col, row);

    if (tile_hit.atlas_idx < 0 || tile_hit.tile_idx < 0) {
        return CollisionType::COLL_EMPTY;
    }

    if (tile_hit.atlas_idx >= (int)scene.loaded_atlases.size()) {
        return CollisionType::COLL_EMPTY;
    }

    const TileAtlas & atlas = scene.loaded_atlases[tile_hit.atlas_idx];

    if (tile_hit.tile_idx >= (int)atlas.tile_data.size()) {
        return CollisionType::COLL_EMPTY;
    }

    return atlas.tile_data[tile_hit.tile_idx].collision_data;
}


CollisionType TileGrid::get_tile_coll_pos (Scene & scene, Vec2 position) const {

    int col = translate_world_x_col(position.x);
    int row = translate_world_y_row(position.y);

     if (col < gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE || col > gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE ||
        row < gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE || row > gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE) {
        return CollisionType::COLL_FULL_SOLID;
    }

    const Tile tile_hit = get_tile(col, row);

    if (tile_hit.atlas_idx < 0 || tile_hit.tile_idx < 0) {
        return CollisionType::COLL_EMPTY;
    }

    if (tile_hit.atlas_idx >= (int)scene.loaded_atlases.size()) {
        return CollisionType::COLL_EMPTY;
    }

    const TileAtlas & atlas = scene.loaded_atlases[tile_hit.atlas_idx];

    if (tile_hit.tile_idx >= (int)atlas.tile_data.size()) {
        return CollisionType::COLL_EMPTY;
    }

    return atlas.tile_data[tile_hit.tile_idx].collision_data;
}






