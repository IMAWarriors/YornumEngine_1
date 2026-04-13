// AssetManager.h

#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <filesystem>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cstddef>

#include "raylib.h"



class AssetManager {

    private:

        struct LoadedTextureAsset {
            std::string path;
            std::unique_ptr<Texture2D> texture;
            std::size_t references = 0;
        };

        std::vector<LoadedTextureAsset> loaded_textures;

        std::string NormalizePath(const std::string & _path) const;

        int FindTextureIndexByPath(const std::string & _path) const;

        int FindTextureIndexByPointer(const Texture2D * _texture_ptr) const;
    


    public:

        Texture2D & LoadTextureAsset (const std::string & _path);

        bool UnloadTextureAsset (Texture2D * _texture_ptr);

        bool UnloadTextureAssetByPath (const std::string & _path);

        bool IsTextureAssetLoaded(const std::string & _path) const;

        Texture2D * GetTextureAssetIfLoaded(const std::string & _path);

        std::size_t GetTextureAssetReferenceCount(const std::string & _path) const;

        void UnloadAllTextureAssets();

        Texture2D & LoadTilesetTexture (const std::string & _path);

        bool UnloadTilesetTexture(Texture2D * _texture_ptr);

        void UnloadAllAssets();

        void UnloadAllTilesetTextureAssets();

        // Getter Functions
        


        // Utility Functions

        std::vector<std::string> GetTilesetFilenames (const std::string & _path);
        std::vector<std::string> GetTilesetPaths (const std::string & _path);

        std::vector<std::string> GetFilenamesInDirectory (const std::string & _path, const std::string & _extension);
        std::vector<std::string> GetFilepathsInDirectory (const std::string & _path, const std::string & _extension);


};

#endif
