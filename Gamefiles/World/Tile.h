// Tile.h

#include <cstdint>

#ifndef TILE_H
#define TILE_H

// ==========================================
// Tiles are pretty much just defiend in terms of their
// atlas and are references to atlas indecies based on the active
// index.

struct Tile {
    int atlas_idx;
    int tile_idx;

    Tile () {
        atlas_idx   = -1;
        tile_idx    = -1;
    }

    Tile (int _atlas_idx, int _tile_idx) {
        atlas_idx   = _atlas_idx;
        tile_idx    = _tile_idx;
    }
};












#endif
