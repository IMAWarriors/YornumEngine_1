// TileGrid.cpp

#include "TileGrid.h"




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

            tile_ptr = &tilegrid[col + 320][row + 180];
            return *tile_ptr;

    } else {
        
        throw std::runtime_error("ERROR: TRIED TO ACCESS TILE THAT DOES NOT EXIST");

    }


}

TileGrid::TileGrid () {
    for (int TILE_COL_IDX = -(gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE); TILE_COL_IDX <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE; TILE_COL_IDX++) {
        for (int TILE_ROW_IDX = -(gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE); TILE_COL_IDX <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE; TILE_COL_IDX++) {
            get_tile(TILE_COL_IDX, TILE_ROW_IDX) = {0};
        }
    }
}










