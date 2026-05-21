// Animation.h

#ifndef ANIMATION_H
#define ANIMATION_H

#include "Avatar.h"

struct Animation {

    std::string name;

    int joints_defined = 0; // Joints per frame
    int ms_per_tick_frame = 18;

    float total_tick_frames = -1; // uninitialized
    float last_blend_frame_idx = -1; // uninitialized

    std::vector<KeyAnimFrame> frames;

    Animation () {
        name = "UNTITLED_ANIMATION_DEFAULT_CONSTRUCTOR";
        joints_defined = 0;
        frames.clear();
    }


    Animation (std::string _name) {
        name = _name;
        joints_defined = 0;
        frames.clear();
    }


    void sync_frame_order_seq_id () {

        int i = 0;
        int tick_frame_cursor = 0;
        
        for (KeyAnimFrame & frame : frames) {
            frame.sequence_id = i;
            frame.start_tick_frame = tick_frame_cursor;
            i++;
            tick_frame_cursor += frame.time_to_next + 1;
        }
    }

    void new_frame (const Avatar& _avatar) {

	    KeyAnimFrame frame = KeyAnimFrame(_avatar);

        // Copy Unique IDs so that these will be consistent across joints
        // across the animation and can be used as reference points for sorting
        // where a joint with existing ID should be drawn given an animation frame
        // and a new draw order
        // ---------------------------------------
        for (int i = 0; i < frame.joints.size(); i++) {
            frame.joints[i].unique_id = _avatar.default_frame.joints[i].unique_id;
        }

        
        if (frames.size() > 0)
            assert(joints_defined == frame.joints.size());
        else
            joints_defined = frame.joints.size();

        frames.push_back(frame);

        if (frames.size() > 0)
            frames[frames.size() - 1].sequence_id =  frames.size() - 1;

        sync_frame_order_seq_id();
    }


    // Overload takes a keyframe as an input to... copy it I guess...
    // --> For copying prevoius frames
    void new_frame (const Avatar& _avatar, const KeyAnimFrame & _frame) {

        // Assert compatability between avatar joints and the frame we want to add's joints
        // (in this case, usually a previous frame of the animation that wants to be copied)
        assert(_avatar.default_frame.joints.size() == _frame.joints.size());

        // Remember constructor by default assigns draw order depending on unique id of avatar joints;
        // if we want to copy we have to manually assign 
	    KeyAnimFrame copy = _frame;

        // This draw order should be drawn from the previous frame,
        // but it should be compatible with the original avatar
        // ---------------------------------------
        for (int i = 0; i < copy.joints.size(); i++) {
            copy.joints[i].unique_id = _frame.joints[i].unique_id;
        }
        
        if (frames.size() > 0)
            assert(joints_defined == copy.joints.size());
        else
            joints_defined = copy.joints.size();

        frames.push_back(copy);

        if (frames.size() > 0)
            frames[frames.size() - 1].sequence_id =  frames.size() - 1;

        sync_frame_order_seq_id();
    }

    //  This function gives the SIZE of the total number of tick frames
    //  as if it were a list--keep in mind size is not intervalic;
    //  Index of last valid tick frame for animation is total_tick_frame_count() - 1
    int total_tick_frame_count () {
        if (total_tick_frames < 0) {
            
            // Get the proper length of the animation in ticks
            int animation_tick_frame_length = 0;

            for (const KeyAnimFrame& kframe : frames) {
                animation_tick_frame_length += kframe.time_to_next + 1;
            }

            total_tick_frames = animation_tick_frame_length;

            return total_tick_frames;

        } else {
            return total_tick_frames;
        }
    }

    // Function should be used to blend queued animations by getting
    // the last tick frame index of the last key animation frame of a queued animation
    int last_blend_frame_tick () {

        if (last_blend_frame_idx == -1) {
            // Get the proper length of the animation in ticks
            int len = 0;
            int idx = 0;

            for (const KeyAnimFrame& kframe : frames) {
                // Dont add time to next if we are on the last frame,
                // then preserve that index to get the last frames start index
                if (idx != frames.size()-1) {
                    len += kframe.time_to_next + 1;
                }

                idx++;
            }

            last_blend_frame_idx = len;
            return last_blend_frame_idx;
        } else {
            return last_blend_frame_idx;
        }
    }


    bool SaveAnimFile (const std::string& filename, const std::string& path);

    bool LoadAnimFile (const std::string& filename, const std::string& path);

};


#endif

