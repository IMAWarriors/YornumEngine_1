// Animation.cpp

#include "Animation.h"

#include <fstream>
#include <sstream>
#include <limits>

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
    // unique_id            int           
    // origin.x             float
    // origin.y             float
    // rotation             float
    // draw_order           int
    // normal_rotation      bool (int)

    const std::string version = "VERSION_2";

    file << "~ANIMATION_FILE" << '\n';
    file << version << '\n';

    file << "~ANIMATION_NAME" << '\n';
    file << name << '\n';

    file << "~ANIMATION_META" << '\n';
    file << joints_defined << " " << ms_per_tick_frame << " " << frames.size() << '\n';

    file << "~KEYFRAMES" << '\n';

    for (const KeyAnimFrame& frame : frames) {

        file << frame.sequence_id << " "
             << frame.time_to_next << " "
             << frame.start_tick_frame << " "
             << static_cast<int>(frame.transition_mode) << " "
             << frame.joints.size() << '\n';

        for (const AnimJointAdjustmentFrame& joint : frame.joints) {
            file << joint.unique_id << " "
                 << joint.origin.x << " "
                 << joint.origin.y << " "
                 << joint.rotation << " "
                 << joint.draw_order << " "
                 << static_cast<int>(joint.normal_rotation) << " "
                 << joint.anim_texture_idx << '\n';
        }
    }

    file << "~END_OF_FILE" << '\n';

    return true;

}

bool Animation::LoadAnimFile (const std::string& filename, const std::string& path) {

    std::string fullpath = path + filename + ".anim";
    std::ifstream file(fullpath);

    if (!file.is_open())
        return false;

    std::string token;

    file >> token;
    if (token != "~ANIMATION_FILE")
        return false;

    std::string version;
    file >> version;
    const bool version_1 = (version == "VERSION_1");
    const bool version_2 = (version == "VERSION_2");
    if (!version_1 && !version_2)
        return false;

    file >> token;
    if (token != "~ANIMATION_NAME")
        return false;

    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(file, name, '\n');

    file >> token;
    if (token != "~ANIMATION_META")
        return false;


    int loaded_joints_defined = 0;
    int loaded_ms_per_tick_frame = 18;
    int loaded_frame_count = 0;

    file >> loaded_joints_defined >> loaded_ms_per_tick_frame >> loaded_frame_count;
    if (loaded_joints_defined < 0 || loaded_ms_per_tick_frame <= 0 || loaded_frame_count < 0)
        return false;

    file >> token;
    if (token != "~KEYFRAMES")
        return false;

    std::vector<KeyAnimFrame> loaded_frames;
    loaded_frames.reserve(loaded_frame_count);

    for (int i = 0; i < loaded_frame_count; i++) {
        KeyAnimFrame frame;
        int transition_mode_int = 0;
        int loaded_joint_count = 0;

        file >> frame.sequence_id
             >> frame.time_to_next
             >> frame.start_tick_frame
             >> transition_mode_int
             >> loaded_joint_count;

        if (loaded_joint_count < 0 || frame.time_to_next < 0)
            return false;

        switch (transition_mode_int) {
            case 0: frame.transition_mode = KeyAnimFrame::TransitionMode::Linear; break;
            case 1: frame.transition_mode = KeyAnimFrame::TransitionMode::Instant; break;
            case 2: frame.transition_mode = KeyAnimFrame::TransitionMode::EaseInOut; break;
            default: return false;
        }

        frame.joints.reserve(loaded_joint_count);
        for (int j = 0; j < loaded_joint_count; j++) {
            AnimJointAdjustmentFrame joint({0.0f, 0.0f}, 0.0f, 0);
            int normal_rotation_int = 1;

            file >> joint.unique_id
                 >> joint.origin.x
                 >> joint.origin.y
                 >> joint.rotation
                 >> joint.draw_order
                 >> normal_rotation_int;

            if (version_2) {
                file >> joint.anim_texture_idx;
            } else {
                joint.anim_texture_idx = -1;
            }

            joint.normal_rotation = (normal_rotation_int != 0);
            frame.joints.push_back(joint);
        }

        loaded_frames.push_back(frame);
    }

    file >> token;
    if (token != "~END_OF_FILE")
        return false;

    frames = loaded_frames;
    joints_defined = loaded_joints_defined;
    ms_per_tick_frame = loaded_ms_per_tick_frame;
    sync_frame_order_seq_id();

    if (!frames.empty() && joints_defined == 0)
        joints_defined = frames[0].joints.size();

    for (const KeyAnimFrame& frame : frames) {
        if ((int)frame.joints.size() != joints_defined)
            return false;
    }

    return true;
    
}