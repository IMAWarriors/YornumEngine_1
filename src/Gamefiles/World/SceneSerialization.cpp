// SceneSerialization.cpp


#include "SceneSerialization.h"

#include <filesystem>

namespace {
    constexpr int SCENE_FILE_VERSION_WITH_CAMERA_CLAMPS = 1;

    std::string ResolveTilesetPathFromSceneRecord(const std::string& rawPath) {
        namespace fs = std::filesystem;

        if (rawPath.empty()) {
            return {};
        }

        fs::path candidate(rawPath);
        if (fs::exists(candidate)) {
            return candidate.string();
        }

        // Fallback for legacy scene files after project structure refactors:
        // keep only the filename and search in the current tileset directory.
        fs::path fallback = fs::path("assets/sprites/tilesets") / candidate.filename();
        if (fs::exists(fallback)) {
            return fallback.string();
        }

        return {};
    }

    void WriteVec2(std::ofstream& file, const Vec2& value) {
        file.write((char*)&value.x, sizeof(float));
        file.write((char*)&value.y, sizeof(float));
    }

    bool ReadVec2(std::ifstream& file, Vec2& value) {
        file.read((char*)&value.x, sizeof(float));
        file.read((char*)&value.y, sizeof(float));
        return file.good();
    }
}

bool serial::SaveSceneToFile(const Scene & scene, const std::string& filepath) {

    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    // ================= HEADER =================
    SceneFileHeader header = {{'S','C','E','N','E','\0'}, SCENE_FILE_VERSION_WITH_CAMERA_CLAMPS};
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

    // ================= CAMERA CLAMPS =================
    int clamp_count = (int)scene.active_clamps.size();
    file.write((char*)&clamp_count, sizeof(int));

    for (const CameraClamp& clamp : scene.active_clamps) {
        WriteVec2(file, clamp.clamp_top_left);
        WriteVec2(file, clamp.clamp_bottom_right);
        WriteVec2(file, clamp.player_zone_top_left);
        WriteVec2(file, clamp.player_zone_bottom_right);

        file.write((char*)&clamp.smoothing_override, sizeof(float));

        uint8_t snap_to_clamp = clamp.snap_to_clamp ? 1 : 0;
        file.write((char*)&snap_to_clamp, sizeof(uint8_t));
    }

    if (!file.good()) {
        return false;
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

    if (header.version < 0 || header.version > SCENE_FILE_VERSION_WITH_CAMERA_CLAMPS) return false;
    if (!file.good()) return false;

    // ================= LAYER INFO =================
    int layer_header[7];
    file.read((char*)layer_header, sizeof(layer_header));

    int layer_count = layer_header[0];
    if (!file.good() || layer_count < 0) {
        return false;
    }

    

    // ================= ATLAS COUNT =================
    int atlas_count;
    file.read((char*)&atlas_count, sizeof(int));

    if (!file.good() || atlas_count < 0) {
        return false;
    }

    scene.loaded_atlases.clear();

    // ================= LOAD ATLASES =================
    for (int i = 0; i < atlas_count; i++) {

        // name
        int len;
        file.read((char*)&len, sizeof(int));
        if (!file.good() || len < 0) {
            return false;
        }


        std::string name(len, '\0');
        file.read(name.data(), len);

        // path
        file.read((char*)&len, sizeof(int));
        if (!file.good() || len < 0) {
            return false;
        }


        std::string path(len, '\0');
        file.read(path.data(), len);

        path = ResolveTilesetPathFromSceneRecord(path);
        if (path.empty()) {
            return false;
        }

        // dimensions
        int tilesize, tpr, tpc;
        file.read((char*)&tilesize, sizeof(int));
        file.read((char*)&tpr, sizeof(int));
        file.read((char*)&tpc, sizeof(int));
        if (!file.good() || tilesize <= 0 || tpr <= 0 || tpc <= 0) {
            return false;
        }

        scene.import_new_tileset(assets, name, path, tilesize, tpr, tpc);

        // tile data
        int tiledata_count;
        file.read((char*)&tiledata_count, sizeof(int));
        if (!file.good() || tiledata_count < 0) {
            return false;
        }

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
            if (!file.good()) {
                return false;
            }

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
        if (!file.good() || count < 0) {
            return false;
        }

        atlas_idxs.resize(count);
        file.read((char*)atlas_idxs.data(), count * sizeof(int));
        if (!file.good()) {
            return false;
        }

        std::vector<int> tile_idxs;
        file.read((char*)&count, sizeof(int));
        if (!file.good() || count < 0) {
            return false;
        }

        tile_idxs.resize(count);
        file.read((char*)tile_idxs.data(), count * sizeof(int));
        if (!file.good()) {
            return false;
        }

        const int expected_tiles = gwconst::WORLD_TILEGRID_WIDTH * gwconst::WORLD_TILEGRID_HEIGHT;
        if ((int)atlas_idxs.size() != expected_tiles || (int)tile_idxs.size() != expected_tiles) {
            return false;
        }


        int idx = 0;

        for (int r = gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE;
             r <= gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE; r++) {

            for (int c = gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE;
                 c <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE; c++) {

                const int atlas_idx = atlas_idxs[idx];
                const int tile_idx = tile_idxs[idx];

                if (atlas_idx < 0 || tile_idx < 0) {
                    layer.get_tile(c, r) = {-1, -1};
                } else if (atlas_idx >= (int)scene.loaded_atlases.size() || tile_idx >= (int)scene.loaded_atlases[atlas_idx].tile_data.size()) {
                    layer.get_tile(c, r) = {-1, -1};
                } else {
                    layer.get_tile(c, r) = {atlas_idx, tile_idx};
                }

                idx++;
            }
        }
    }

    // ================= LOAD CAMERA CLAMPS =================
    scene.active_clamps.clear();

    if (header.version >= SCENE_FILE_VERSION_WITH_CAMERA_CLAMPS) {
        int clamp_count = 0;
        file.read((char*)&clamp_count, sizeof(int));
        if (!file.good() || clamp_count < 0) {
            return false;
        }

        scene.active_clamps.resize(clamp_count);
        for (CameraClamp& clamp : scene.active_clamps) {
            if (!ReadVec2(file, clamp.clamp_top_left)) return false;
            if (!ReadVec2(file, clamp.clamp_bottom_right)) return false;
            if (!ReadVec2(file, clamp.player_zone_top_left)) return false;
            if (!ReadVec2(file, clamp.player_zone_bottom_right)) return false;

            file.read((char*)&clamp.smoothing_override, sizeof(float));
            uint8_t snap_to_clamp = 0;
            file.read((char*)&snap_to_clamp, sizeof(uint8_t));

            if (!file.good()) {
                return false;
            }

            clamp.snap_to_clamp = (snap_to_clamp != 0);
        }
    }

    return true;
}



