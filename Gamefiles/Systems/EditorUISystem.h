// EditorUISystem.h

#ifndef EDITORUISYSTEM_H
#define EDITORUISYSTEM_H

// Get System
#include "../../Engine/ECS/System.h"

// Get Entity
#include "../../Engine/ECS/Entity.h"

// Get Specific Components
#include "../Components/Components.h"

#include <vector>
#include <string>

class Scene;

class Renderer;

class AssetManager;


class EditorUISystem : public System {

    private:

        Renderer & renderer;
        Scene & scene;
        AssetManager & assets;


    public: 

        EditorUISystem (Renderer & _renderer, Scene & _scene, AssetManager & _assets) : renderer(_renderer), scene(_scene), assets(_assets) {}
        
    
        void update (Registry & registry, float deltatime) override;

    


};


#endif