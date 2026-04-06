// Spawndef_EngineManager.h

#ifndef SPAWNDEF_ENGINEMANAGER_H
#define SPAWNDEF_ENGINEMANAGER_H

#include "../../../Engine/ECS/Registry.h"
#include "../../../Gamefiles/Components/Components.h"

namespace spawndef {

    inline Entity SpawnEngineManager (Registry & registry) {

        Entity engineManager = registry.create_entity();
        
        registry.apply_component<comp::MouseTracker>(engineManager, {{0.0f,0.0f},{0.0f,0.0f}});
        registry.apply_component<comp::FramerateTracker>(engineManager, {0.0f,0.0f,0.0f,0.0f});
        registry.apply_component<comp::WorldAnimationState>(engineManager, {0.0f});
        registry.apply_component<tag::EngineManager>(engineManager, {});

        return engineManager;

    }


}




#endif