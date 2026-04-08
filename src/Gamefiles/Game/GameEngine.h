// GameEngine.h

#ifndef GAMEENGINE_H
#define GAMEENGINE_H

// Get Engine Tools
#include "../../Engine/ECS/SystemManager.h"
#include "../Assets/AssetManager.h"
#include "../Assets/EditorAssets.h"
#include "../../Engine/ECS/ComponentPool.h"               // Get the GENERAL COMPONENT POOL INFORMATION for Component refs

// Get Systems
#include "../Systems/Systems.h"

#include "../../Engine/ECS/Enum_SystemPhases.h"

#include "../../Tooling/Debug/DebugManager.h"
// #include "../../Tooling/Debug/Struct_DebugData.h"

/*
auto& player_transform = registry.get_component<comp::Transform>(player);
auto& camera_transform = registry.get_component<comp::Transform>(camera);
auto& camera_details = registry.get_component<comp::Camera>(camera);
auto& screen_mouse = registry.get_component<comp::MouseTracker>(engine).screen_mouse_position;
auto& world_mouse = registry.get_component<comp::MouseTracker>(engine).world_mouse_position;
float & fps = registry.get_component<comp::FramerateTracker>(engine).frames_per_second;
float & dt = registry.get_component<comp::FramerateTracker>(engine).frame_deltatime;
float & ticks = registry.get_component<comp::FramerateTracker>(engine).frame_simulation_ticks;
float & acc = registry.get_component<comp::FramerateTracker>(engine).accumulator;

renderer->rdraw_text(std::string("Camera X: ")           +   std::to_string(camera_transform.position.x),   10, 45, 18, WHITE);
renderer->rdraw_text(std::string("Camera Y: ")           +   std::to_string(camera_transform.position.y),   10, 65, 18, WHITE);
renderer->rdraw_text(std::string("Camera Zoom: ")        +   std::to_string(camera_details.zoom),           10, 85, 18, WHITE);
renderer->rdraw_text(std::string("Screen Mouse X: ")     +   std::to_string(screen_mouse.x),                10, 105, 18, WHITE);
renderer->rdraw_text(std::string("Screen Mouse Y: ")     +   std::to_string(screen_mouse.y),                10, 125, 18, WHITE);
renderer->rdraw_text(std::string("World Mouse X: ")      +   std::to_string(world_mouse.x),                 10, 145, 18, WHITE);
renderer->rdraw_text(std::string("World Mouse Y: ")      +   std::to_string(world_mouse.x),                 10, 165, 18, WHITE);
*/



#include "../World/Scene.h"

enum class Flags {
    EMPTY       = 0,
    EDITOR,
    DEBUG,
    FLAG_COUNT
};

class Renderer;

class InputManager;

struct FrameStats;

class Window;

class GameEngine {

    private:

        // Public Systems Managers
    
        SystemManager systems;

        // GameEngine Attribtues

        // Camera    camera;
        Registry registry;

        // Scene?
        Scene scene;

        AssetManager assets;
        EditorAssets editorAssets;

        // DebugManager debug;

        // Pointers to CoreApplication Attributes
        Renderer * renderer = nullptr;
        InputManager * input = nullptr;
        FrameStats * frame = nullptr;
        Window * window = nullptr;

        

        // bool RUNNING_EDITOR = false;

        bool GAME_FLAGS [(int)(Flags::FLAG_COUNT)] = {false};

        
     
    public:

        /*
        bool IsRunningEditor();

        void SetEditorMode();
        */

        void Initialize(Renderer & _renderer, InputManager & _input, FrameStats & _frame, Window & _window);

        void TickPhase (Phases phase, float deltatime);

        void AddEditorSystems();

        void CleanUp();

        bool LoadScene (const std::string & scenename);

        void SetGameFlag(int flag, bool state) {
            if ((int)(Flags::FLAG_COUNT) > flag && flag >= 0) {
                GAME_FLAGS[flag] = state;
            }
        }

        void SetGameFlag(Flags flag, bool state) {
            if ((int)(Flags::FLAG_COUNT) > (int)flag && (int)flag >= 0) {
                GAME_FLAGS[(int)flag] = state;
            }
        }

        bool CheckFlag (int flag) const {
            if ((int)(Flags::FLAG_COUNT) > flag && flag >= 0) {
                return (GAME_FLAGS[flag]);
            }
            return false;
        }

        bool CheckFlag (Flags flag) const {
            if ((int)(Flags::FLAG_COUNT) > (int)flag && (int)flag >= 0) {
                return (GAME_FLAGS[(int)flag]);
            }
            return false;
        }
        

};




#endif

