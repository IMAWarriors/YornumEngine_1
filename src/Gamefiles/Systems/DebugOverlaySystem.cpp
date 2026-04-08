// DebugOverlaySystem.cpp

#include "DebugOverlaySystem.h"

#include "../World/Overhead/Gwconst.h"


void DebugOverlaySystem::update (Registry & registry, float deltatime)  {

    

    /*DEPRICATED
    for (auto port : debug_info.float_ports) {
        if (port.rounding == 0) {
            renderer.rdraw_text(renderer.text("%s: %.0f", port.label.c_str(), *(port.data)), (int)(port.x), (int)(port.y), 16, WHITE);
        } else if (port.rounding == 4) {
            renderer.rdraw_text(renderer.text("%s: %.4f", port.label.c_str(), *(port.data)), (int)(port.x), (int)(port.y), 16, WHITE);
        } 
    }
    */


    // DRAW FRAME STATS

    for (Entity entity : registry.view<tag::EngineManager>()) {
        const auto & frame = registry.get_component<comp::FramerateTracker>(entity);
        const auto & mouse = registry.get_component<comp::MouseTracker>(entity);

        G_DEBUGGER.push({std::to_string((int)frame.frames_per_second) + " FPS", {15,15}, 18, WHITE});
        G_DEBUGGER.push({std::to_string((int)mouse.screen_mouse_position.x) + " , " + std::to_string((int)mouse.screen_mouse_position.y), {15,35}, 18, WHITE});

        G_DEBUGGER.push({"World", {15,60}, 18, WHITE});
        G_DEBUGGER.push({std::to_string((int)mouse.world_mouse_position.x) + " , " + std::to_string((int)mouse.world_mouse_position.y), {15,80}, 18, WHITE});

        G_DEBUGGER.push({"Col: " + std::to_string(( (int)(std::floor(mouse.world_mouse_position.x / (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS)) )), {15,100}, 18, WHITE});
        G_DEBUGGER.push({"Row: " + std::to_string(( (int)(std::floor(mouse.world_mouse_position.y / (float)gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS)) )), {15,120}, 18, WHITE});
        
    }


    // DRAW PLAYER STATS

    for (Entity entity : registry.view<tag::Player>()) {
        const auto & player_input = registry.get_component<comp::InputState>(entity);
        const auto & player_pos = registry.get_component<comp::Transform>(entity);
        const auto & player_vel = registry.get_component<comp::Velocity>(entity);

        const auto & player_body = registry.get_component<comp::PhysicsBody>(entity);

        G_DEBUGGER.push({"Player", {1000,15}, 18, WHITE});
        G_DEBUGGER.push({"[ " + std::to_string(player_input.horz_axis) + " , " + std::to_string(player_input.vert_axis) + " ] ", {1000,35}, 18, WHITE});
        G_DEBUGGER.push({"( " + std::to_string((int)player_pos.position.x) + " , " + std::to_string((int)player_pos.position.y) + " ) ", {1000,55}, 18, WHITE});
        G_DEBUGGER.push({"< " + std::to_string((int)player_vel.magnitude.x) + " , " + std::to_string((int)player_vel.magnitude.y) + " > ", {1000,75}, 18, WHITE});
        if (player_body.inColl) {
            G_DEBUGGER.push({"Body Collision: INSIDE", {1000,150}, 18, RED});
        } else {
            G_DEBUGGER.push({"Body Collision: outside", {1000,150}, 18, GREEN});
        }
        if (player_body.innerSkinInColl) {
            G_DEBUGGER.push({"Inskin Collision: INSIDE", {1000,170}, 18, RED});
        } else {
            G_DEBUGGER.push({"Inskin Collision: outside", {1000,170}, 18, GREEN});
        }

    }

    // DRAW CAMERA STATS

    for (Entity entity : registry.view<comp::Camera>()) {
        const auto & camera_pos = registry.get_component<comp::Transform>(entity).position;
        const auto & camera_zoom = registry.get_component<comp::Camera>(entity).zoom;
        
        G_DEBUGGER.push({"Camera", {1000, 100}, 18, WHITE});
        G_DEBUGGER.push({"( " + std::to_string((int)camera_pos.x) + " , " + std::to_string((int)camera_pos.y) + " )  : " + std::to_string((int)camera_zoom), {1000,120}, 18, WHITE});

    }

    
    

}

/*
void DebugOverlaySystem::draw_text (const std::string & text, Vec2 position, int size, ColorRGBA color) {

    renderer.rdraw_text(text, (int)(position.x), (int)(position.y), size, {color.r, color.g, color.b, color.a});

}

*/












