// DebugStats.h

#ifndef DEBUGSTATS_H
#define DEBUGSTATS_H

struct DebugStats {

    float frame_deltatime;
    float frame_accumulator;
    int frame_simulation_ticks;


    void set_frame_stats (float f_dt, float f_a, int f_s_t) {
        frame_deltatime = f_dt;
        frame_accumulator = f_a;
        frame_simulation_ticks = f_s_t;
    }



};










#endif

