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
};












#endif
