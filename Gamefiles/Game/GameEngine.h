// GameEngine.h

#ifndef GAMEENGINE_H
#define GAMEENGINE_H

// Get Engine Tools
#include "../../Engine/ECS/SystemManager.h"
#include "../../Engine/ECS/ComponentPool.h"               // Get the GENERAL COMPONENT POOL INFORMATION for Component refs

// Get Systems
#include "../Systems/RenderSystem.h"       
#include "../Systems/MovementSystem.h"
#include "../Systems/PlayerControllerSystem.h"
#include "../Systems/InputSystem.h"

#include "../../Engine/ECS/Enum_SystemPhases.h"


class Renderer;

class InputManager;

class GameEngine {

    private:

        // Public Systems Managers
    
        SystemManager systems;

        // GameEngine Attribtues

        // Camera    camera;
        Registry registry;

        // Pointers to CoreApplication Attributes
        Renderer * renderer = nullptr;
        InputManager * input = nullptr;
        
    

    public:

        void Initialize(Renderer & _renderer, InputManager & _input);

        void TickPhase (Phases phase, float deltatime);





};




#endif

