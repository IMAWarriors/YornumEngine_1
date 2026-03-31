// SceneSerialization.cpp


#include "SceneSerialization.h"

#include <filesystem>

bool serial::SaveSceneToFile(const Scene & scene, const std::string& filepath) {

    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    // ================= HEADER =================
    SceneFileHeader header = {{'S','C','E','N','E','\0'}, 0};
    file.write((char*)&header, sizeof(header));

    // ================= LAYER INFO =================
    int layer_header[7] = {
        (int)scene.tile_layers.size(),
        gwconst::SCREEN_WIDTH_GAMEPIXELS,
        gwconst::SCREEN_HEIGHT_GAMEPIXELS,
        gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE,
        gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE,
        gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE,
        gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE
    };

    file.write((char*)layer_header, sizeof(layer_header));

    // ================= ATLAS COUNT =================
    int atlas_count = (int)scene.loaded_atlases.size();
    file.write((char*)&atlas_count, sizeof(int));

    // ================= ATLASES =================
    for (const TileAtlas & atlas : scene.loaded_atlases) {

        // --- name ---
        int name_len = (int)atlas.name.size();
        file.write((char*)&name_len, sizeof(int));
        file.write(atlas.name.data(), name_len);

        // --- path ---
        int path_len = (int)atlas.imgpath.size();
        file.write((char*)&path_len, sizeof(int));
        file.write(atlas.imgpath.data(), path_len);

        // --- dimensions ---
        int tilesize = (int)atlas.tile_size;
        int tpr = (int)atlas.tiles_per_row;
        int tpc = (int)atlas.tiles_per_col;

        file.write((char*)&tilesize, sizeof(int));
        file.write((char*)&tpr, sizeof(int));
        file.write((char*)&tpc, sizeof(int));

        // --- tile data ---
        int tiledata_count = (int)atlas.tile_data.size();
        file.write((char*)&tiledata_count, sizeof(int));

        for (const TileData & data : atlas.tile_data) {

            int api = data.anim_parent_index;
            uint8_t iat = data.is_anim_tile ? 1 : 0;
            int afc = data.anim_frame_count;
            float afs = data.anim_frame_speed;
            uint8_t col = (uint8_t)data.collision_data;

            file.write((char*)&api, sizeof(int));
            file.write((char*)&iat, sizeof(uint8_t));
            file.write((char*)&afc, sizeof(int));
            file.write((char*)&afs, sizeof(float));
            file.write((char*)&col, sizeof(uint8_t));
        }
    }

    // ================= LAYERS =================
    for (const TileGrid & layer : scene.tile_layers) {

        std::vector<int> atlas_idxs;
        std::vector<int> tile_idxs;

        for (int r = gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE;
             r <= gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE; r++) {

            for (int c = gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE;
                 c <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE; c++) {

                const Tile & tile = layer.get_tile(c, r);

                atlas_idxs.push_back(tile.atlas_idx);
                tile_idxs.push_back(tile.tile_idx);
            }
        }

        int count = (int)atlas_idxs.size();

        file.write((char*)&count, sizeof(int));
        file.write((char*)atlas_idxs.data(), count * sizeof(int));

        file.write((char*)&count, sizeof(int));
        file.write((char*)tile_idxs.data(), count * sizeof(int));
    }

    return true;
}



bool serial::LoadSceneFromFile(Scene & scene, AssetManager & assets, const std::string& filepath) {

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    // ================= HEADER =================
    SceneFileHeader header;
    file.read((char*)&header, sizeof(header));

    if (strncmp(header.magic, "SCENE", 5) != 0) return false;

    if (header.version != 0) return false;
    if (!file.good()) return false;

    // ================= LAYER INFO =================
    int layer_header[7];
    file.read((char*)layer_header, sizeof(layer_header));

    int layer_count = layer_header[0];

    

    // ================= ATLAS COUNT =================
    int atlas_count;
    file.read((char*)&atlas_count, sizeof(int));

    scene.loaded_atlases.clear();

    // ================= LOAD ATLASES =================
    for (int i = 0; i < atlas_count; i++) {

        // name
        int len;
        file.read((char*)&len, sizeof(int));
        std::string name(len, '\0');
        file.read(name.data(), len);

        // path
        file.read((char*)&len, sizeof(int));
        std::string path(len, '\0');
        file.read(path.data(), len);

        if (path.empty() || !std::filesystem::exists(path)) {
            return false;
        }

        // dimensions
        int tilesize, tpr, tpc;
        file.read((char*)&tilesize, sizeof(int));
        file.read((char*)&tpr, sizeof(int));
        file.read((char*)&tpc, sizeof(int));

        scene.import_new_tileset(assets, name, path, tilesize, tpr, tpc);

        // tile data
        int tiledata_count;
        file.read((char*)&tiledata_count, sizeof(int));

        scene.loaded_atlases[i].tile_data.resize(tiledata_count);

        for (int j = 0; j < tiledata_count; j++) {

            int api;
            uint8_t iat;
            int afc;
            float afs;
            uint8_t col;

            file.read((char*)&api, sizeof(int));
            file.read((char*)&iat, sizeof(uint8_t));
            file.read((char*)&afc, sizeof(int));
            file.read((char*)&afs, sizeof(float));
            file.read((char*)&col, sizeof(uint8_t));

            TileData & td = scene.loaded_atlases[i].tile_data[j];

            td.anim_parent_index = api;
            td.is_anim_tile = (iat != 0);
            td.anim_frame_count = afc;
            td.anim_frame_speed = afs;
            td.collision_data = (CollisionType)col;
        }
    }

    // ================= LOAD LAYERS =================
    scene.tile_layers.clear();

    for (int l = 0; l < layer_count; l++) {

        scene.tile_layers.push_back(TileGrid());
        TileGrid & layer = scene.tile_layers.back();

        int count;

        std::vector<int> atlas_idxs;
        file.read((char*)&count, sizeof(int));
        atlas_idxs.resize(count);
        file.read((char*)atlas_idxs.data(), count * sizeof(int));

        std::vector<int> tile_idxs;
        file.read((char*)&count, sizeof(int));
        tile_idxs.resize(count);
        file.read((char*)tile_idxs.data(), count * sizeof(int));

        int idx = 0;

        for (int r = gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE;
             r <= gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE; r++) {

            for (int c = gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE;
                 c <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE; c++) {

                layer.get_tile(c, r) = {atlas_idxs[idx], tile_idxs[idx]};

                idx++;
            }
        }
    }

    return true;
}




