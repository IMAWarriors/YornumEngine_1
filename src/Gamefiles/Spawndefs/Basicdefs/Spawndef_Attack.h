// Spawndef_Attack.h

#ifndef SPAWNDEF_ATTACK_H
#define SPAWNDEF_ATTACK_H

#include "../../../Engine/ECS/Registry.h"
#include "../../../Gamefiles/Components/Components.h"

#include "../../../Gamefiles/Assets/AssetManager.h"


namespace spawndef {

    inline Entity SpawnAttack (Registry & registry, Vec2 offset, const hbpos::Attack& type, const Entity& owner, bool mirror = true) {

        Entity attack = registry.create_entity();

        registry.apply_component<tag::AttackDataNode>(attack, {}); 
        registry.apply_component<comp::Transform>(attack, comp::Transform(offset));
        registry.apply_component<comp::AttackStats>(attack, comp::AttackStats(owner, type, mirror));
        
        auto& owner_attack_state = registry.get_component<comp::BodyAttackState>(owner);
       
        // registry.apply_component<comp::Velocity>        (camera, {0.0f, 0.0f} );
        // registry.apply_component<comp::InputState>      (camera, {0, false});



        return attack;

    }



}

#endif