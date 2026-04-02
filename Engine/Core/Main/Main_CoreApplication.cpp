// CoreApplication.cpp

#include "CoreApplication.h"


// =========================================================================================
// NORMAL NON-EDITOR BUILD
// =========================================================================================

void CoreApplication::RunCoreEngine(GameEngine & game, const std::string & scenename) {


    // Initialize window and program
    Window window("古残阳: Old Sun || Alpha v1.0.0 ");

    // Initialize Raylib Drawing Aspects
    Renderer            renderer;         
    RenderTexture2D     canvas;           // Intialize CANVAS to draw game onto
    InputManager        inputManager;
    DebugStats          debugInfo;

    renderer.init_canvas(canvas);

    game.Initialize(renderer, inputManager, debugInfo);

    if (scenename != "NOFILE.NONE") {

        game.LoadScene(scenename);
        
    }


    float accumulator = 0.0f;               // Store Time bucket
    float frame_deltatime = 0.0f;           // Store time from last frame
    float frame_cps = 0.0f;                 // Store FPS
    float frame_simulation_ticks = 0.0f;    // Store number of ticks executed in last computed frame

    float alpha = 0.0f;                     // Store the deltatime as an alpha

    while ( window.IsRunning() ) {

        // STEP 1:          LOGIC  | Update logic and game frame state
        // ==========================================
        
        frame_cps = (float)(GetFPS());
        frame_deltatime = GetFrameTime();
        frame_deltatime = std::min(frame_deltatime, config::MAX_FRAME_LAG);

        accumulator += frame_deltatime; // Store how much time passed last frame

        window.ListenFullscreenToggle();
        
        //  Input Logic, VARIABLE TIME
        game.TickPhase(Phases::INPUT, frame_deltatime);

        //  Simulaiton Logic, FIXED TIME
        int simulation_ticks = 0;
        while (accumulator >= config::FIXED_DELTATIME) {
            // Normal Simulation Tick
            game.TickPhase(Phases::SIMULATION, config::FIXED_DELTATIME);
            accumulator -= config::FIXED_DELTATIME;
            simulation_ticks++;
        }
        frame_simulation_ticks = simulation_ticks;

        alpha = accumulator / config::FIXED_DELTATIME;
        debugInfo.set_frame_stats(frame_deltatime, accumulator, simulation_ticks, frame_cps);
       
        // STEP 2:          TEXTURE | Draw to texture cycle
        // ===========================================

        renderer.begin_texture_frame(canvas);

        //  Render Logic, VARIABLE TIME
        game.TickPhase(Phases::RENDERING, alpha);

        renderer.end_texture_frame();
        renderer.present(canvas, game, alpha);   // Begin Draw -> Draw Pro texture -> End Draw

    }

    game.CleanUp();

    // Close the program
    window.EndProgram();
} 







void CoreApplication::RunCoreEngineMainEditor (GameEngine & game) {

    // Do nothing

}


