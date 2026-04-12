// CameraClampManagerSystem.cpp

#include "CameraClampManagerSystem.h"

#include "../World/Scene.h"

#include "../World/CameraClamp.h"

void CameraClampManagerSystem::update (Registry & registry, float deltatime) {

    // Get player position

    float player_x;
    float player_y;
    
    for (Entity entity : registry.view<tag::Player>()) {

        auto position = registry.get_component<comp::Transform>(entity).position;
        player_x = position.x;
        player_y = position.y;
    }
    
    

    for (Entity entity : registry.view<comp::CameraClampManager>()) {      // For each iteration of Entity

        comp::CameraClampManager & manager = registry.get_component<comp::CameraClampManager>(entity);

        manager.isThereActiveClamping = false;

        manager.camera_x_min = 0;
        manager.camera_y_min = 0;
        manager.camera_x_max = 0;
        manager.camera_y_max = 0;

        for (CameraClamp clamp : scene.active_clamps) {

            // We just look for the first possible clamp that fits the criteria bro don't efver make overlapping camera clamps
            // cuz idk waht will happen

            if (clamp.player_zone_top_left.x < player_x && player_x < clamp.player_zone_bottom_right.x &&
                clamp.player_zone_top_left.y < player_y && player_y < clamp.player_zone_bottom_right.y) {

                    manager.camera_x_min = clamp.clamp_top_left.x;
                    manager.camera_y_min = clamp.clamp_top_left.y;
                    manager.camera_x_max = clamp.clamp_bottom_right.x;
                    manager.camera_y_max = clamp.clamp_bottom_right.y;

                    manager.isThereActiveClamping = true;
                
            }



        }
        



    }


}