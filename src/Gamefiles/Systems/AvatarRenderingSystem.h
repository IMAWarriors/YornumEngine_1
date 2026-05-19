// AvatarRenderingSystem.h

#ifndef AVATARRENDERINGSYSTEM_H
#define AVATARRENDERINGSYSTEM_H

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


class AvatarRenderingSystem : public System {
    private:
        Scene & scene;
        Renderer & renderer;
    public: 
        AvatarRenderingSystem (Scene & _scene, Renderer & _renderer) : scene(_scene), renderer(_renderer) {}
        void update (Registry & registry, float deltatime) override;
};
#endif