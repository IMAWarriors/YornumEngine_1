// PlayerConfig.h

#ifndef PLAYERCONFIG_H
#define PLAYERCONFIG_H

#include "../../../Engine/Core/Overhead/GameTypes.h"

namespace comp {

struct PlayerConfig {

    float NatRunSpeed      = 650.0f;
    float NatRunAccel      = 1500.0f;
    float NatRunFriction   = 2700.0f;
    float NatJumpForce     = 1150.0f;

};

}


#endif