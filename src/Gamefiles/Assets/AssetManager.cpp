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

std::string AssetManager::NormalizePath(const std::string & _path) const {
    const std::filesystem::path rawPath(_path);
    std::error_code ec;
    const std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(rawPath, ec);

    if (!ec) {
        return canonicalPath.lexically_normal().string();
    }

    return rawPath.lexically_normal().string();
}

int AssetManager::FindTextureIndexByPath(const std::string & _path) const {
    for (size_t i = 0; i < loaded_textures.size(); i++) {
        if (loaded_textures[i].path == _path) {
            return (int)i;
        }
    }
    return -1;
}

int AssetManager::FindTextureIndexByPointer(const Texture2D * _texture_ptr) const {
    if (_texture_ptr == nullptr) {
        return -1;
    }

    for (size_t i = 0; i < loaded_textures.size(); i++) {
        if (loaded_textures[i].texture.get() == _texture_ptr) {
            return (int)i;
        }
    }

    return -1;
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
    UnloadAllTextureAssets();
}

void AssetManager::UnloadAllTilesetTextureAssets() {
    UnloadAllTextureAssets();
}

Texture2D & AssetManager::LoadTextureAsset (const string & _path) {

    const string normalizedPath = NormalizePath(_path);
    const int existingIndex = FindTextureIndexByPath(normalizedPath);

    if (existingIndex >= 0) {
        loaded_textures[(size_t)existingIndex].references++;
        return *(loaded_textures[(size_t)existingIndex].texture);
    }

    LoadedTextureAsset asset;
    asset.path = normalizedPath;
    asset.texture = std::make_unique<Texture2D>(LoadTexture(normalizedPath.c_str()));
    asset.references = 1;

    if (asset.texture->id == 0) {
        throw std::runtime_error("Failed to load texture asset: " + normalizedPath);
    }
    SetTextureWrap(*asset.texture, TEXTURE_WRAP_CLAMP);
    loaded_textures.push_back(std::move(asset));

    return *(loaded_textures.back().texture);
}

bool AssetManager::UnloadTextureAsset(Texture2D * _texture_ptr) {

    const int index = FindTextureIndexByPointer(_texture_ptr);
    if (index < 0) {
        return false;
    }

    LoadedTextureAsset & asset = loaded_textures[(size_t)index];

    if (asset.references > 1) {
        asset.references--;
        return true;
    }

    UnloadTexture(*asset.texture);
    loaded_textures.erase(loaded_textures.begin() + index);
    return true;

}

bool AssetManager::UnloadTextureAssetByPath(const string & _path) {
    const string normalizedPath = NormalizePath(_path);
    const int index = FindTextureIndexByPath(normalizedPath);

    if (index < 0) {
        return false;
    }

    return UnloadTextureAsset(loaded_textures[(size_t)index].texture.get());
}

bool AssetManager::IsTextureAssetLoaded(const string & _path) const {
    const string normalizedPath = NormalizePath(_path);
    return FindTextureIndexByPath(normalizedPath) >= 0;
}

Texture2D * AssetManager::GetTextureAssetIfLoaded(const string & _path) {
    const string normalizedPath = NormalizePath(_path);
    const int index = FindTextureIndexByPath(normalizedPath);

    if (index < 0) {
        return nullptr;
    }

    return loaded_textures[(size_t)index].texture.get();
}

size_t AssetManager::GetTextureAssetReferenceCount(const string & _path) const {
    const string normalizedPath = NormalizePath(_path);
    const int index = FindTextureIndexByPath(normalizedPath);

    if (index < 0) {
        return 0;
    }

    return loaded_textures[(size_t)index].references;
}

void AssetManager::UnloadAllTextureAssets() {
    for (auto & loadedTexture : loaded_textures) {
        UnloadTexture(*loadedTexture.texture);
    }
    loaded_textures.clear();
}

Texture2D & AssetManager::LoadTilesetTexture (const string & _path) {
    return LoadTextureAsset(_path);

}


bool AssetManager::UnloadTilesetTexture(Texture2D * _texture_ptr) {
    return UnloadTextureAsset(_texture_ptr);
}