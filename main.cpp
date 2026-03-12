// main.cpp

// C++ Util Includes
#include <algorithm>
#include <cmath>

// Get Engine Assets
#include "Engine/Core/Config.h"                     // Get the CONFIGURATION CONSTNATS

#include "Engine/Window/Window.h"

#include "Engine/Platform/Input/InputManager.h"     // Get the INPUTS LAYER INFO

#include "Engine/Rendering/Renderer.h"              // Get the RENDERER PIPELINE ASSETS

#include "Engine/ECS/ComponentPool.h"               // Get the GENERAL COMPONENT POOL INFORMATION for Component refs
#include "Engine/ECS/SystemManager.h"   

// Get Systems
#include "Gamefiles/Systems/RenderSystem.h"       
#include "Gamefiles/Systems/MovementSystem.h"
#include "Gamefiles/Systems/PlayerControllerSystem.h"
#include "Gamefiles/Systems/InputSystem.h"

// Get Components
#include "Gamefiles/Components/Components.h"

// Get Spawndefs
#include "Gamefiles/Spawndefs/Spawndefs.h"





int main () {

    // Initialize window and program
    
    Window window("古残阳: Old Sun || Alpha v1.0.0 ");

    // Initialize Raylib Drawing Aspects
    Renderer            renderer;         
    RenderTexture2D     canvas;           // Intialize CANVAS to draw game onto

    renderer.init_canvas(canvas);

    // Public Systems Managers
    InputManager        inputManager;
    SystemManager       systems;
    RenderSystem        renderSystem(renderer);

    

    // Public Game Concept Initialization
    Camera    camera;
    Registry  registry;
    
    // Systems Initialization
    
    systems.add_system  <InputSystem>               (inputManager);

    systems.add_system  <PlayerControllerSystem>    ();

    systems.add_system  <MovementSystem>            ();
    


    // Entities Initialization
    Entity player = spawndef::SpawnPlayer(registry);




    


    while ( !WindowShouldClose() ) {

        // STEP 1:          LOGIC  | Update logic and game frame state

        float deltatime = GetFrameTime();

        window.ListenFullscreenToggle();
        
        systems.update_all(registry, deltatime);
        

        // STEP 2:          TEXTURE | Draw to texture cycle

        renderer.begin_texture_frame(canvas);

        renderSystem.update(registry, deltatime);

        // STEP 2.5  // Draw Debug Overview

        int OUT_P_X = (int)registry.get_component<comp::Transform>(player).position.x;
        int OUT_P_Y = (int)registry.get_component<comp::Transform>(player).position.y;
        int OUT_V_X = (int)registry.get_component<comp::Velocity>(player).magnitude.x;
        int OUT_V_Y = (int)registry.get_component<comp::Velocity>(player).magnitude.y;


        DrawText(TextFormat("Position: ( X: %d Y: %d )", OUT_P_X, OUT_P_Y), 10, 10, 20, WHITE);
        DrawText(TextFormat("Velocity: < X: %d Y: %d >", OUT_V_X, OUT_V_Y), 10, 30, 20, WHITE);


        // STEP 3:           DRAW   | Draw texture to screen

        renderer.end_texture_frame();
        renderer.present(canvas);   // Begin Draw -> Draw Pro texture -> End Draw

    }

    // Close the program
    window.EndProgram();

    return 0;

}


