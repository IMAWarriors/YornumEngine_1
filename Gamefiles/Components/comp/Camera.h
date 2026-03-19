// Camera.h

#ifndef COMP_CAMERA_H
#define COMP_CAMERA_H

#include "../../../Engine/ECS/Entity.h"

namespace comp {

    struct Camera {

        Entity target;
        bool isFrozen;

        Vec2 offset;
        float followSmoothing;

    };





}


#endif
