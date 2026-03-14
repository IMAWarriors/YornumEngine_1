// TileGrid.h

#ifndef TILEGRID_H
#define TILEGRID_H

#include "Overhead/Gwconst.h"
#include "Tile.h"

#include <stdexcept>

class TileGrid {

    private:

        Tile tilegrid[gwconst::WORLD_TILEGRID_WIDTH][gwconst::WORLD_TILEGRID_HEIGHT];

        Tile & get_tile (int col, int row);





    public:

        TileGrid ();





};







#endif







