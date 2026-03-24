// TileAtlas.h

#ifndef TILEATLAS_H
#define TILEATLAS_H

#include "Tile.h"
#include "../../Engine/Core/Overhead/GameTypes.h"

#include "raylib.h"

#include <cstdint>
#include <string>



struct TileAtlas {

    // Don't really know how to store this yet but the tilepage will basically hold a vector
    // of all the images that we could have for all the tiles of this particular tilepage
    // We also have to hold just hte actual original source image i think idk how that works either

    // ... ../Assets/Sprites/Tilesets/tilset_test_1_A.png

    std::string name;
    Texture2D * image_sheet_source;

    // std::vector<TileImageSlice> tile_images; // not really any reason to store this data cuz we can do this math on the fly

    size_t tile_atlas_width; // Px
    size_t tile_atlas_height; // Px

    size_t tile_size; // Px?, starting in top left hand corner i think
    size_t tile_count;

    size_t tiles_per_row;   // Width of page in tiles
    size_t tiles_per_col;   // Height of page in tiles

    // Input type for each TileAtlas should be (in terms of important designer details):
    //  TILE SIZE:      How big are the tiles
    //  TILES PER ROW:  How many tiles you expect per row
    //  TILES PER COL:  How many tiles you should expect per column
    //  IMAGE        :  The actual image source we wanna store i guess too bc we gotta split it up

    TileAtlas (const std::string & _name, Texture2D & _image, size_t _tile_px_size, size_t _tiles_per_row, size_t _tiles_per_col) {
        // Placeholder for actual image storage
        image_sheet_source = &_image;
        name = _name;

        tile_size = _tile_px_size;
        tiles_per_row = _tiles_per_row;
        tiles_per_col = _tiles_per_col;

        // Predicted values (might not be right with incorrect image sizing, must check validity somehow)
        // some point but calculations should be accurate if images are loaded correctly
        // 
        // Also techinically tile count is just measuring max tile count cuz some of the last tiles in the set
        // could be gagrbage or smth, but we'll just ignore those tiles and not use them in levels in that case

        tile_atlas_width = tile_size * tiles_per_row;
        tile_atlas_height = tile_size * tiles_per_col;
        tile_count = tiles_per_row * tiles_per_col;
    }

    //
    //  Pretty self explanatory, but remember Tile Size is measured in pixels
    //  Tiles per row/col are measured in Tiles (which are thereby kinda measured in pixels you can imagine)
    //
    //  Basically instructions for primitive tileslicer where all tilesheets must be strictly defined where every
    //  tile is the same size, starts at the top left hand corner going down to the right
    //
    //  Tile page width and height i guess would just be calcluated by the program of the image size orwtv when we get the image
    //  and we process if theres an issue loading the tiles, i.e. the tilepage's width or height is NOT the congruent or consistent
    //  with the tile size we expect to clip or the tile count per row / col.

    // Little helper function to just plug in the tiles' unique index in the tilepage according to Column and Row
    // WILL PRODUCE OUTPUT -1 WITH NONSENSE COLUMN AND ROW INPUTS
    int getTileIdx (int _c, int _r) const {
        if (0 > _c || _c >= tiles_per_row) { return -1; }
        if (0 > _r || _r >= tiles_per_col) { return -1; }
        return _c + (_r * tiles_per_row);
    }
    


    // Gets the source rectangle on the tilesheet of where we want the tile based on basically just index

    
        
    Rectangle getRectCR (int _c, int _r) const {

        float x = (float)_c * (float)tile_size;
        float y = (float)_r * (float)tile_size;

        float width = (float)tile_size;
        float height = (float)tile_size;


        return {x,y,width,height};

    }

    Rectangle getRect (int _index) const {

        int src_column = _index % tiles_per_row;
        int src_row    = _index / tiles_per_row;

        return getRectCR (src_column, src_row);

    }
    

};

#endif