// GameEngine.cpp

#include "GameEngine.h"

DebugManager G_DEBUGGER;

// Get Spawndefs
#include "../Spawndefs/Spawndefs.h"

void GameEngine::Initialize(Renderer & _renderer, InputManager & _input, FrameStats & _frame, Window & _window) {

    renderer = &_renderer;
    input    = &_input;
    frame    = &_frame;
    window   = &_window;

    if (CheckFlag(Flags::EDITOR) || CheckFlag(Flags::DEBUG)) {
        G_DEBUGGER.init(_renderer);
    }

    editorAssets.init();

    scene.new_scene();


    systems.add_system   <InputSystem>               (Phases::INPUT, *input);
    systems.add_system   <PlayerControllerSystem>    (Phases::SIMULATION);
    systems.add_system   <PhysicsBodyMovementSystem> (Phases::SIMULATION, scene, *renderer);
    systems.add_system   <CameraClampManagerSystem>  (Phases::SIMULATION, scene);
    systems.add_system   <CameraSystem>              (Phases::SIMULATION, *renderer);
    systems.add_system   <EngineManagerSystem>       (Phases::SIMULATION, *frame);


    if (CheckFlag(Flags::EDITOR)) { 
        systems.add_system<RenderSystem>(Phases::RENDERING,*renderer,scene,true, *window);
    } else {
        systems.add_system<RenderSystem>(Phases::RENDERING, *renderer, scene, *window);
    }
    
    systems.add_system   <DebugOverlaySystem>        (Phases::RENDERING, *renderer, *frame);

    if (CheckFlag(Flags::EDITOR)) {
        AddEditorSystems();
        G_DEBUGGER.moveoffset(0, 30);
    }    


    // Entities Initialization
    Entity player = spawndef::SpawnPlayer(registry);
    Entity cameraManager = spawndef::SpawnCameraManager(registry);
    Entity camera = spawndef::SpawnCamera(registry, player, 5.0f);
    Entity engine = spawndef::SpawnEngineManager(registry);




    if (CheckFlag(Flags::EDITOR) || CheckFlag(Flags::DEBUG)) {
     

    }
    

}


void GameEngine::TickPhase (Phases phase, float deltatime) {
    // Execute update for systems 
    systems.execute_phase(phase, registry, deltatime);

    if (phase == Phases::SIMULATION) {
        registry.execute_queue_operations();    // Clears queue as well
    }

    if (phase == Phases::RENDERING) {
        if (CheckFlag(Flags::DEBUG) || CheckFlag(Flags::EDITOR)) {
            G_DEBUGGER.draw();
        }
    }
    
}


bool GameEngine::LoadScene (const std::string & scenename) {
    bool success = scene.load_scene_by_name(scenename, assets);
    return success;
}



void GameEngine::CleanUp() {
    editorAssets.shutdown();
}

void DebugManager::push(DebugData data) {
    if (renderer != nullptr) {
        queue.push_back(data);
    }
}

void DebugManager::push(const std::string & text, Vec2 position) {
    if (renderer != nullptr) {
        queue.push_back({text, position, 18, WHITE});
    }
}

void DebugManager::push(const std::string & text, Vec2 position, int size) {
    if (renderer != nullptr) {
        queue.push_back({text, position, size, WHITE});
    }
}

void DebugManager::push(const std::string & text, Vec2 position, Color color) {
    if (renderer != nullptr) {
        queue.push_back({text, position, 18, color});
    }
}


void DebugManager::draw() {
    if (renderer != nullptr) {
        for (DebugData data : queue) {

            std::string str = data.text;
            int x           = (int)data.position.x + offset_x;
            int y           = (int)data.position.y + offset_y;
            int size        = data.size;
            Color color     = data.color;

            renderer->rdraw_text(str, x, y, size, color);
        }
    }
    queue.clear();
}

void DebugManager::moveoffset (int x, int y) {

    if (renderer != nullptr) {
        offset_x += x;
        offset_y += y;
    }
            
}



/*
bool GameEngine::IsRunningEditor () {
    return RUNNING_EDITOR;
}

void GameEngine::SetEditorMode () {
    RUNNING_EDITOR = true;
}*/
