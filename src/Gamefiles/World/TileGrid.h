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

        // Tile layers are large (over 230,000 tiles).  Keeping their storage on
        // the heap makes moving a layer cheap and prevents vector growth from
        // copying multi-megabyte grids or placing one on the stack.
        std::vector<Tile> tilegrid;

    public:

        // Default constructor
        TileGrid ();

        // Move Only, delete copy constructor + delete copy assignment constructor
        TileGrid(const TileGrid&) = delete;
        TileGrid& operator=(const TileGrid&) = delete;

        // Move Only move constructor + move assignment operator
        TileGrid(TileGrid&&) noexcept = default;
        TileGrid& operator=(TileGrid&&) noexcept = default;

        // Accessors
        Tile & get_tile (int col, int row); // general mutator
        const Tile & get_tile (int col, int row) const; // view only
        Tile & get_tile_world_pos (Vec2 pos);

        // Translate ROW or COL to WORLD_X or WORLD_Y
        int translate_world_x_col (float world_x) const;
        int translate_world_y_row (float world_y) const;

        // Get the type of Collision here
        CollisionType get_tile_coll (Scene & scene, int col, int row) const;
        CollisionType get_tile_coll_pos (Scene & scene, Vec2 position) const;

        // Get a group of collision data within a rectangle
        std::vector<CollisionType> get_tile_coll_rect (Scene & scene, Vec2 rect_bottomleft, Vec2 rect_size) const;



};







#endif







