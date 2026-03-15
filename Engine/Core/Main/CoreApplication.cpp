// CoreApplication.cpp

#include "CoreApplication.h"


void CoreApplication::RunCoreEngine(GameEngine & game) {


    // Initialize window and program
    Window window("古残阳: Old Sun || Alpha v1.0.0 ");

    // Initialize Raylib Drawing Aspects
    Renderer            renderer;         
    RenderTexture2D     canvas;           // Intialize CANVAS to draw game onto
    InputManager inputManager;

    DebugStats debugInfo;


    renderer.init_canvas(canvas);
    game.Initialize(renderer, inputManager, debugInfo);

    float accumulator = 0.0f;   // Time bucket


    while ( window.IsRunning() ) {

        // STEP 1:          LOGIC  | Update logic and game frame state
        // ==========================================
        
        float frame_deltatime = GetFrameTime();
        frame_deltatime = std::min(frame_deltatime, config::MAX_FRAME_LAG);

        accumulator += frame_deltatime; // Store how much time passed last frame

        window.ListenFullscreenToggle();
        
        //  Input Logic, VARIABLE TIME
        game.TickPhase(Phases::INPUT, frame_deltatime);

        //  Simulaiton Logic, FIXED TIME

        int simulation_ticks = 0;

        while (accumulator >= config::FIXED_DELTATIME) {
            game.TickPhase(Phases::SIMULATION, config::FIXED_DELTATIME);
            accumulator -= config::FIXED_DELTATIME;
            simulation_ticks++;
        }

        debugInfo.set_frame_stats(frame_deltatime, accumulator, simulation_ticks);
       
        // STEP 2:          TEXTURE | Draw to texture cycle
        // ===========================================

        renderer.begin_texture_frame(canvas);

        //  Render Logic, VARIABLE TIME
        game.TickPhase(Phases::RENDERING, frame_deltatime);

        renderer.end_texture_frame();
        renderer.present(canvas);   // Begin Draw -> Draw Pro texture -> End Draw

    }

    // Close the program
    window.EndProgram();



} 




