// Renderer.cpp

#include "Renderer.h"


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

void Renderer::present (RenderTexture2D & canvas) {








    //





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


    // Draw Debuglayer here

    // ClearBackground(Color({255, 255, 255, 255}));       // Flush gameworld window
    

    

    // DrawText(TextFormat("Screen: %d x %d",      GetScreenWidth(), GetScreenHeight()),        20 , 20, 20 * scaleX, RED);
    // DrawText(TextFormat("Monitor: %d x %d",     GetMonitorWidth(0), GetMonitorHeight(0)),   20 , 50, 20 * scaleX, RED);

    

    

    // ==============================================================
    /* END   TAG */ EndDrawing();
    // =============================================================================================
}


void Renderer::rdraw_circle(float _x, float _y, float _radius, Color _color) {

    DrawCircle((int)_x, (int)_y, _radius, _color);


}

void Renderer::init_canvas (RenderTexture2D & render_texture) {

    // Game texture itself setup
    render_texture = LoadRenderTexture(config::GAME_WORLD_WIDTH, config::GAME_WORLD_HEIGHT);
    SetTextureFilter(render_texture.texture, TEXTURE_FILTER_BILINEAR);
    
}