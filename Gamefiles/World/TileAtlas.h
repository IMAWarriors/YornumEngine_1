// TileAtlas.h

#ifndef TILEATLAS_H
#define TILEATLAS_H

#include "Tile.h"
#include "../../Engine/Core/Overhead/GameTypes.h"

#include "raylib.h"
#include <utility>

#include <cstdint>
#include <string>
#include <vector>

enum class CollisionType {
    COLL_EMPTY = 0,
    COLL_FULL_SOLID,
    COLL_PSLOPE1_SOLID,
    COLL_NSLOPE1_SOLID,
    COLL_FULL_SEMISOLID
};


struct TileData {

    int             anim_parent_index;
    bool            is_anim_tile;
    int             anim_frame_count;
    float           anim_frame_speed;
    CollisionType   collision_data;
};




struct TileAtlas {

    

    std::string name;
    Texture2D * image_sheet_source = nullptr;
    std::vector<TileData> tile_data;
    size_t tile_atlas_width;        // Px
    size_t tile_atlas_height;       // Px
    size_t tile_size;               // Px?, starting in top left hand corner i think
    size_t tile_count;
    size_t tiles_per_row;           // Width of page in tiles
    size_t tiles_per_col;           // Height of page in tiles


    TileAtlas () {
        image_sheet_source = nullptr;
    }

    TileAtlas (const std::string & _name, Texture2D & _img, size_t _tsize, size_t _tpr, size_t _tpc) {

        name = _name;
        split_tileset(_img, _tsize, _tpr, _tpc);
        
    }

    // Has empty tile data?
    bool is_empty () {
        if (image_sheet_source == nullptr) {
            return true;
        }
        return false;
    }


    // Check if split is valid
    bool is_split_valid (Texture2D & _image_sheet_source, size_t _tile_px_size, size_t _tiles_per_row, size_t _tiles_per_col) {

        image_sheet_source = &_image_sheet_source;

        tile_atlas_width = image_sheet_source->width;
        tile_atlas_height = image_sheet_source->height;

        bool valid_tsize = (256 >= _tile_px_size && _tile_px_size > 0);
        bool valid_width = tile_atlas_width >= (_tiles_per_row * _tile_px_size);
        bool valid_height = tile_atlas_height >= (_tiles_per_col * _tile_px_size);

        if (!(valid_tsize && valid_width && valid_height)) {
            image_sheet_source = nullptr;
        }

        return (valid_tsize && valid_width && valid_height);
    }

    // Attempt to split and return if it was successful
    bool split_tileset (Texture2D & _image_sheet_source, size_t _tile_px_size, size_t _tiles_per_row, size_t _tiles_per_col) {
        if (!is_split_valid(_image_sheet_source, _tile_px_size, _tiles_per_row, _tiles_per_col)) {
            return false;
        }
        
        tile_size = _tile_px_size;
        tiles_per_row = _tiles_per_row;
        tiles_per_col = _tiles_per_col;
        tile_count = tiles_per_row * tiles_per_col;

        tile_data.clear();
        tile_data.resize( tile_count, {-1, false, 0, 0.0f, CollisionType::COLL_EMPTY} );

        return true;
    }

    void set_tile_collision_data (size_t _tile_idx, CollisionType _coll_data) {
        tile_data[_tile_idx].collision_data = _coll_data;
    }

    CollisionType get_tile_collision_data (size_t _tile_idx) {
        return tile_data[_tile_idx].collision_data;
    }

    bool is_tile_animated (size_t _tile_idx) {
        return tile_data[_tile_idx].is_anim_tile;
    }

    bool is_tile_animation_child(size_t idx) const {
        return tile_data[idx].anim_parent_index >= 0 && tile_data[idx].anim_parent_index != (int)idx;
    }

    bool is_valid_tile_index (int idx) const {
        return idx >= 0 && idx < (int)tile_data.size();
    }

    int get_animation_parent_index (int idx) const {
        if (!is_valid_tile_index(idx)) {
            return -1;
        }

        int parent_idx = tile_data[idx].anim_parent_index;
        if (parent_idx >= 0) {
            return parent_idx;
        }

        return idx;
    }

    void reset_all_tile_animation () {
        for (TileData & data : tile_data) {
            data.is_anim_tile = false;
            data.anim_parent_index = -1;
            data.anim_frame_count = 0;
            data.anim_frame_speed = 0.0f;
        }
    }

    void clear_tile_animation (size_t _tile_idx) {

        // Store the Parent Tile of Animation
        TileData & tempdata = tile_data[_tile_idx];

        // Cycle through Child Tiles in the animation and free them from animation binding
        size_t cursor = _tile_idx + 1;

        size_t max_frames = std::min(
            (size_t)tempdata.anim_frame_count,
            tile_data.size() - (_tile_idx + 1)
        );


        for (size_t i = 0; i < max_frames; i++) {
            // Force clear for safety
            tile_data[cursor].anim_parent_index = -1; // Releases tile's role as an intermediate animation
            tile_data[cursor].anim_frame_count = 0;
            tile_data[cursor].anim_frame_speed = 0.0f;
            tile_data[cursor].is_anim_tile = false;
            
            cursor++;
        }

        tile_data[_tile_idx] = {-1, false, 0, 0.0f, tempdata.collision_data}; // Releases Parent Tile's role as Parent Tile Animation

        
    }

