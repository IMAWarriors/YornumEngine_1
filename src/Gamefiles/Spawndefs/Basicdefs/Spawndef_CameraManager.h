// Spawndef_CameraManager.h

#ifndef SPAWNDEF_CAMERAMANAGER_H
#define SPAWNDEF_CAMERAMANAGER_H

#include "../../../Engine/ECS/Registry.h"
#include "../../../Gamefiles/Components/Components.h"

namespace spawndef {

    inline Entity SpawnCameraManager (Registry & registry) {

        Entity cameraManager = registry.create_entity();
    
        registry.apply_component<comp::CameraClampManager>(cameraManager, {false, 0.0f, 0.0f, 0.0f, 0.0f, 8.0f, 1.0f});

        return cameraManager;

    }


}




#endif