// DebugStats.h

#ifndef DEBUGSTATS_H
#define DEBUGSTATS_H

#include <string>
#include <vector>

struct FloatPort {
    std::string label;
    float * data;
    float x;
    float y;
    int rounding;
};


struct DebugStats {

    

    float frame_deltatime;
    float frame_accumulator;
    int frame_simulation_ticks;

    std::vector<FloatPort> float_ports;

    void set_frame_stats (float f_dt, float f_a, int f_s_t) {
        frame_deltatime = f_dt;
        frame_accumulator = f_a;
        frame_simulation_ticks = f_s_t;
    }

    void add_watchport (std::string label, float &_variable, int _round) {

        for (auto port : float_ports) {
            if (port.data == &_variable) {
                return;
            }
        }

        float_ports.push_back({label, &_variable, 25, 40.0f + (35.0f * float_ports.size()), _round});
    }




};










#endif

