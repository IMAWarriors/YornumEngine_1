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

        bool RUNNING_EDITOR = false;
        
     
    public:


        bool IsRunningEditor();

        void SetEditorMode();

        void Initialize(Renderer & _renderer, InputManager & _input, DebugStats & _debug);

        void TickPhase (Phases phase, float deltatime);

        void AddEditorSystems();

        void CleanUp();




};




#endif

