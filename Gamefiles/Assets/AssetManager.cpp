// AssetManager.cpp

#include "AssetManager.h"

using namespace std::filesystem;
using namespace std;

// Utility
vector<string> AssetManager::GetTilesetFilenames (const string & _path) {

    vector<string> TilesetFilenames;

    for (const auto & entry : directory_iterator(_path)) {

        if (entry.is_regular_file() && entry.path().extension() == ".png") {

            string file = entry.path().filename().string();
            TilesetFilenames.push_back(file);

        }

    }

    return TilesetFilenames;

}

vector<string> AssetManager::GetTilesetPaths (const string & _path) {

    vector<string> TilesetFilenames;

    for (const auto & entry : directory_iterator(_path)) {

        if (entry.is_regular_file() && entry.path().extension() == ".png") {

            string file = entry.path().string();
            TilesetFilenames.push_back(file);

        }

    }

    return TilesetFilenames;

}

void AssetManager::UnloadAllAssets() {
    for (Texture2D & tileset : loaded_tilesets) {
        UnloadTexture(tileset);
    }
    loaded_tilesets_paths.clear();
    loaded_tilesets.clear();
}

Texture2D & AssetManager::LoadTilesetTexture (const string & _path) {

    if (DoesTilesetWithPathExist(_path)) {
        return GetTilesetTextureAtPath(_path);
    }

    int new_tileset_index = loaded_tilesets.size();

    loaded_tilesets.push_back(LoadTexture(_path.c_str()));
    loaded_tilesets_paths.push_back(_path);

    Texture2D & tileset = loaded_tilesets[new_tileset_index];
    return tileset;

}