// CoreApplication.cpp

#include "CoreApplication.h"


void CoreApplication::RunCoreEngine(GameEngine & game) {


    // Initialize window and program
    Window window("古残阳: Old Sun || Alpha v1.0.0 ");

    // Initialize Raylib Drawing Aspects
    Renderer            renderer;         
    RenderTexture2D     canvas;           // Intialize CANVAS to draw game onto
    InputManager inputManager;

    renderer.init_canvas(canvas);
    game.Initialize(renderer, inputManager);


    while ( window.IsRunning() ) {

        // STEP 1:          LOGIC  | Update logic and game frame state
        // ==========================================
        
        float deltatime = GetFrameTime();
        window.ListenFullscreenToggle();
        
        game.TickPhase(Phases::INPUT, deltatime);

        game.TickPhase(Phases::SIMULATION, deltatime);

       
        // STEP 2:          TEXTURE | Draw to texture cycle
        // ===========================================

        renderer.begin_texture_frame(canvas);

        game.TickPhase(Phases::RENDERING, deltatime);

        renderer.end_texture_frame();
        renderer.present(canvas);   // Begin Draw -> Draw Pro texture -> End Draw

    }

    // Close the program
    window.EndProgram();



} 




