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

class Window;

class Renderer;


class RenderSystem : public System {

    private:

        Renderer & renderer;
        Scene & scene;
        Window & window;
        bool renderSceneEditorUI = false;

    public:

        RenderSystem (Renderer & _renderer, Scene & _scene, Window & _window) : renderer(_renderer), scene(_scene), window(_window) {}

        RenderSystem (Renderer & _renderer, Scene & _scene, bool _editor, Window & _window) : renderer(_renderer), scene(_scene), renderSceneEditorUI(_editor), window(_window) {} 

        void update (Registry & registry, float deltatime) override;


};





#endif 
