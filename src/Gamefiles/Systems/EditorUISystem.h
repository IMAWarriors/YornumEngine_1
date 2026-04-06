// EditorUISystem.h

#ifndef EDITORUISYSTEM_H
#define EDITORUISYSTEM_H

// Get System
#include "../../Engine/ECS/System.h"

// Get Entity
#include "../../Engine/ECS/Entity.h"

// Get Specific Components
#include "../Components/Components.h"

#include "../../../external/rlimgui/rlImGui.h"
#include "../../../external/imgui/imgui.h"

#include <vector>
#include <string>

class Scene;

class Renderer;

class AssetManager;

struct EditorAssets;


class EditorUISystem : public System {

    private:

        Renderer & renderer;
        Scene & scene;
        AssetManager & assets;
        EditorAssets & editorAssets;


        float imgsizeMax = 512.0f;


        const std::string SCENEDIR = "assets/scenes/";
        const std::string TILESETDIR = "assets/sprites/tilesets/";
        

    public:

        EditorUISystem (Renderer & _renderer, Scene & _scene, AssetManager & _assets, EditorAssets & _editorAssets) : renderer(_renderer), scene(_scene), assets(_assets), editorAssets(_editorAssets) {

            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = "../../../cache/imgui/EMain_imgui.ini";

        }
        
    
        void update (Registry & registry, float deltatime) override;

    


};


#endif