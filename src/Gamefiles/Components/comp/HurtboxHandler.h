// HurtboxHandler.h

#ifndef HURTBOXHANDLER_H
#define HURTBOXHANDLER_H

#include "../../../Engine/Core/Overhead/GameTypes.h"
#include "../../../Engine/Core/Overhead/Allignment.h"

namespace comp {


struct HurtboxHandler {

    Vec2 size;
    Vec2 offset;

    // Foot detector height
    float foot_dcheck = 10.0f;
    

    HurtboxHandler () {
        size = {20.0f, 40.0f};
        offset = {15.0f, 0.0f};
    }


    


    

};



}

#endif