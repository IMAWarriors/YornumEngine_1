// Renderer.cpp

#include "Renderer.h"

#include "../../../Gamefiles/Game/GameEngine.h"


void Renderer::begin_texture_frame (RenderTexture2D & canvas) {
    // ======================= DRAW ===========================
    /* START TAG */ BeginTextureMode(canvas);
                    ClearBackground(BLANK);
    // ========================================================
}

void Renderer::end_texture_frame () {
    // ======================= END DRAW =======================
    /* END TAG */ EndTextureMode();
    // ========================================================
}

// Defined in Func_ files: void Renderer::present (RenderTexture2D & canvas, GameEngine & game, float alpha) {...}

void Renderer::set_camera_position (Vec2 position) {

    camera_position = position;

}

Vec2 Renderer::get_camera_position () const {

    return camera_position;

}

void Renderer::set_camera_zoom (float zoom) {
    camera_zoom = zoom;
}

float Renderer::get_camera_zoom () const {
    return camera_zoom;
}


Vec2 Renderer::world_camera_transform (Vec2 world_coords) {

    const int SCREEN_HALF_WIDTH = config::GAME_WORLD_WIDTH/2;
    const int SCREEN_HALF_HEIGHT = config::GAME_WORLD_HEIGHT/2;

    Vec2 new_coords = {     (world_coords.x - camera_position.x) * camera_zoom,
                            (world_coords.y - camera_position.y) * camera_zoom};
    new_coords = {new_coords.x + SCREEN_HALF_WIDTH, new_coords.y + SCREEN_HALF_HEIGHT};             // Center camera coords

    return new_coords;

}

void Renderer::init_canvas (RenderTexture2D & render_texture) {

    // Game texture itself setup
    render_texture = LoadRenderTexture(config::GAME_WORLD_WIDTH, config::GAME_WORLD_HEIGHT);
    SetTextureFilter(render_texture.texture, TEXTURE_FILTER_BILINEAR);

    debug_font = LoadFontEx("Gamefiles/Assets/Fonts/consolai.ttf",24,0,0);
    
}




// ============================================================================ //
//                          Render Drawing Wrappers                             //
// ============================================================================ //

//       Wrapping Functions
// ================================================================
std::string Renderer::text(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int size = std::vsnprintf(nullptr, 0, fmt, args) + 1;
    va_end(args);

    std::vector<char> buffer(size);

    va_start(args, fmt);
    std::vsnprintf(buffer.data(), size, fmt, args);
    va_end(args);

    return std::string(buffer.data());
}



//       Static Screen Drawings
// ================================================================

void Renderer::rdraw_text (const std::string & _text, int _x, int _y, int _size, Color _color) {
    // This kind of text will be drawn to screenspace for now
    DrawTextEx(debug_font,_text.c_str(), {(float)_x, (float)_y}, _size, 1, _color);
}



//       Game World Drawings
// ================================================================

void Renderer::rdraw_circle(float _x, float _y, float _radius, Color _color) {

    Vec2 new_coords = world_camera_transform({_x, _y});
    DrawCircle((int)(new_coords.x), (int)(new_coords.y), _radius * camera_zoom, _color);

}



void Renderer::rdraw_sprite (Texture2D _texture, Rectangle _crop, Rectangle _paste) {

    DrawTexturePro(_texture, _crop, _paste, {0.0f, 0.0f}, 0.0f, WHITE);

}

void Renderer::rdraw_sprite_col (Texture2D _texture, Rectangle _crop, Rectangle _paste, Color _color) {
    DrawTexturePro(_texture, _crop, _paste, {0.0f, 0.0f}, 0.0f, _color);
}
