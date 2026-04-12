// DebugOverlaySystem.cpp

#include "DebugOverlaySystem.h"

#include "../World/Overhead/Gwconst.h"

#include <sstream>
#include <iomanip>



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

    
    auto to_string_dp = [&](float value, int dp) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(dp) << value;
        return out.str();
    };


    


    // DRAW FRAME STATS

    for (Entity entity : registry.view<tag::EngineManager>()) {

        const auto & frame = registry.get_component<comp::FramerateTracker>(entity);
        const auto & mouse = registry.get_component<comp::MouseTracker>(entity);

        if (IsKeyPressed(KEY_RIGHT_SHIFT)) {
            // Right Shift pressed
            G_DEBUGGER.MODE_ONEFRAME = !G_DEBUGGER.MODE_ONEFRAME;
        }



        if (G_DEBUGGER.MODE_ONEFRAME == true) {

            G_DEBUGGER.push({"ONE FRAME | " + std::to_string((int)frame.frames_per_second) + " FPS", {15,15}, 18, WHITE});
            G_DEBUGGER.push({"Press ENTER to step simulation by 1 fixed tick", {15,140}, 18, YELLOW});
            
        } else {

            G_DEBUGGER.push({std::to_string((int)frame.frames_per_second) + " FPS", {15,15}, 18, WHITE});
            
        }


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



        std::string posx, posy, velx, vely;

        posx = to_string_dp(player_pos.position.x, 2);
        posy = to_string_dp(player_pos.position.y, 2);
        velx = to_string_dp(player_vel.magnitude.x, 2);
        vely = to_string_dp(player_vel.magnitude.y, 2);

        G_DEBUGGER.push({"Player", {1000,15}, 18, WHITE});
        G_DEBUGGER.push({"[ " + std::to_string(player_input.horz_axis) + " , " + std::to_string(player_input.jump_key) + " ] ", {1000,35}, 18, WHITE});
        G_DEBUGGER.push({"( " + posx + " , " + posy + " ) ", {1000,55}, 18, WHITE});
        G_DEBUGGER.push({"< " + velx + " , " + vely + " > ", {1000,75}, 18, WHITE});
        if (player_body.inColl) {
            G_DEBUGGER.push({"Body Collision: INSIDE", {1000,150}, 18, RED});
        } else {
            G_DEBUGGER.push({"Body Collision: outside", {1000,150}, 18, GREEN});
        }

        G_DEBUGGER.push({"[ " + std::to_string(player_body.falling) + " , " + std::to_string(player_body.vjump_window) + " ] ", {1000,190}, 18, WHITE});

        
        if (player_body.onSolidGround) {
            G_DEBUGGER.push({"Grounded", {1000,210}, 18, GREEN});
        } else {
            G_DEBUGGER.push({"Airborne", {1000,210}, 18, RED});
        }


        if (player_body.wallPush > 0) {
            G_DEBUGGER.push({"Pushing Against Right Wall", {1000,230}, 18, BLUE});
        } else if (player_body.wallPush < 0) {
            G_DEBUGGER.push({"Pushing Against Left Wall", {1000,230}, 18, PURPLE});
        }  else {
            G_DEBUGGER.push({"No active push", {1000,230}, 18, WHITE});
        }


        if (player_body.againstWall) {
            G_DEBUGGER.push({"CAN WALLJUMP", {1000,250}, 18, GREEN});
        } else {
            G_DEBUGGER.push({"cannot walljump", {1000,250}, 18, RED});
        }


        if (player_body.lastWallPush == 1) {
            G_DEBUGGER.push({"Carrying residual R. Wall Push", {1000,270}, 18, BLUE});
        } else {
            G_DEBUGGER.push({"Carrying residual L. Wall Push", {1000,270}, 18, BLUE});
        }
        


        G_DEBUGGER.push({"WJ Buffer: " + std::to_string(player_body.walljumpBuffer), {1000, 290}, 18, ORANGE});
        G_DEBUGGER.push({"WJ Window: " + std::to_string(player_body.walljumpWindow), {1000, 310}, 18, ORANGE});


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












