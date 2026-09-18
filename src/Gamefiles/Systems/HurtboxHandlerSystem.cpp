// HurtboxHandlerSystem.cpp

#include "HurtboxHandlerSystem.h"


void HurtboxHandlerSystem::update(Registry& registry, float deltatime) {

    for (Entity entity : registry.view<comp::Transform, comp::PhysicsBody, comp::Velocity, comp::HurtboxHandler, comp::AgentStats>()) {

        auto& transform = registry.get_component<comp::Transform>(entity);
        auto& hurtbox = registry.get_component<comp::HurtboxHandler>(entity);
        auto& stats   = registry.get_component<comp::AgentStats>(entity);
        auto& body    = registry.get_component<comp::PhysicsBody>(entity);
        auto& velocity= registry.get_component<comp::Velocity>(entity);


        // Update invinciblity frames per game update tick
        if (stats.iframes > 0) {
            stats.iframes--;
        }

        Vec2 top_left;
        Vec2 bottom_right;

        top_left.x = transform.position.x - (body.size.x / 2.0f);
        top_left.y = transform.position.y - (body.size.y / 2.0f);
        bottom_right.x = transform.position.x + (body.size.x / 2.0f);
        bottom_right.y = transform.position.y + (body.size.y / 2.0f);
            
        Vec2 ffdetect_top_left;
        Vec2 ffdetect_bot_right;

        ffdetect_top_left.x = top_left.x;
        ffdetect_bot_right.x = top_left.x + body.size.x;
        ffdetect_top_left.y = (bottom_right.y) - hurtbox.foot_dcheck;
        ffdetect_bot_right.y = (bottom_right.y) + hurtbox.foot_dcheck;

        /*enum class CollisionType : uint8_t {
            COLL_EMPTY = 0,
            COLL_FULL_SOLID,
            COLL_PSLOPE1_SOLID,
            COLL_NSLOPE1_SOLID,
            COLL_FULL_SEMISOLID,
            COLL_GROUND_HAZARD
        };*/

        bool touchingGroundHazard = scene.is_rect_groundhazard(ffdetect_top_left, {body.size.x , (hurtbox.foot_dcheck * 2.0f)});

        if (touchingGroundHazard) {            
            
            if (stats.iframes <= 0) {
                stats.iframes = 58;
            }

            if (velocity.magnitude.y < 1.0f) {
                velocity.magnitude.x = -800.0f * body.direction;
                velocity.magnitude.y = -1500.0f;
                body.falling = 999;
            }
        }












        

    }

}