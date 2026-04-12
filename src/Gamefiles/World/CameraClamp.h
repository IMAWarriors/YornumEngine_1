// CameraClamp.h

#include "../../Engine/Core/Overhead/GameTypes.h"

#ifndef CAMERACLAMP_H
#define CAMERACLAMP_H

struct CameraClamp {

    Vec2 clamp_top_left;
    Vec2 clamp_bottom_right;

    Vec2 player_zone_top_left;
    Vec2 player_zone_bottom_right;

    float smoothing_override;
    bool snap_to_clamp;

};


#endif