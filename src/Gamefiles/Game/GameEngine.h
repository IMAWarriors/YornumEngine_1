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




#include "../World/Scene.h"



class Renderer;

class InputManager;

struct DebugStats;

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



        // Pointers to CoreApplication Attributes
        Renderer * renderer = nullptr;
        InputManager * input = nullptr;
        DebugStats * debug = nullptr;
        Window * window = nullptr;

        bool RUNNING_EDITOR = false;




        // Very important that initialization does not edit these thoings
        bool GAME_FLAGS [100] = {false};

        // GAME_FLAGS INFORMATION
        /*********************************
         *  [0]         DETAILED_DEBUG          Verbose debugging menus and options
         * 
         * 
         * 
         * 
         * 
         */
        
        
     
    public:

        bool IsRunningEditor();

        void SetEditorMode();

        void Initialize(Renderer & _renderer, InputManager & _input, DebugStats & _debug, Window & _window);

        void TickPhase (Phases phase, float deltatime);

        void AddEditorSystems();

        void CleanUp();

        bool LoadScene (const std::string & scenename);

        void SetGameFlag(int flag, bool state) {
            if (100 > flag && flag >= 0) {
                GAME_FLAGS[flag] = state;
            }
        }

};




#endif

