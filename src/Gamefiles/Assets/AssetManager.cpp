// AssetManager.cpp

#include "AssetManager.h"



using namespace std::filesystem;
using namespace std;


namespace {
    std::string NormalizeExtensionFilter(std::string extension) {
        if (!extension.empty() && extension.front() != '.') {
            extension.insert(extension.begin(), '.');
        }
        return extension;
    }
}


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

std::vector<std::string> AssetManager::GetFilenamesInDirectory (const std::string & _path, const std::string & _extension) {

    vector<string> Filenames;
    const std::string extensionFilter = NormalizeExtensionFilter(_extension);

    for (const auto & entry : directory_iterator(_path)) {

        if (entry.is_regular_file() && entry.path().extension() == extensionFilter) {

            string file = entry.path().filename().string();
            Filenames.push_back(file);

        }

    }

    return Filenames;

}


std::vector<std::string> AssetManager::GetFilepathsInDirectory (const std::string & _path, const std::string & _extension) {

    vector<string> Filepaths;
    const std::string extensionFilter = NormalizeExtensionFilter(_extension);

    for (const auto & entry : directory_iterator(_path)) {

        if (entry.is_regular_file() && entry.path().extension() == extensionFilter) {

            string file = entry.path().string();
            Filepaths.push_back(file);

        }

    }

    return Filepaths;


}















void AssetManager::UnloadAllAssets() {
    for (auto & tileset : loaded_tilesets) {
        UnloadTexture(*tileset);
    }
    loaded_tilesets_paths.clear();
    loaded_tilesets.clear();
}

void AssetManager::UnloadAllTilesetTextureAssets() {
    for (auto & tileset : loaded_tilesets) {
        UnloadTexture(*tileset);
    }
    loaded_tilesets_paths.clear();
    loaded_tilesets.clear();
}

Texture2D & AssetManager::LoadTilesetTexture (const string & _path) {

    if (DoesTilesetWithPathExist(_path)) {
        return GetTilesetTextureAtPath(_path);
    }

    int new_tileset_index = loaded_tilesets.size();

    loaded_tilesets.push_back(std::make_unique<Texture2D>(LoadTexture(_path.c_str())));
    // SetTextureFilter(*loaded_tilesets.back(), TEXTURE_FILTER_POINT);
    SetTextureWrap(*loaded_tilesets.back(), TEXTURE_WRAP_CLAMP);
    loaded_tilesets_paths.push_back(_path);

    return *(loaded_tilesets[new_tileset_index]);

}


bool AssetManager::UnloadTilesetTexture(Texture2D * _texture_ptr) {
    if (_texture_ptr == nullptr) {
        return false;
    }

    for (size_t i = 0; i < loaded_tilesets.size(); i++) {
        if (loaded_tilesets[i].get() == _texture_ptr) {
            UnloadTexture(*loaded_tilesets[i]);
            loaded_tilesets.erase(loaded_tilesets.begin() + i);
            loaded_tilesets_paths.erase(loaded_tilesets_paths.begin() + i);
            return true;
        }
    }

    return false;
}