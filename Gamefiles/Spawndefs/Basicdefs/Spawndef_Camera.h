// Spawndef_Camera.h

#ifndef SPAWNDEF_CAMERA_H
#define SPAWNDEF_CAMERA_H

#include "../../../Engine/ECS/Registry.h"
#include "../../../Gamefiles/Components/Components.h"


namespace spawndef {

    inline Entity SpawnCamera (Registry & registry,  Entity target) {

        Entity camera = registry.create_entity();

        registry.apply_component<comp::Camera>          (camera, {target, false, {0.0f, 0.0f}});

        registry.apply_component<comp::Transform>       (camera,  { {0.0f, 0.0f}, 0.0f, {1.0f, 1.0f} });
       
        // registry.apply_component<comp::Velocity>        (camera, {0.0f, 0.0f} );
        // registry.apply_component<comp::InputState>      (camera, {0, false});

        return camera;

    }






}




#endif



