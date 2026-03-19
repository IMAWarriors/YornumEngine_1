// GameEngine.cpp

#include "GameEngine.h"

// Get Spawndefs
#include "../Spawndefs/Spawndefs.h"

void GameEngine::Initialize(Renderer & _renderer, InputManager & _input, DebugStats & _debug) {

    renderer = &_renderer;
    input    = &_input;
    debug    = &_debug;

    systems.add_system   <InputSystem>               (Phases::INPUT, *input);
    systems.add_system   <PlayerControllerSystem>    (Phases::SIMULATION);
    systems.add_system   <MovementSystem>            (Phases::SIMULATION);
    systems.add_system   <RenderSystem>              (Phases::RENDERING, *renderer);
    systems.add_system   <DebugOverlaySystem>        (Phases::RENDERING, *renderer, *debug);
    systems.add_system   <CameraSystem>              (Phases::RENDERING, *renderer);


    // Entities Initialization
    Entity player = spawndef::SpawnPlayer(registry);

    Entity camera = spawndef::SpawnCamera(registry, player);

    // Debug watchers

    comp::Transform & player_transform = registry.get_component<comp::Transform>(player);
    comp::Transform & camera_transform = registry.get_component<comp::Transform>(camera);
    

    debug->add_watchport("Player X",player_transform.position.x,0);
    debug->add_watchport("Player Y",player_transform.position.y,0);
    debug->add_watchport("Camera X",camera_transform.position.x,0);
    debug->add_watchport("Camera Y",camera_transform.position.y,0);



}


void GameEngine::TickPhase (Phases phase, float deltatime) {

    // Execute update for systems 
    systems.execute_phase(phase, registry, deltatime);

    if (phase == Phases::SIMULATION) {

        

    }

}


