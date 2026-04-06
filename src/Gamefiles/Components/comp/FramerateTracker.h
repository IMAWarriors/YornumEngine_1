// FramerateTracker.h

#ifndef FRAMERATETRACKER_H
#define FRAMERATETRACKER_H

#include "../../../Engine/Core/Overhead/GameTypes.h"

namespace comp {

    struct FramerateTracker {

        float accumulator = 0.0f;   // Time bucket
        float frame_deltatime = 0.0f;
        float frames_per_second = 0.0f;
        float frame_simulation_ticks = 0.0f;


    };

}

#endif

