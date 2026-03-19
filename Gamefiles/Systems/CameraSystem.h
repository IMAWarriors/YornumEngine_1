// CameraSystem.h

#ifndef CAMERASYSTEM_H
#define CAMERASYSTEM_H

// Get System
#include "../../Engine/ECS/System.h"

// Get Entity
#include "../../Engine/ECS/Entity.h"

// Get Specific Components
#include "../Components/Components.h"

class Renderer;


class CameraSystem : public System {


    private:

        Renderer & renderer;

    public: 

        CameraSystem (Renderer & renderer) : renderer(renderer) {}
    
        void update (Registry & registry, float deltatime) override;

        

    


};

















#endif
