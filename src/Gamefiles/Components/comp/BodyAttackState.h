// BodyAttackState.h

#ifndef BODYATTACKSTATE_H
#define BODYATTACKSTATE_H

#include <algorithm>
#include <cmath>
#include <vector>
#include <cassert>

#include "../../../Engine/ECS/Registry.h"
#include "../../../Engine/ECS/Entity.h"
#include "../../../Engine/Core/Overhead/GameTypes.h"

#include "../../Components/comp/AttackStats.h"

namespace comp {

struct BodyAttackState {

    Entity intended_target;
    std::vector<Entity> entities_hit;

    bool hurt_player = false;
    bool hurt_enemies = true;
    bool hurt_civilians = true;
    bool hurt_others = true;

    bool attacking = false;
    int frames_left = 0;

    // Default Constructor
    BodyAttackState () {


    }

    // Make an attack
    void BeginAttack (const hbpos::Attack& attack) {

        // Attack Allignment
        hurt_player = false;
        hurt_enemies = true;
        hurt_civilians = true;
        hurt_others = true;

        // Attack timing setup
        attacking = true;
        frames_left = attack.frame_count;
        entities_hit.clear();

    }

    void AdvanceAttackFrames (int frames_advanced) {
        frames_left = std::max(0, frames_left - frames_advanced);

        if (frames_left <= 0) {
            attacking = false;
        }
    }

    void FinishAttack () {
        attacking = false;
        frames_left = 0;
    }


    
    
};

}

#endif