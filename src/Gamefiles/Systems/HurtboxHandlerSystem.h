// HurtboxHandlerSystem.h

#ifndef HURTBOXHANDLERSYSTEM_H
#define HURTBOXHANDLERSYSTEM_H

#include "../../Engine/Core/Overhead/WindowStats.h"
#include "../../Engine/ECS/System.h"
#include "../../Engine/ECS/Entity.h"
#include "../Components/Components.h"
#include "../../Engine/Core/Overhead/FrameStats.h"
#include "../World/Scene.h"


class HurtboxHandlerSystem : public System {

    private:

        Scene& scene;

    public: 

        HurtboxHandlerSystem (Scene& scene) : scene(scene) {}
    
        void update (Registry & registry, float deltatime) override;

    


};

#endif
