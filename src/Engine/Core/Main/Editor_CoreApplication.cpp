// Editor_CoreApplication.cpp

#include "CoreApplication.h"

#include "../../../../external/rlimgui/rlImGui.h"

#include "../../../Gamefiles/Game/GameEngine.h"

// =========================================================================================
// EDITOR BUILD
// =========================================================================================

void CoreApplication::RunCoreEngineMainEditor (GameEngine & game) {

    game.SetGameFlag(Flags::EDITOR, true);

    // Initialize window and program
    Window window("Scene Editor & Manager", true);

    // Initialize for GUI drawing
    rlImGuiSetup(true);

    // Initialize Raylib Drawing Aspects
    Renderer            renderer;         
    RenderTexture2D     canvas;           // Intialize CANVAS to draw game onto
    InputManager        inputManager;
    FrameStats          frame;

    renderer.init_canvas(canvas);
    game.Initialize(renderer, inputManager, frame, window);
    
    float accumulator = 0.0f;   // Time bucket
    float frame_deltatime = 0.0f;
    float frame_cps = 0.0f;
    float frame_simulation_ticks = 0.0f;

    float alpha = 0.0f;



    while ( window.IsRunning() ) {


        // STEP 0:          SHADER HANDLING???

        bool next_frame = (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER));

        if (!G_DEBUGGER.MODE_ONEFRAME || (G_DEBUGGER.MODE_ONEFRAME && next_frame)) {

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
            frame.update(frame_deltatime, accumulator, simulation_ticks, frame_cps);

            // STEP 2:          TEXTURE | Draw to texture cycle
            // ===========================================

            renderer.begin_texture_frame(canvas);

            // BeginShaderMode(window.painter);

            //  Render Logic, VARIABLE TIME
            game.TickPhase(Phases::RENDERING, alpha);

            // EndShaderMode();

            renderer.end_texture_frame();
            renderer.present(canvas, game, alpha, window);   // Begin Draw -> Draw Pro texture -> End Draw

        }

    }

    // Close the program

    game.CleanUp();

    rlImGuiShutdown();
    window.EndProgram();

    


}



void CoreApplication::RunCoreEngine(GameEngine & game, const std::string & scenepath) {

    // Do nothing

}

