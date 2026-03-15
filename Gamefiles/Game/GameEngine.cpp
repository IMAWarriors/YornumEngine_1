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


    // Entities Initialization
    Entity player = spawndef::SpawnPlayer(registry);






}


void GameEngine::TickPhase (Phases phase, float deltatime) {

    // Execute update for systems 
    systems.execute_phase(phase, registry, deltatime);

}


