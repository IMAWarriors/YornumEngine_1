// InputSystem.h

#ifndef INPUTSYSTEM_H
#define INPUTSYSTEM_H

// Get input wrapper (raylib) platform
#include "../../Engine/Platform/Input/InputManager.h"

// Get System
#include "../../Engine/ECS/System.h"

// Get Entity
#include "../../Engine/ECS/Entity.h"

// Get Specific Components
#include "../Components/comp/InputState.h"

class InputManager;


class InputSystem : public System {

    private:

        InputManager & input;

    public: 

        InputSystem(InputManager & _input) : input(_input) {};
    
        void update (Registry & registry, float deltatime);

};



#endif




