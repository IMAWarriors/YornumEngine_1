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

class Renderer;
class PhysicsBodyMovementSystem : public System {
    private:
        Scene & scene;
        Renderer & renderer;
    public: 
        PhysicsBodyMovementSystem (Scene & _scene, Renderer & _renderer) : scene(_scene), renderer(_renderer) {}
        void update (Registry & registry, float deltatime) override;
};
#endif