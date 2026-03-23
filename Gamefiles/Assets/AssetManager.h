// AssetManager.h

#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <filesystem>
#include <string>
#include <vector>

#include "raylib.h"

class AssetManager {

    private:

        // LOADED ASSETS
        std::vector<std::string> loaded_tilesets_paths;
        std::vector<Texture2D> loaded_tilesets;

        bool DoesTilesetWithPathExist (const std::string & _path) {
            for (const std::string & pathname : loaded_tilesets_paths) {
                if (pathname == _path) {
                    return true;
                }
            }
            return false;
        }

        Texture2D & GetTilesetTextureAtPath (const std::string & _path) {
            int index = 0;
            for (const std::string & pathname : loaded_tilesets_paths) {
                if (pathname == _path) {
                    return loaded_tilesets[index];
                }
                index++;
            }
        }
    


    public:

        Texture2D & LoadTilesetTexture (const std::string & _path);

        void UnloadAllAssets();

        // Getter Functions
        


        // Utility Functions

        std::vector<std::string> GetTilesetFilenames (const std::string & _path);
        std::vector<std::string> GetTilesetPaths (const std::string & _path);


};

#endif

