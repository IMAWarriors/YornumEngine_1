// EngineManagerSystem.cpp

#include "EngineManagerSystem.h"

void EngineManagerSystem::update (Registry & registry, float deltatime) {

    Vec2 camera_position = {0.0f, 0.0f};

    for (Entity entity : registry.view<comp::Camera>()) {
        camera_position = registry.get_component<comp::Transform>(entity).position;
        break;
    }

    

    for (Entity entity : registry.view<comp::MouseTracker, comp::FramerateTracker>()) {      // For each iteration of Entity


        // To get to this point, we have confirmed this entity has both VELOCITY and TRANSFORM, so should be valid to get component
        // Go ahead and grab struct references to the components we want to manipulate so we can edit them directly

        Vec2 & screen_position = registry.get_component<comp::MouseTracker>(entity).screen_mouse_position;
        Vec2 & world_position = registry.get_component<comp::MouseTracker>(entity).world_mouse_position;

        float & frames_per_second = registry.get_component<comp::FramerateTracker>(entity).frames_per_second;
        float & frame_deltatime = registry.get_component<comp::FramerateTracker>(entity).frame_deltatime;
        float & accumulator = registry.get_component<comp::FramerateTracker>(entity).accumulator;
        float & frame_simulation_ticks = registry.get_component<comp::FramerateTracker>(entity).frame_simulation_ticks;

    
        // Transform

        screen_position = winstats::ScreenMousePosition();
        world_position = {camera_position.x + screen_position.x, camera_position.y + screen_position.y};


        // frames_per_second

        frames_per_second = debugInfo.frames_per_second;
        frame_deltatime = debugInfo.frame_deltatime;
        accumulator = debugInfo.frame_accumulator;
        frame_simulation_ticks = debugInfo.frame_simulation_ticks;


    }

}