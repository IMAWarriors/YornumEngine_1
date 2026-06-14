// AttackManagerSystem.cpp

#include "AttackManagerSystem.h"



void AttackManagerSystem::update (Registry & registry, float deltatime) {

    for (Entity entity : registry.view<comp::Transform, comp::AttackStats, tag::AttackDataNode>()) {

        comp::AttackStats& attack_stats = registry.get_component<comp::AttackStats>(entity);
        comp::Transform& position = registry.get_component<comp::Transform>(entity);

        

        if (attack_stats.delete_flag) {
            registry.destroy_entity(entity);
            continue;
        }

        // To Tick
        if (attack_stats.attack_frame < attack_stats.ilk_id.frame_count) {
            attack_stats.tick_one_frame(deltatime);
        }

        

    }

}