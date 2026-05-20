// Renderer.cpp

#include "Renderer.h"
#include "rlgl.h"

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

    debug_font = LoadFontEx("assets/fonts/consolai.ttf",24,0,0);
    
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

void Renderer::rdraw_rect(float _x, float _y, float _w, float _h, Color _color) {

    Vec2 new_coords = world_camera_transform({_x, _y});
    Vec2 new_max_coords = world_camera_transform({_x + _w, _y + _h});
    Vec2 new_size = {new_max_coords.x - new_coords.x, new_max_coords.y - new_coords.y};
    DrawRectangle((int)(new_coords.x), (int)(new_coords.y), (int)(new_size.x), (int)(new_size.y), _color);



}


void Renderer::rdraw_wfrect(float _x, float _y, float _w, float _h, Color _color, float _stroke) {
    
    Vec2 new_coords = world_camera_transform({_x, _y});
    Vec2 new_max_coords = world_camera_transform({_x + _w, _y + _h});
    Vec2 new_size = {new_max_coords.x - new_coords.x, new_max_coords.y - new_coords.y};

    Rectangle rect = {new_coords.x, new_coords.y, new_size.x, new_size.y};
    DrawRectangleLinesEx(rect, _stroke, _color);

}



void Renderer::rdraw_sprite (Texture2D _texture, Rectangle _crop, Rectangle _paste) {

    DrawTexturePro(_texture, _crop, _paste, {0.0f, 0.0f}, 0.0f, WHITE);

}

void Renderer::rdraw_sprite_col (Texture2D _texture, Rectangle _crop, Rectangle _paste, Color _color) {
    DrawTexturePro(_texture, _crop, _paste, {0.0f, 0.0f}, 0.0f, _color);
}


// Draw a textured quad with 4 explicitly supplied SCREEN-SPACE corners.
// This is the Raylib equivalent of ImGui's draw_list->AddImageQuad().
//
// corners[0] = top-left
// corners[1] = top-right
// corners[2] = bottom-right
// corners[3] = bottom-left
//
// uv_min / uv_max are the normalised UV coordinates of the crop region
// (same values as crop_min / crop_max stored on AvatarJoint).
//
// The function bypasses Raylib's higher-level sprite helpers so that we can
// supply arbitrary corner positions - necessary for rotation + offset.
 

void Renderer::rdraw_quad_screen(
    Texture2D     _texture,
    const Vector2 corners[4],
    Vector2       uv_min,
    Vector2       uv_max,
    Color         _color)
{
    float u0 = uv_min.x, v0 = uv_min.y;
    float u1 = uv_max.x, v1 = uv_max.y;
 
    rlSetTexture(_texture.id);
 
    rlBegin(RL_QUADS);
 
        rlColor4ub(_color.r, _color.g, _color.b, _color.a);
 
        // Raylib/OpenGL expects counter-clockwise winding for front faces
        // when drawn as quads via rlgl.  The corner order TL,BL,BR,TR gives
        // correct CCW winding matching how AddImageQuad renders.
 
        // Top-Left
        rlTexCoord2f(u0, v0);
        rlVertex2f(corners[0].x, corners[0].y);
 
        // Bottom-Left
        rlTexCoord2f(u0, v1);
        rlVertex2f(corners[3].x, corners[3].y);
 
        // Bottom-Right
        rlTexCoord2f(u1, v1);
        rlVertex2f(corners[2].x, corners[2].y);
 
        // Top-Right
        rlTexCoord2f(u1, v0);
        rlVertex2f(corners[1].x, corners[1].y);
 
    rlEnd();
 
    rlSetTexture(0);
}


