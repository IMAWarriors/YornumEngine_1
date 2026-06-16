// AttackManagerSystem.cpp

#include "AttackManagerSystem.h"



void AttackManagerSystem::update (Registry & registry, float deltatime) {

    std::vector<Entity> attacks_to_destroy;

    for (Entity entity : registry.view<comp::Transform, comp::AttackStats, tag::AttackDataNode>()) {

        comp::AttackStats& attack_stats = registry.get_component<comp::AttackStats>(entity);
        comp::BodyAttackState& attack_state = registry.get_component<comp::BodyAttackState>(attack_stats.owner);

        if (attack_stats.delete_flag) {
            attacks_to_destroy.push_back(entity);
            continue;
        }

        // To Tick
        if (attack_stats.attack_frame < attack_stats.ilk_id.frame_count) {
            attack_stats.tick_one_frame(attack_state, deltatime);
        }

        // Check again if should be destroyed
        if (attack_stats.delete_flag)
            attacks_to_destroy.push_back(entity);
    
    }

    for (Entity entity : attacks_to_destroy) {
        if (registry.is_valid(entity)) {
            registry.destroy_entity(entity);
        }
    }
    

}