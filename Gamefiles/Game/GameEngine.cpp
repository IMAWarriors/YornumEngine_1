// GameEngine.cpp

#include "GameEngine.h"

// Get Spawndefs
#include "../Spawndefs/Spawndefs.h"

void GameEngine::Initialize(Renderer & _renderer, InputManager & _input) {

    renderer = &_renderer;
    input    = &_input;

    systems.add_system   <InputSystem>               (Phases::INPUT, *input);
    systems.add_system   <PlayerControllerSystem>    (Phases::SIMULATION);
    systems.add_system   <MovementSystem>            (Phases::SIMULATION);
    systems.add_system   <RenderSystem>              (Phases::RENDERING, *renderer);


    // Entities Initialization
    Entity player = spawndef::SpawnPlayer(registry);



}


void GameEngine::TickPhase (Phases phase, float deltatime) {

    // Execute update for systems 
    systems.execute_phase(phase, registry, deltatime);

}


