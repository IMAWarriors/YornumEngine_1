// MovementSystem.h

#ifndef ENGINEMANAGERSYSTEM_H
#define ENGINEMANAGERSYSTEM_H

#include "../../Engine/Core/Overhead/WindowStats.h"

// Get System
#include "../../Engine/ECS/System.h"

// Get Entity
#include "../../Engine/ECS/Entity.h"

// Get Specific Components
#include "../Components/Components.h"

#include "../../Engine/Core/Overhead/FrameStats.h"


class EngineManagerSystem : public System {

    private:

        FrameStats & frame;

    public: 

        EngineManagerSystem (FrameStats & _frame) : frame(_frame) {}
    
        void update (Registry & registry, float deltatime) override;

    


};

















#endif