    bool are_anim_params_valid (size_t _tile_idx, uint32_t _total_anim_frames, float _anim_frametime) {

        if (_anim_frametime <= 0) { return false; }

        if (_tile_idx + _total_anim_frames > tile_data.size()) { return false; }

        size_t cursor = _tile_idx;
        for (int i = 0; i < _total_anim_frames; i++) {
            if (tile_data[cursor].is_anim_tile && _tile_idx != cursor) {
                return false;
            } else {
                cursor++;
            }
        }

        return true;

    }

    void assign_tile_animation (size_t _tile_idx, uint32_t _total_anim_frames, float _anim_frametime) {

        if (_total_anim_frames == 0 || _anim_frametime == 0.0f) { // Unassign animation
            tile_data[_tile_idx].is_anim_tile     = false;
            tile_data[_tile_idx].anim_frame_count = 0;
            tile_data[_tile_idx].anim_frame_speed = 0.0f;
            tile_data[_tile_idx].anim_parent_index = -1;

            for (TileData & data : tile_data) {
                if (data.anim_parent_index == _tile_idx) {
                    data.is_anim_tile = false;
                    data.anim_parent_index = -1;
                    data.anim_frame_count = 0;
                    data.anim_frame_speed = 0.0f;
                }
            }

            return;
        }

        if (tile_data[_tile_idx].is_anim_tile) {        // REASSIGN animation when animation already exists, meaning clear all previous children
            for (TileData & data : tile_data) {
                if (data.anim_parent_index == _tile_idx) {
                    data.is_anim_tile = false;
                    data.anim_parent_index = -1;
                    data.anim_frame_count = 0;
                    data.anim_frame_speed = 0.0f;
                }
            }
        }

        tile_data[_tile_idx].is_anim_tile     = true;
        tile_data[_tile_idx].anim_frame_count = _total_anim_frames;
        tile_data[_tile_idx].anim_frame_speed = _anim_frametime;
        tile_data[_tile_idx].anim_parent_index = -1;

        size_t cursor = _tile_idx;

        for (int i = 0; i < _total_anim_frames; i++) {

            if (tile_data[cursor].anim_parent_index != -1 /*|| tile_data[cursor].is_anim_tile*/) {
                return; // or assert
            }

            tile_data[cursor].anim_parent_index = _tile_idx;
            tile_data[cursor].anim_frame_count = 0;
            tile_data[cursor].anim_frame_speed = 0.0f;
            tile_data[cursor].is_anim_tile     = false;
            cursor++;
        }
    }

    

    
    int getTileIdx (int _c, int _r) const {
        if (0 > _c || _c >= tiles_per_row) { return -1; }
        if (0 > _r || _r >= tiles_per_col) { return -1; }
        return _c + (_r * tiles_per_row);
    }
    
    
        
    Rectangle getRectCR (int _c, int _r) const {
        float x = (float)_c * (float)tile_size;
        float y = (float)_r * (float)tile_size;

        float width = (float)tile_size;
        float height = (float)tile_size;

        return {x,y,width,height};
    }
    
    Rectangle getRectCR (float current_tile_frame_time, int _c, int _r) const {

        int index = getTileIdx(_c, _r);

        if (index < 0 || index >= (int)tile_data.size()) {
            return {0,0,0,0}; // or assert
        }
        
        if (tile_data[index].is_anim_tile == false) {

            float x = (float)_c * (float)tile_size;
            float y = (float)_r * (float)tile_size;

            float width = (float)tile_size;
            float height = (float)tile_size;

            return {x,y,width,height};
        } else {

            const TileData& data = tile_data[index];

            float max_frame_time_anim = data.anim_frame_count * data.anim_frame_speed;

            if (data.anim_frame_count <= 0 || data.anim_frame_speed <= 0.0f) {
                // Invalid animation → just render base tile
                return getRectCR(_c, _r);
            }

            float frame_time = current_tile_frame_time;
            while (frame_time > max_frame_time_anim) {
                frame_time -= max_frame_time_anim;
            }

            float anim_progress;
            if (max_frame_time_anim > 0.0f) {
                anim_progress = (frame_time / max_frame_time_anim) * data.anim_frame_count;
            } else {
                anim_progress = 0.0f;
            }

            int tile_anim_frame = (int)anim_progress;

            if (tile_anim_frame >= data.anim_frame_count) {
                tile_anim_frame = data.anim_frame_count - 1;
            }

            index += tile_anim_frame;

            int src_column = index % tiles_per_row;
            int src_row    = index / tiles_per_row;

            float x = (float)src_column * (float)tile_size;
            float y = (float)src_row * (float)tile_size;

            float width = (float)tile_size;
            float height = (float)tile_size;

            return {x,y,width,height};
        }

    }

    Rectangle getRect (int _index) const {

        int src_column = _index % tiles_per_row;
        int src_row    = _index / tiles_per_row;

        return getRectCR (src_column, src_row);

    }

    Rectangle getRect (float current_tile_frame_time, int _index) const {

        int src_column = _index % tiles_per_row;
        int src_row    = _index / tiles_per_row;

        return getRectCR (current_tile_frame_time, src_column, src_row);

    }

    
        



        
    

    
    

};

#endif