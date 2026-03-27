// Scene.h

#ifndef SCENE_H
#define SCENE_H

#include "Overhead/Gwconst.h"
#include "../../Engine/Core/Overhead/GameTypes.h"

#include "TileAtlas.h"
#include "TileGrid.h"

#include <vector>
#include <array>
#include <cstdint>
#include <string>

#include <cstdlib>   // for rand(), srand()
#include <ctime>

class Scene {

    public:

        // Editor Wiring
        int EDITOR_ONLY_SELECTED_ATLAS = -1;
        int EDITOR_ONLY_SELECTED_PALLET_TILE = -1;
        bool EDITOR_ONLY_ACTIVE_TAEDITOR = false;


        std::string loaded_scene_name;
        std::vector<TileAtlas> loaded_atlases;
        std::vector<TileGrid> tile_layers;

        Tile get_tile (size_t layer, int column, int row);
        void set_tile (size_t layer, int column, int row, Tile newtile);

        void tiles_clear_all ();
        void tiles_clear (size_t layer);

        void tiles_load_garbagedata (size_t layer, int atlasIndex) {

            for (int row = gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE; row <= gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE; row++) {

                for (int col = gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE; col <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE; col++) {

                    Tile & tile = tile_layers[layer].get_tile(col, row);
                    size_t atlasMaxIndex = loaded_atlases[atlasIndex].getTileIdx(
                        loaded_atlases[atlasIndex].tiles_per_row - 1, 
                        loaded_atlases[atlasIndex].tiles_per_col - 1);

                    int random_tile_index = rand() % (atlasMaxIndex+1);

                    tile = {atlasIndex, random_tile_index};  

                }

            }


        }


        void tiles_push_new_layer () {

            tile_layers.push_back(TileGrid());
            // For now just garbage data them when loading too

            tiles_load_garbagedata(0, 0);

        }


        void load_new_tileset (const std::string & _name, Texture2D & _image, size_t _tile_px_size, size_t _tiles_per_row, size_t _tiles_per_col) {
            
            loaded_atlases.push_back(TileAtlas(_name, _image, _tile_px_size, _tiles_per_row, _tiles_per_col));

        }

        bool is_valid_atlas_index (int atlas_idx) const {
            return atlas_idx >= 0 && atlas_idx < (int)loaded_atlases.size();
        }

        int normalize_tile_to_animation_parent (int atlas_idx, int tile_idx) const {
            if (!is_valid_atlas_index(atlas_idx)) {
                return -1;
            }

            return loaded_atlases[atlas_idx].get_animation_parent_index(tile_idx);
        }

        void remap_layer_tiles_to_animation_parent (int atlas_idx, int animation_parent_idx, uint32_t animation_frame_count) {
            if (!is_valid_atlas_index(atlas_idx)) {
                return;
            }
            if (animation_frame_count <= 1) {
                return;
            }

            const int first_child = animation_parent_idx + 1;
            const int last_child = animation_parent_idx + ((int)animation_frame_count - 1);

            for (TileGrid & layer : tile_layers) {
                for (int row = gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE; row <= gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE; row++) {
                    for (int col = gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE; col <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE; col++) {
                        Tile & tile = layer.get_tile(col, row);

                        if (tile.atlas_idx != atlas_idx) {
                            continue;
                        }

                        if (tile.tile_idx >= first_child && tile.tile_idx <= last_child) {
                            tile.tile_idx = animation_parent_idx;
                        }
                    }
                }
            }
        }

        bool apply_tile_animation (int atlas_idx, int tile_idx, uint32_t animation_frame_count, float animation_frametime) {
            if (!is_valid_atlas_index(atlas_idx)) {
                return false;
            }

            TileAtlas & atlas = loaded_atlases[atlas_idx];
            if (!atlas.is_valid_tile_index(tile_idx)) {
                return false;
            }

            if (!atlas.are_anim_params_valid(tile_idx, animation_frame_count, animation_frametime)) {
                return false;
            }

            atlas.assign_tile_animation(tile_idx, animation_frame_count, animation_frametime);
            remap_layer_tiles_to_animation_parent(atlas_idx, tile_idx, animation_frame_count);
            return true;
        }





};





#endif


