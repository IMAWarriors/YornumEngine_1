// HurtboxHandler.h

#ifndef HURTBOXHANDLER_H
#define HURTBOXHANDLER_H

#include "../../../Engine/Core/Overhead/GameTypes.h"
#include "../../../Engine/Core/Overhead/Allignment.h"

namespace comp {


struct HurtboxHandler {

    Vec2 size;
    Vec2 offset;

    int iframes = 0;
    int std_hit_iframes = 16;
    float iframe_speed = 25.0f; // ms
    float accumulator = 0.0f;
    

    HurtboxHandler () {
        size = {20.0f, 40.0f};
        offset = {15.0f, 0.0f};
    }


    // HANDLE DIFFERENT KINDS OF HITS, the actual TAKING A HIT PART not the dealing with whether it counts
    // as a hit--if this function is called, a hit should fucking take place
    void TakeDamageLowGroundHazardThreat (int damage_points = 0, Vec2 knockback_magnitude = {0.0f, 0.0f}, int take_iframes = -99) {

        // Handle iframes
        if (take_iframes < 0)
            iframes = std_hit_iframes;

        if (take_iframes >= 0)
            iframes = take_iframes;


        // Handle knockback



        // Handle damage


        

    }


    

};



}

#endif