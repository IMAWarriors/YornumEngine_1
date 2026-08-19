// AgentStats.h

#ifndef AGENTSTATS_H
#define AGENTSTATS_H

#include "../../../Engine/Core/Overhead/GameTypes.h"
#include "../../../Engine/Core/Overhead/Allignment.h"

namespace comp {

    struct AgentStats {

        // Allignment
        Allignment allignment = Allignment::Neutral;

        // Health
        int health = 30;
        int health_max = 30;
        
        // Aura
        int aura = 10;
        int aura_max = 10;

        // Energy
        int energy = 20;
        int energy_max = 20;
        int energy_timer = 0;
        int energy_timer_peak = 150;




        

    };

}

#endif

