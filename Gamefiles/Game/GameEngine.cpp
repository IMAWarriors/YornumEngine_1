// GameEngine.cpp

#include "GameEngine.h"


// Get Spawndefs
#include "../Spawndefs/Spawndefs.h"

void GameEngine::Initialize(Renderer & _renderer, InputManager & _input, DebugStats & _debug) {

    renderer = &_renderer;
    input    = &_input;
    debug    = &_debug;

    scene.load_new_tileset("Cave Tileset", assets.LoadTilesetTexture("Gamefiles/Assets/Sprites/Tilesets/cave_tileset.png"),32,10,7);
    scene.load_new_tileset("Test Tileset", assets.LoadTilesetTexture("Gamefiles/Assets/Sprites/Tilesets/tileset_test_1_A.png"),8,4,4);

    scene.tiles_push_new_layer();       // Pushes automatic test layer

    systems.add_system   <InputSystem>               (Phases::INPUT, *input);
    systems.add_system   <PlayerControllerSystem>    (Phases::SIMULATION);
    systems.add_system   <MovementSystem>            (Phases::SIMULATION);
    systems.add_system   <CameraSystem>              (Phases::SIMULATION, *renderer);
    systems.add_system   <EngineManagerSystem>       (Phases::SIMULATION, *debug);

    if (RUNNING_EDITOR) { 
        systems.add_system<RenderSystem>(Phases::RENDERING,*renderer,scene,true);
    } else {
        systems.add_system<RenderSystem>(Phases::RENDERING, *renderer, scene);
    }
    
    systems.add_system   <DebugOverlaySystem>        (Phases::RENDERING, *renderer, *debug);

    if (RUNNING_EDITOR) {
        AddEditorSystems();
    }    


    // Entities Initialization
    Entity player = spawndef::SpawnPlayer(registry);
    Entity camera = spawndef::SpawnCamera(registry, player, 5.0f);
    Entity engine = spawndef::SpawnEngineManager(registry);

    // Debug watchers

    comp::Transform & player_transform = registry.get_component<comp::Transform>(player);
    comp::Transform & camera_transform = registry.get_component<comp::Transform>(camera);
    comp::Camera    & camera_details = registry.get_component<comp::Camera>(camera);
    
    Vec2 & screen_mouse = registry.get_component<comp::MouseTracker>(engine).screen_mouse_position;
    Vec2 & world_mouse = registry.get_component<comp::MouseTracker>(engine).world_mouse_position;

    float & fps = registry.get_component<comp::FramerateTracker>(engine).frames_per_second;
    float & dt = registry.get_component<comp::FramerateTracker>(engine).frame_deltatime;
    float & ticks = registry.get_component<comp::FramerateTracker>(engine).frame_simulation_ticks;
    float & acc = registry.get_component<comp::FramerateTracker>(engine).accumulator;

    debug->add_watchport("FPS",fps,0);

    /*
    debug->add_watchport("DT",dt,4);
    debug->add_watchport("Acc",acc,4);
    debug->add_watchport("Ticks",ticks,0);
    
    debug->add_watchport("Player X",player_transform.position.x,0);
    debug->add_watchport("Player Y",player_transform.position.y,0);

    */

    if (RUNNING_EDITOR) {

        debug->set_watchport_list_position({10.0f, 45.0f});

        debug->add_watchport("Camera X",camera_transform.position.x,0);
        debug->add_watchport("Camera Y",camera_transform.position.y,0);
        debug->add_watchport("Camera Zoom", camera_details.zoom, 4);

        debug->add_watchport("Screen Mouse X",screen_mouse.x,0);
        debug->add_watchport("Screen Mouse Y",screen_mouse.y,0);

        debug->add_watchport("World Mouse X",world_mouse.x,0);
        debug->add_watchport("World Mouse Y",world_mouse.y,0);
    }
    

}


void GameEngine::TickPhase (Phases phase, float deltatime) {


    // Execute update for systems 
    systems.execute_phase(phase, registry, deltatime);

    if (phase == Phases::SIMULATION) {

        registry.execute_queue_operations();    // Clears queue as well

    }

}

bool GameEngine::IsRunningEditor () {
    return RUNNING_EDITOR;
}

void GameEngine::SetEditorMode () {
    RUNNING_EDITOR = true;
}


