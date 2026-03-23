// AssetManager.cpp

#include "AssetManager.h"

using namespace std::filesystem;
using namespace std;

// Utility
vector<string> AssetManager::GetTilesetFilenames (const string & _path) {

    vector<string> TilesetFilenames;

    for (const auto & entry : directory_iterator(_path)) {

        if (entry.is_regular_file() && entry.path().extension() == ".png") {

            TilesetFilenames.push_back(entry.path().string());

        }

    }

    return TilesetFilenames;

}

void AssetManager::UnloadAllAssets() {

    loaded_tilesets.clear();

}

Texture2D & AssetManager::LoadTilesetTexture (const string & _path) {

    int new_tileset_index = loaded_tilesets.size();

    loaded_tilesets.push_back(LoadTexture(_path.c_str()));
    loaded_tilesets_paths.push_back(_path);

    Texture2D & tileset = loaded_tilesets[new_tileset_index];
    return tileset;

}