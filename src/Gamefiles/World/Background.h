// Background.h

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <string>
#include <vector>
#include <optional>

#include "../../Engine/Core/Overhead/GameTypes.h"

class AssetManager;

struct ParallaxNode {

    Texture2D* image = nullptr;
    std::string image_path;

    bool x_repeating = false;
    bool y_repeating = false;

    int seat_x = 0;
    int seat_y = 0;
};


struct ParallaxLayer {
    float x_dist_offset = 0.0f;
    float y_dist_offset = 0.0f;
    float z_dist_offset = 1.0f;

    Color tint = WHITE;

    std::vector<ParallaxNode> nodes;
};

class Background {

public:

    static constexpr int ALLOWED_NODE_SPACE_X_MIN = -64;
    static constexpr int ALLOWED_NODE_SPACE_X_MAX = 64;
    static constexpr int ALLOWED_NODE_SPACE_Y_MIN = -64;
    static constexpr int ALLOWED_NODE_SPACE_Y_MAX = 64;

    Texture2D* backdrop_image = nullptr;
    std::string backdrop_image_path;
    std::vector<ParallaxLayer> layers;

    void clear();

    bool set_backdrop_image(AssetManager& assets, const std::string& image_path);
    void clear_backdrop_image(AssetManager& assets);

    bool new_layer(float ox = 0.0f, float oy = 0.0f, float oz = 1.0f, Color t = WHITE);
    bool remove_layer(int layer_idx, AssetManager& assets);
    bool move_layer(int layer_idx, int destination_idx);

    bool new_parallax(int layer_idx, AssetManager& assets, const std::string& image_path, int x = 0, int y = 0, bool xrep = false, bool yrep = false);
    bool remove_parallax(int layer_idx, int node_idx, AssetManager& assets);
    
    bool set_node_repeat_x(int layer_idx, int node_idx, bool enabled);
    bool set_node_repeat_y(int layer_idx, int node_idx, bool enabled);

    std::optional<ParallaxNode> resolve_node_for_tile(const ParallaxLayer& layer, int seat_x, int seat_y) const;

    bool resolve_assets(AssetManager& assets);
    void release_assets(AssetManager& assets);
    bool validate_image_for_screen_tile(const std::string& image_path) const;
    


private:

    void enforce_repeat_rules_for_node(ParallaxLayer& layer, int node_idx);
    bool is_valid_layer_index(int layer_idx) const;
    bool is_valid_node_seat(int x, int y) const;

};


#endif