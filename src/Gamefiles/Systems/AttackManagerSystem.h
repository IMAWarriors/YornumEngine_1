// AttackManagerSystem.h

#ifndef ATTACKMANAGERSYSTEM_H
#define ATTACKMANAGERSYSTEM_H



// Get System
#include "../../Engine/ECS/System.h"
// Get Entity
#include "../../Engine/ECS/Entity.h"
// Get Specific Components
#include "../Components/Components.h"
#include "../../Gamefiles/World/Scene.h"

#include "../Elements/Avatar/Avatar.h"
#include "../Elements/Avatar/Animation.h"
#include "../../Gamefiles/World/Scene.h"

#include <vector>


class AttackManagerSystem : public System {
    private:
        Scene & scene;
        Renderer & renderer;
    public: 
        AttackManagerSystem (Scene & _scene, Renderer & _renderer) : scene(_scene), renderer(_renderer) {}
        void update (Registry & registry, float deltatime) override;
};









#endif