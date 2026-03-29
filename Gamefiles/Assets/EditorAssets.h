// EditorAssets.h

#ifndef EDITORASSETS_H
#define EDITORASSETS_H

#include "raylib.h"

enum class CollisionIcons {
    UTIL_EMPTY_COL_ICO = 0,
    UTIL_FULL_COL_ICO,
    UTIL_PSLOPE1_COL_ICO,
    UTIL_NSLOPE1_COL_ICO
};

struct EditorAssets {

    Texture2D util_tileset;

    void init() {
        util_tileset = LoadTexture("Gamefiles/Assets/Sprites/Tilesets/util_tileset_1.png");
    }

    Rectangle util_tileset_geticon (CollisionIcons ico) {
        if (ico == CollisionIcons::UTIL_EMPTY_COL_ICO) {
            return {0,0,32,32};
        }

        if (ico == CollisionIcons::UTIL_FULL_COL_ICO) {
            return {32,0,32,32};
        }

        if (ico == CollisionIcons::UTIL_PSLOPE1_COL_ICO) {
            return {0,32,32,32};
        }

        if (ico == CollisionIcons::UTIL_NSLOPE1_COL_ICO) {
            return {32,32,32,32};
        }

        return {0,0,0,0};
    }



    void shutdown() {
        UnloadTexture(util_tileset);
    }


};

#endif