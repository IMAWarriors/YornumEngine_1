// Func_RendererPresentDef.cpp

#include "Renderer.h"

#include "../../../Gamefiles/Game/GameEngine.h"
#include "../Window/Window.h"


void Renderer::present (RenderTexture2D & canvas, GameEngine & game, float alpha, Window & window) {

    // ========== DRAW PHASE :    Texture to screen =================
    /* START TAG */ BeginDrawing();
    // ==============================================================

    ClearBackground(BLACK);

    float scaleX = (float)GetScreenWidth()  / config::GAME_WORLD_WIDTH;
    float scaleY = (float)GetScreenHeight() / config::GAME_WORLD_HEIGHT;

    float scale = scaleY;

    if (fabs(scaleX - scaleY) < 0.01f) {
        scale = scaleX; // force full coverage when nearly identical
    }

    // Setup values
    float width  = config::GAME_WORLD_WIDTH  * scale;
    float height = config::GAME_WORLD_HEIGHT * scale;
    float offsetX = (GetScreenWidth()  - width)  / 2.0f;
    float offsetY = (GetScreenHeight() - height) / 2.0f;

    if (window.painter_ready) {
        Vec2 camera_pos = get_camera_position();
        const float camera_pos_data[2] = {camera_pos.x, camera_pos.y};
        const float resolution_data[2] = {
            (float)config::GAME_WORLD_WIDTH,
            (float)config::GAME_WORLD_HEIGHT
        };
        const float viewport_data[4] = {offsetX, offsetY, width, height};
        float camera_zoom_value = get_camera_zoom();
        float time_value = GetTime();

        if (window.painter_camera_pos_loc >= 0) {
            SetShaderValue(window.painter, window.painter_camera_pos_loc, camera_pos_data, SHADER_UNIFORM_VEC2);
        }
        if (window.painter_resolution_loc >= 0) {
            SetShaderValue(window.painter, window.painter_resolution_loc, resolution_data, SHADER_UNIFORM_VEC2);
        }
        if (window.painter_viewport_loc >= 0) {
            SetShaderValue(window.painter, window.painter_viewport_loc, viewport_data, SHADER_UNIFORM_VEC4);
        }
        if (window.painter_zoom_loc >= 0) {
            SetShaderValue(window.painter, window.painter_zoom_loc, &camera_zoom_value, SHADER_UNIFORM_FLOAT);
        }
        if (window.painter_time_loc >= 0) {
            SetShaderValue(window.painter, window.painter_time_loc, &time_value, SHADER_UNIFORM_FLOAT);
        }

        BeginShaderMode(window.painter);
        DrawTexturePro(
            canvas.texture, { 0, 0, (float)config::GAME_WORLD_WIDTH, -(float)config::GAME_WORLD_HEIGHT },
            { offsetX, offsetY, width, height }, { 0, 0 }, 0.0f, WHITE
        );
        EndShaderMode();
    } else {
        DrawTexturePro(
            canvas.texture, { 0, 0, (float)config::GAME_WORLD_WIDTH, -(float)config::GAME_WORLD_HEIGHT },
            { offsetX, offsetY, width, height }, { 0, 0 }, 0.0f, WHITE
        );
    }

    // ==============================================================
    /* END   TAG */ EndDrawing();
    // =============================================================================================

}