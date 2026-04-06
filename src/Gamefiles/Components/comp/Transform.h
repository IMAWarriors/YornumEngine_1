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
        
    };

}

#endif
