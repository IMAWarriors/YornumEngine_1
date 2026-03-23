// DebugStats.h

#ifndef DEBUGSTATS_H
#define DEBUGSTATS_H

#include "GameTypes.h"

#include <string>
#include <vector>

struct FloatPort {
    std::string label;
    float * data;
    float x;
    float y;
    int rounding;
    int id; 
};


struct DebugStats {

    Vec2 position = {25.0f, 25.0f};

    float frame_deltatime;
    float frame_accumulator;
    int frame_simulation_ticks;

    float frames_per_second;

    int next_id = 0;

    std::vector<FloatPort> float_ports;

    void set_frame_stats (float f_dt, float f_a, int f_s_t, float fps) {
        frame_deltatime = f_dt;
        frame_accumulator = f_a;
        frame_simulation_ticks = f_s_t;
        frames_per_second = fps;
    }

    void set_watchport_list_position (Vec2 newpos) {
        position = newpos;
        for (FloatPort & float_port : float_ports) {
            float_port.x = position.x;
            float_port.y = position.y + (15.0f * (float)(float_port.id));
        }
    }

    void add_watchport (std::string label, float &_variable, int _round) {

        for (auto port : float_ports) {
            if (port.data == &_variable) {
                return;
            }
        }

        float_ports.push_back({label, &_variable, position.x, position.y + (15.0f * (float)next_id), _round, next_id});
        next_id++;
    }




};

#endif

