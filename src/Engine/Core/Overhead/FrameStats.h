// FrameStats.h

#ifndef FRAMESTATS_H
#define FRAMESTATS_H

#include "GameTypes.h"

struct FrameStats {
    float deltatime;
    float accumulator;
    int simulation_ticks;
    float fps;

    void update(float dt,float a,int s,float fps){
        deltatime=dt;
        accumulator=a;
        simulation_ticks=s;
        this->fps=fps;
    }
};

#endif

