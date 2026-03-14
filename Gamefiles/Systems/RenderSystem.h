// RenderSystem.h


#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

// Does not know about raylib
// But interacts with Renderer.cpp to render

// Get System
#include "../../Engine/ECS/System.h"

// Get Entity
#include "../../Engine/ECS/Entity.h"

// Get Specific Components
#include "../Components/Components.h"


class Renderer;


class RenderSystem : public System {

    private:

        Renderer & renderer;

    public:

        RenderSystem (Renderer & _renderer) : renderer(_renderer) {}

        void update (Registry & registry, float deltatime) override;


};





#endif 
