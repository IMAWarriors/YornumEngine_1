// Animation.h

#ifndef ANIMATION_H
#define ANIMATION_H

#include "Avatar.h"

struct Animation {

    std::string name;

    int joints_defined = 0; // Joints per frame
    int ms_per_tick_frame = 18;

    std::vector<KeyAnimFrame> frames;


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



    bool SaveAnimFile (const std::string& filename);

    bool LoadAnimFile (const std::string& filename);

};


#endif

