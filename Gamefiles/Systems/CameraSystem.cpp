// CameraSystem.cpp

#include "CameraSystem.h"
#include "../../Engine/Core/Rendering/Renderer.h"

// DELETE THIS EVENTUALLY WHEN WE ADD NORMAL ZOOMING, TEMPORARY ZOOM CONTROLS
#include "raylib.h"
// ====================================================================

void CameraSystem::update (Registry & registry, float deltatime) {
    

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

        if (!camera.isFrozen) {
            if (target_exists) {

                float smoothing = camera.followSmoothing;

                float factor = 1.0f - exp(-smoothing * deltatime);
                transform.previous_position = transform.position;
                camera.previous_zoom = camera.zoom;

                transform.position.x += (target_position.x - transform.position.x) * factor;
                transform.position.y += (target_position.y - transform.position.y) * factor;
                
            }

            if (IsKeyDown(KEY_LEFT_CONTROL)) {
                camera.zoom += 0.05f * deltatime;
                if (camera.zoom > camera.maxZoom) {
                    camera.zoom = camera.maxZoom;
                }
            }

            if (IsKeyDown(KEY_LEFT_SHIFT)) {
                camera.zoom -= 0.05f * deltatime;
                if (camera.zoom < camera.minZoom) {
                    camera.zoom = camera.minZoom;
                }
            }   

            

        }

        

        //

    }
}