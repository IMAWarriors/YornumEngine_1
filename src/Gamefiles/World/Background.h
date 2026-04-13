// Background.h

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <string>
#include <vector>

#include "../../Engine/Core/Overhead/GameTypes.h"

struct ParallaxNode {

    Texture2D* image;
    int layer;
    bool x_repeating;
    bool y_repeating;
    int seat_x;
    int seat_y;
};

struct ParallaxLayer {
    float x_dist_offset;
    float y_dist_offset;
    float z_dist_offset;

    Color tint;

    std::vector<ParallaxNode> nodes;
};

class Background {

public:

    const int ALLOWED_NODE_SPACE_X_MIN = -16;
    const int ALLOWED_NODE_SPACE_X_MAX = 16;
    const int ALLOWED_NODE_SPACE_Y_MIN = -16;
    const int ALLOWED_NODE_SPACE_Y_MAX = 16;

    Background() {




    }



    bool new_layer (float ox = 0.0f, float oy = 0.0f, float oz = 1.0f, Color t = WHITE) {

        bool FAIL_SAMEZDIST = false;

        for (ParallaxLayer layer : layers) {
            if (layer.z_dist_offset == oz) {
                FAIL_SAMEZDIST = true;
                break;
            }
        }

        if (FAIL_SAMEZDIST) {
            return false;
        }

        layers.push_back({ox, oy, oz,t});
        layers[layers.size()-1].nodes.clear();
    }

    bool new_parallax (int layer, Texture2D* image, int x = 0, int y = 0, bool xrep = false, bool yrep = false) {

        if (ALLOWED_NODE_SPACE_X_MIN < x || x > ALLOWED_NODE_SPACE_X_MAX) { return false; }
        if (ALLOWED_NODE_SPACE_Y_MIN < y || y > ALLOWED_NODE_SPACE_Y_MAX) { return false; }
        if (layers.size() >= layer || layer < 0) { return false; }

        layers[layer].nodes.push_back({image, layer, xrep, yrep, x, y});
        
    }
    



private:

    Texture2D* backdrop_image;

    std::vector<ParallaxLayer> layers;

};


#endif