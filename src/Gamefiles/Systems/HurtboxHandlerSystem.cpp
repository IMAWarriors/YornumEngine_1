// HurtboxHandlerSystem.cpp

#include "HurtboxHandlerSystem.h"


void HurtboxHandlerSystem::update(Registry& registry, float deltatime) {

    for (Entity entity : registry.view<comp::Transform, comp::Velocity, comp::HurtboxHandler, comp::AgentStats>()) {

        auto& hurtbox = registry.get_component<comp::HurtboxHandler>(entity);
        auto& stats   = registry.get_component<comp::AgentStats>(entity);

        

    }

}