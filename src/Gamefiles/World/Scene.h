// Scene.h

#ifndef SCENE_H
#define SCENE_H

#include "Overhead/Gwconst.h"
#include "../../Engine/Core/Overhead/GameTypes.h"
#include "../Assets/AssetManager.h"



#include "TileAtlas.h"
#include "TileGrid.h"

#include <vector>
#include <array>
#include <cstdint>
#include <string>

#include <cstdlib>   // for rand(), srand()
#include <ctime>

#include <fstream>   // file streams
#include <sstream>   // string streams (parsing)

#include <memory>

class Scene;

namespace serial {
    bool SaveSceneToFile(const Scene&, const std::string&);
    bool LoadSceneFromFile(Scene&, AssetManager&, const std::string&);
}



class Scene {

    private:

        std::string DEFAULT_PATH_SCENEFILES = "assets/scenes/";



    public:

        // Editor Wiring
        int EDITOR_ONLY_SELECTED_ATLAS = -1;
        int EDITOR_ONLY_SELECTED_PALLET_TILE = -1;
        int EDITOR_ONLY_SELECTED_LAYER = 0;
        bool EDITOR_ONLY_ACTIVE_TAEDITOR = false;

        bool uiCapturesMouse = false;

        std::vector<std::unique_ptr<Texture2D>> texture_dependencies;

        // ****************************** SERIALIZED ATTRIBUTES ******************************
        //      SAVE DATA FOR SCENES
        //  ----------------------------------------------------------------------------------

        std::string loaded_scene_name;
        std::vector<TileAtlas> loaded_atlases;
        std::vector<TileGrid> tile_layers; 

        // ====================================================================================
        

        void new_scene () {
            loaded_scene_name   = "untitled_scene";
            loaded_atlases.clear();
            tile_layers.clear();
        }

        bool load_scene (const std::string & _path, AssetManager & _assets) {

            // Needs assets ref to be passed because when loading a scene,
            // all assets are automatically unloaded and assets needed for the scene are loaded

            bool success = false;

            _assets.UnloadAllTilesetTextureAssets();

            success = serial::LoadSceneFromFile(*this, _assets, _path);

            return success;

        }

        bool load_scene_by_name (const std::string & _name, AssetManager & _assets) {

            // Needs assets ref to be passed because when loading a scene,
            // all assets are automatically unloaded and assets needed for the scene are loaded

            bool success = false;

            int _extIdx = -1;
            for (int i=0;i<_name.size();i++) {
                if (_name[i]=='.') {
                    _extIdx = i;
                    break;
                }
            }

            std::string _path;

            if (_extIdx >= 0) {
                _path = _name.substr(0,_extIdx);
            } else {
                _path = _name;
            }
             
            _path = std::string(DEFAULT_PATH_SCENEFILES + _path + ".scene");

            _assets.UnloadAllTilesetTextureAssets();

            success = serial::LoadSceneFromFile(*this, _assets, _path);

            return success;

        }

        bool save_scene (const std::string & _path) {

            bool success = false;

            success = serial::SaveSceneToFile(*this, _path);

            return success;

        }


        // ******************************************************************
        //  UNDEFINED FUCNTIONS 
        // ------------------------------------->
        //
        /*      Tile get_tile (size_t layer, int column, int row);
                void set_tile (size_t layer, int column, int row, Tile newtile);

                void tiles_clear_all ();
                void tiles_clear (size_t layer);
                
        */

        // *******************************************************************

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

        void tiles_load_emptydata (size_t layer) {

            for (int row = gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE; row <= gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE; row++) {

                for (int col = gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE; col <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE; col++) {

                    Tile & tile = tile_layers[layer].get_tile(col, row);
                    tile = {-1, -1};  

                }
            }

        }


        void tiles_push_new_layer () {

            tile_layers.push_back(TileGrid());
            // For now just garbage data them when loading too

            tiles_load_emptydata(tile_layers.size()-1);

        }

        // To import a tileset to assets and load the tioleset
        void import_new_tileset (AssetManager & _assets, std::string & _name, const std::string & _imgpath, size_t _tile_px_size, size_t _tiles_per_row, size_t _tiles_per_col) {

            // Probably save the dependency or smth later idk i have to figure out how raylib naturall handles textures
            // so iknow what to do with my textures and how to do it
            //


            loaded_atlases.push_back(TileAtlas(_name, _assets.LoadTilesetTexture(_imgpath), _tile_px_size, _tiles_per_row, _tiles_per_col));
            loaded_atlases[loaded_atlases.size()-1].imgpath = _imgpath;

        }

        // To actively load a passively loaded tileset from assets
        void load_new_tileset (const std::string & _name, Texture2D & _image, size_t _tile_px_size, size_t _tiles_per_row, size_t _tiles_per_col, const std::string & _imgpath = "") {
            
            loaded_atlases.push_back(TileAtlas(_name, _image, _tile_px_size, _tiles_per_row, _tiles_per_col));
            loaded_atlases.back().imgpath = _imgpath;

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


        int translate_world_x_col (float world_x) const {
            return (int)(std::floor(world_x / gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS));
        }

        int translate_world_y_row (float world_y) const {
            return (int)(std::floor(world_y / gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS));
        }

        Vec2 get_tile_pos_UL (int col, int row) const {
            Vec2 position = {(float)(col * gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS), (float)(row * gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS)};
            return position;
        }

        int get_tilesize () const {
            return gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS;
        }








};





#endif


