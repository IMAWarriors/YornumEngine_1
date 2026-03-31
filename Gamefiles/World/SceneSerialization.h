// SceneSerialization.h

#ifndef SCENESERIALIZATION_H
#define SCENESERIALIZATION_H

#include <cstring>
#include <string>
#include <fstream>

#include "Scene.h"
#include "TileAtlas.h"
#include "TileGrid.h"
#include "Overhead/Gwconst.h"


struct SceneFileHeader {

    char    magic[6];
    int     version;

};


namespace serial {



bool SaveSceneToFile(const Scene & scene, const std::string& filepath);

bool LoadSceneFromFile(Scene & scene, AssetManager & assets, const std::string& filepath);


}


#endif

