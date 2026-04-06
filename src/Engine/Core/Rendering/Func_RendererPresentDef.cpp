// Func_RendererPresentDef.cpp

#include "Renderer.h"

#include "../../../Gamefiles/Game/GameEngine.h"
#include "../Window/Window.h"


void Renderer::present (RenderTexture2D & canvas, GameEngine & game, float alpha, Window & window) {

    // ========== DRAW PHASE :    Texture to screen =================
    /* START TAG */ BeginDrawing();
    // ==============================================================

    BeginShaderMode(window.painter);

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

    // Draw onto texture here:
    DrawTexturePro(
        canvas.texture, { 0, 0, (float)config::GAME_WORLD_WIDTH, -(float)config::GAME_WORLD_HEIGHT },
        { offsetX, offsetY, width, height }, { 0, 0 }, 0.0f, WHITE
    );

    EndShaderMode();

    // ==============================================================
    /* END   TAG */ EndDrawing();
    // =============================================================================================

}