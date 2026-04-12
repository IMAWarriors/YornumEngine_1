// CameraSystem.cpp

#include "CameraSystem.h"
#include "../../Engine/Core/Rendering/Renderer.h"

// DELETE THIS EVENTUALLY WHEN WE ADD NORMAL ZOOMING, TEMPORARY ZOOM CONTROLS
#include "raylib.h"
// ====================================================================

void CameraSystem::update (Registry & registry, float deltatime) {





    Vec2 camera_clamp_min = {0.0f, 0.0f};
    Vec2 camera_clamp_max = {0.0f, 0.0f};

    bool camera_clamp_active = false;

    float override_smoothing = 1.0f;

    for (Entity manager : registry.view<comp::CameraClampManager>()) {

        if (registry.get_component<comp::CameraClampManager>(manager).isThereActiveClamping) {
            camera_clamp_active = true;
            camera_clamp_min = {registry.get_component<comp::CameraClampManager>(manager).camera_x_min, registry.get_component<comp::CameraClampManager>(manager).camera_y_min};
            camera_clamp_max = {registry.get_component<comp::CameraClampManager>(manager).camera_x_max, registry.get_component<comp::CameraClampManager>(manager).camera_y_max};
            override_smoothing = registry.get_component<comp::CameraClampManager>(manager).camera_smoothing_override;
        }


    }
    

    

    for (Entity entity : registry.view<comp::Camera>()) {      // For each iteration of Entity

        comp::Camera & camera = registry.get_component<comp::Camera>(entity);
        comp::Transform & transform = registry.get_component<comp::Transform>(entity);
        
        // Move camera i guess

        Vec2 target_position = {0.0f, 0.0f};    
        bool target_exists = false;

        if (registry.has_component<comp::Transform>(camera.target)) {

            Vec2 camera_position = registry.get_component<comp::Transform>(camera.target).position;
            target_position = {camera_position.x + camera.offset.x, camera_position.y + camera.offset.y};
            target_exists = true;

        }


        // APPLY CLAMPING TO THE TARGET POSITION IF IT APPLY

        if (!IsKeyDown(KEY_TAB)) {

            if (camera_clamp_active) {

                if (target_position.x > camera_clamp_max.x) { target_position.x = camera_clamp_max.x; }
                if (target_position.x < camera_clamp_min.x) { target_position.x = camera_clamp_min.x; }
                if (target_position.y > camera_clamp_max.y) { target_position.y = camera_clamp_max.y; }
                if (target_position.y < camera_clamp_min.y) { target_position.y = camera_clamp_min.y; }



            }

        }






        if (!camera.isFrozen) {
            if (target_exists) {

                // ======================= ACTUAL MOVEMENT LOGIC ===============================
                
                transform.previous_position = transform.position;
                camera.previous_zoom = camera.zoom;

                Vec2 offset = {(target_position.x - transform.position.x), (target_position.y - transform.position.y)};
                Vec2 dist = {fabs(offset.x), fabs(offset.y)};

                float base_smoothing = camera.followSmoothing;
                float gain = 0.015f;
                float peak = 4.0f;

                Vec2 smoothing = {
                    (1.0f + std::min(dist.x * gain, peak)) * base_smoothing,
                    (1.0f + std::min(dist.y * gain, peak)) * base_smoothing
                };


                if (camera_clamp_active) {
                    if (camera_clamp_min.x < transform.position.x && transform.position.x < camera_clamp_max.x &&
                        camera_clamp_min.y < transform.position.y && transform.position.y < camera_clamp_max.y) {

                            smoothing = {override_smoothing, override_smoothing};

                    }
                }



                Vec2 factor = {
                    1.0f - exp(-smoothing.x * deltatime),
                    1.0f - exp(-smoothing.y * deltatime)
                };


                transform.position.x += offset.x * factor.x;
                transform.position.y += offset.y * factor.y;

                // ==============================================================================
                
            }

            if (IsKeyDown(KEY_LEFT_CONTROL)) {
                camera.zoom += 2.0f * deltatime;
                if (camera.zoom > camera.maxZoom) {
                    camera.zoom = camera.maxZoom;
                }
            }

            if (IsKeyDown(KEY_LEFT_SHIFT)) {
                camera.zoom -= 2.0f * deltatime;
                if (camera.zoom < camera.minZoom) {
                    camera.zoom = camera.minZoom;
                }
            }  
        }
    }
}