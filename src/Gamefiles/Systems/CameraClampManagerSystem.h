// CameraClampManagerSystem.h

#ifndef CAMERACLAMPMANAGERSYSTEM_H
#define CAMERACLAMPMANAGERSYSTEM_H


// Get System
#include "../../Engine/ECS/System.h"

// Get Entity
#include "../../Engine/ECS/Entity.h"

// Get Specific Components
#include "../Components/Components.h"

// class Renderer;


class Scene;

class CameraClampManagerSystem : public System {


    private:

        // Renderer & renderer;
        Scene & scene;

    public: 

        // CameraSystem (Renderer & renderer) : renderer(renderer) {}

        CameraClampManagerSystem(Scene & _scene) : scene(_scene) {}
    
        void update (Registry & registry, float deltatime) override;

        

    


};




#endif