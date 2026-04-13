#include "Background.h"

#include "../Assets/AssetManager.h"
#include "Overhead/Gwconst.h"

#include <algorithm>

void Background::clear() {
    backdrop_image = nullptr;
    backdrop_image_path.clear();
    layers.clear();
}

bool Background::is_valid_layer_index(int layer_idx) const {
    return layer_idx >= 0 && layer_idx < (int)layers.size();
}

bool Background::is_valid_node_seat(int x, int y) const {
    return x >= ALLOWED_NODE_SPACE_X_MIN && x <= ALLOWED_NODE_SPACE_X_MAX
        && y >= ALLOWED_NODE_SPACE_Y_MIN && y <= ALLOWED_NODE_SPACE_Y_MAX;
}

bool Background::validate_image_for_screen_tile(const std::string& image_path) const {
    Image image = LoadImage(image_path.c_str());
    if (!IsImageValid(image)) {
        return false;
    }

    const bool valid = image.width == gwconst::SCREEN_WIDTH_GAMEPIXELS
        && image.height == gwconst::SCREEN_HEIGHT_GAMEPIXELS;

    UnloadImage(image);
    return valid;
}

bool Background::set_backdrop_image(AssetManager& assets, const std::string& image_path) {
    if (!validate_image_for_screen_tile(image_path)) {
        return false;
    }

    Texture2D& texture = assets.LoadTextureAsset(image_path);

    if (backdrop_image != nullptr) {
        assets.UnloadTextureAsset(backdrop_image);
    }

    backdrop_image = &texture;
    backdrop_image_path = image_path;
    return true;
}

void Background::clear_backdrop_image(AssetManager& assets) {
    if (backdrop_image != nullptr) {
        assets.UnloadTextureAsset(backdrop_image);
        backdrop_image = nullptr;
    }
    backdrop_image_path.clear();
}

bool Background::new_layer(float ox, float oy, float oz, Color t) {
    layers.push_back({ox, oy, oz, t, {}});

    std::stable_sort(layers.begin(), layers.end(), [] (const ParallaxLayer& a, const ParallaxLayer& b) {
        return a.z_dist_offset < b.z_dist_offset;
    });

    return true;
}

bool Background::remove_layer(int layer_idx, AssetManager& assets) {
    if (!is_valid_layer_index(layer_idx)) {
        return false;
    }

    for (ParallaxNode& node : layers[(size_t)layer_idx].nodes) {
        if (node.image != nullptr) {
            assets.UnloadTextureAsset(node.image);
            node.image = nullptr;
        }
    }

    layers.erase(layers.begin() + layer_idx);
    return true;
}

bool Background::move_layer(int layer_idx, int destination_idx) {
    if (!is_valid_layer_index(layer_idx) || !is_valid_layer_index(destination_idx)) {
        return false;
    }

    if (layer_idx == destination_idx) {
        return true;
    }

    ParallaxLayer moved = layers[(size_t)layer_idx];
    layers.erase(layers.begin() + layer_idx);
    layers.insert(layers.begin() + destination_idx, moved);
    return true;
}

void Background::enforce_repeat_rules_for_node(ParallaxLayer& layer, int node_idx) {
    if (node_idx < 0 || node_idx >= (int)layer.nodes.size()) {
        return;
    }

    ParallaxNode& selected = layer.nodes[(size_t)node_idx];

    if (selected.x_repeating) {
        for (int i = 0; i < (int)layer.nodes.size(); i++) {
            if (i == node_idx) {
                continue;
            }
            if (layer.nodes[(size_t)i].seat_y == selected.seat_y) {
                layer.nodes[(size_t)i].x_repeating = false;
            }
        }
    }

    if (selected.y_repeating) {
        for (int i = 0; i < (int)layer.nodes.size(); i++) {
            if (i == node_idx) {
                continue;
            }
            if (layer.nodes[(size_t)i].seat_x == selected.seat_x) {
                layer.nodes[(size_t)i].y_repeating = false;
            }
        }
    }
}

