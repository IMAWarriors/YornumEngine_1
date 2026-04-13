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

class Window;


class EditorUISystem : public System {

    private:

        Renderer & renderer;
        Scene & scene;
        AssetManager & assets;
        EditorAssets & editorAssets;
        Window & window;


        float imgsizeMax = 512.0f;


        const std::string SCENEDIR = "assets/scenes/";
        const std::string TILESETDIR = "assets/sprites/tilesets/";
        const std::string BACKGROUNDIMAGEDIR = "assets/sprites/backgrounds/";

    public:

        EditorUISystem (Renderer & _renderer, Scene & _scene, AssetManager & _assets, EditorAssets & _editorAssets, Window & _window) : renderer(_renderer), scene(_scene), assets(_assets), editorAssets(_editorAssets), window(_window) {

            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = "../../../cache/imgui/EMain_imgui.ini";

        }
        
    
        void update (Registry & registry, float deltatime) override;

    


};


#endif