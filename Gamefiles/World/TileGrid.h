// TileGrid.h

#ifndef TILEGRID_H
#define TILEGRID_H

#include "Overhead/Gwconst.h"
#include "../../Engine/Core/Overhead/GameTypes.h"
#include "Tile.h"
#include "TileAtlas.h"

#include <vector>
#include <stdexcept>
#include <cmath>

class Scene;

class TileGrid {

    private:

        Tile tilegrid[gwconst::WORLD_TILEGRID_WIDTH][gwconst::WORLD_TILEGRID_HEIGHT];


    public:

        TileGrid ();

        Tile & get_tile (int col, int row); // general mutator

        const Tile & get_tile (int col, int row) const; // general mutator

        Tile & get_tile_world_pos (Vec2 pos);


        int translate_world_x_col (float world_x) const;
        int translate_world_y_row (float world_y) const;



        float raycast (const Scene & scene, Vec2 start_wc, Vec2 step, int max_steps);



};







#endif