bool Background::new_parallax(int layer_idx, AssetManager& assets, const std::string& image_path, int x, int y, bool xrep, bool yrep) {
    if (!is_valid_layer_index(layer_idx) || !is_valid_node_seat(x, y)) {
        return false;
    }

    if (!validate_image_for_screen_tile(image_path)) {
        return false;
    }

    Texture2D& texture = assets.LoadTextureAsset(image_path);

    ParallaxLayer& layer = layers[(size_t)layer_idx];

    for (ParallaxNode& node : layer.nodes) {
        if (node.seat_x == x && node.seat_y == y) {
            if (node.image != nullptr) {
                assets.UnloadTextureAsset(node.image);
            }
            node.image = &texture;
            node.image_path = image_path;
            node.x_repeating = xrep;
            node.y_repeating = yrep;
            enforce_repeat_rules_for_node(layer, (int)(&node - layer.nodes.data()));
            return true;
        }
    }

    layer.nodes.push_back({&texture, image_path, xrep, yrep, x, y});
    enforce_repeat_rules_for_node(layer, (int)layer.nodes.size() - 1);
    return true;
}

bool Background::remove_parallax(int layer_idx, int node_idx, AssetManager& assets) {
    if (!is_valid_layer_index(layer_idx)) {
        return false;
    }

    ParallaxLayer& layer = layers[(size_t)layer_idx];
    if (node_idx < 0 || node_idx >= (int)layer.nodes.size()) {
        return false;
    }

    ParallaxNode& node = layer.nodes[(size_t)node_idx];
    if (node.image != nullptr) {
        assets.UnloadTextureAsset(node.image);
    }

    layer.nodes.erase(layer.nodes.begin() + node_idx);
    return true;
}

bool Background::set_node_repeat_x(int layer_idx, int node_idx, bool enabled) {
    if (!is_valid_layer_index(layer_idx)) {
        return false;
    }

    ParallaxLayer& layer = layers[(size_t)layer_idx];
    if (node_idx < 0 || node_idx >= (int)layer.nodes.size()) {
        return false;
    }

    layer.nodes[(size_t)node_idx].x_repeating = enabled;
    enforce_repeat_rules_for_node(layer, node_idx);
    return true;
}

bool Background::set_node_repeat_y(int layer_idx, int node_idx, bool enabled) {
    if (!is_valid_layer_index(layer_idx)) {
        return false;
    }

    ParallaxLayer& layer = layers[(size_t)layer_idx];
    if (node_idx < 0 || node_idx >= (int)layer.nodes.size()) {
        return false;
    }

    layer.nodes[(size_t)node_idx].y_repeating = enabled;
    enforce_repeat_rules_for_node(layer, node_idx);
    return true;
}

std::optional<ParallaxNode> Background::resolve_node_for_tile(const ParallaxLayer& layer, int seat_x, int seat_y) const {
    for (const ParallaxNode& node : layer.nodes) {
        if (node.seat_x == seat_x && node.seat_y == seat_y) {
            return node;
        }
    }

    for (const ParallaxNode& node : layer.nodes) {
        if (node.y_repeating && node.seat_x == seat_x && node.seat_y != seat_y) {
            return node;
        }
    }

    for (const ParallaxNode& node : layer.nodes) {
        if (node.x_repeating && node.seat_y == seat_y && node.seat_x != seat_x) {
            return node;
        }
    }

    for (const ParallaxNode& node : layer.nodes) {
        if (node.x_repeating && node.y_repeating) {
            return node;
        }
    }

    return std::nullopt;
}

bool Background::resolve_assets(AssetManager& assets) {
    if (!backdrop_image_path.empty()) {
        if (!validate_image_for_screen_tile(backdrop_image_path)) {
            return false;
        }
        backdrop_image = &assets.LoadTextureAsset(backdrop_image_path);
    }

    for (ParallaxLayer& layer : layers) {
        for (ParallaxNode& node : layer.nodes) {
            if (!validate_image_for_screen_tile(node.image_path)) {
                return false;
            }
            node.image = &assets.LoadTextureAsset(node.image_path);
        }
    }

    return true;
}

void Background::release_assets(AssetManager& assets) {
    if (backdrop_image != nullptr) {
        assets.UnloadTextureAsset(backdrop_image);
        backdrop_image = nullptr;
    }

    for (ParallaxLayer& layer : layers) {
        for (ParallaxNode& node : layer.nodes) {
            if (node.image != nullptr) {
                assets.UnloadTextureAsset(node.image);
                node.image = nullptr;
            }
        }
    }
}