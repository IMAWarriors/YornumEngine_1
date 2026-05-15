// Animation.cpp

#include "Animation.h"

#include <fstream>
#include <sstream>

bool Animation::SaveAnimFile (const std::string& filename, const std::string& path) {

    std::string fullpath = path + filename + ".anim";
    std::ofstream file(fullpath);

    if (!file.is_open())
        return false;

    // --> Parent Animation File Information
    // ========================================
    // name                 std::string
    // joints_defined       int
    // ms_per_tick_frame    int
    // frames               vector of KeyAnimFrame

    // [Substruct] --> KeyAnimFrame (each):
    // sequence_id          int
    // time_to_next         int
    // start_tick_frame     int
    // transition_mode      enum (int)
    // joints               AnimJointAdjustmentFrame

    // [Substruct] --> AnimJointAdjustmentFrame (each):
    // unique_id            int             e.x. -> [   0 1.35 9.35 120.3 0 1
    // origin.x             float
    // origin.y             float
    // rotation             float
    // draw_order           int
    // normal_rotation      bool (int)



}

bool Animation::LoadAnimFile (const std::string& filename, const std::string& path) {


    
}