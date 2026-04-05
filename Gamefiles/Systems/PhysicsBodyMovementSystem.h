// PhysicsBodyMovementSystem.h

#ifndef PHYSICSBODYMOVEMENTSYSTEM_H
#define PHYSICSBODYMOVEMENTSYSTEM_H

// Get System
#include "../../Engine/ECS/System.h"

// Get Entity
#include "../../Engine/ECS/Entity.h"

// Get Specific Components
#include "../Components/Components.h"

#include "../../Gamefiles/World/Scene.h"


class PhysicsBodyMovementSystem : public System {

    private:

        Scene & scene;

    public: 

        

        PhysicsBodyMovementSystem (Scene & _scene) : scene(_scene) {}
    
        void update (Registry & registry, float deltatime) override;

    


};

















#endif
