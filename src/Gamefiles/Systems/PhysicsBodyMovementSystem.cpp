#include "PhysicsBodyMovementSystem.h"

#include <algorithm>
#include <cmath>

void PhysicsBodyMovementSystem::update (Registry & registry, float deltatime) {

    for (Entity entity : registry.view<comp::Transform, comp::Velocity, comp::PhysicsBody>()) {

        comp::Transform & transform = registry.get_component<comp::Transform>(entity);
        comp::Velocity & velocity = registry.get_component<comp::Velocity>(entity);
        comp::PhysicsBody & body = registry.get_component<comp::PhysicsBody>(entity);

    
        transform.previous_position = transform.position;
        float tile_size = gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS;

        
        transform.position.x = transform.position.x + (velocity.magnitude.x * deltatime);
        transform.position.y = transform.position.y + (velocity.magnitude.y * deltatime);
        
    }
}