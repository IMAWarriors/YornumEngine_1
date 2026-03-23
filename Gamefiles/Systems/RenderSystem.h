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

class Scene;

class Renderer;


class RenderSystem : public System {

    private:

        Renderer & renderer;
        Scene & scene;
        bool renderSceneEditorUI = false;

    public:

        RenderSystem (Renderer & _renderer, Scene & _scene) : renderer(_renderer), scene(_scene) {}

        RenderSystem (Renderer & _renderer, Scene & _scene, bool _editor) : renderer(_renderer), scene(_scene), renderSceneEditorUI(_editor) {} 

        void update (Registry & registry, float deltatime) override;


};





#endif 
