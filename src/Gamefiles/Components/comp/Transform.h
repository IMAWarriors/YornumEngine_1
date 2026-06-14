// Transform.h

#ifndef COMP_TRANSFORM_H
#define COMP_TRANSFORM_H

#include "../../../Engine/Core/Overhead/GameTypes.h"

namespace comp {

    struct Transform {

        Vec2  position;
        Vec2  previous_position;
        float rotation;
        Vec2     scale;

        Transform (Vec2 pos) {
            position = pos;
            previous_position = pos;
            rotation = 0.0f;
            scale = {1.0f, 1.0f};
        }
        
    };

}

#endif
