// Func_ERendererPresentDef.cpp

#include "Renderer.h"

#include "../../../Gamefiles/Game/GameEngine.h"

#include "../../../../external/rlimgui/rlImGui.h"

void Renderer::present (RenderTexture2D & canvas, GameEngine & game, float alpha) {

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

    // Draw onto texture here:
    DrawTexturePro(
        canvas.texture, { 0, 0, (float)config::GAME_WORLD_WIDTH, -(float)config::GAME_WORLD_HEIGHT },
        { offsetX, offsetY, width, height }, { 0, 0 }, 0.0f, WHITE
    );


    // UI STUFF WOULD GO HERE I THINK?



    rlImGuiBegin();

    game.TickPhase(Phases::EDITORUI, alpha);

    rlImGuiEnd();


    // ==============================================================
    /* END   TAG */ EndDrawing();
    // =============================================================================================

}



