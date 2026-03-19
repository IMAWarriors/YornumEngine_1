// CameraSystem.cpp

#include "CameraSystem.h"
#include "../../Engine/Core/Rendering/Renderer.h"

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

        if (target_exists) {

            float smoothing = camera.followSmoothing;

            float factor = 1.0f - exp(-smoothing * deltatime);
            transform.previous_position = transform.position;

            transform.position.x += (target_position.x - transform.position.x) * factor;
            transform.position.y += (target_position.y - transform.position.y) * factor;
            
        }

        

        //

    }
}