// Avatar.h

#ifndef AVATAR_H
#define AVATAR_H

#include "../../../Engine/Core/Overhead/GameTypes.h"
#include "../../Assets/AssetManager.h"

#include <string>
#include <vector>
#include <cassert>

// The actual position and rotation of a joint for a frame

struct JointFramePosition {

    int unique_id;

    Vec2 origin;
    Vec2 direction;

    JointFramePosition () {
        origin = {0.0f, 0.0f};
        direction = {32.0f, 0.0f};
    }


    JointFramePosition (Vec2 orig, Vec2 dir) {
        origin = orig;
        direction = dir;
    }

};

struct AnimJointAdjustmentFrame {

    int unique_id;

    Vec2 origin;
    float rotation;
    int draw_order;

    bool normal_rotation = true;

    AnimJointAdjustmentFrame (Vec2 orig_offset, float rot_offset, int d_order) {
        origin = orig_offset;
        rotation = rot_offset;
        draw_order = d_order;
    }

    void SetRotMode_Normal () {
        normal_rotation = true;
    }
    
    void SetRotMode_Inverse () {
        normal_rotation = false;
    }
};

// The texturing info for a specific body part
// --> Armor, Shirts, Expressions on their faces, etc.

struct AvatarJoint {

    // Name of joint
    std::string name;

    // Texturing info for joint
    std::string texturePath;
    Texture2D * texture = nullptr;
    Vec2 offset = {0, 0};     // position relative to joint
    Vec2 scale = {1, 1};      // size multiplier
    float rotation = 0.0f;    // rotation in radians
    Vec2 crop_min = {0, 0};   // UV min (0–1)
    Vec2 crop_max = {1, 1};   // UV max (0–1)

    // Loading functions
    bool joint_has_texture () { return (texture != nullptr);    }

    bool unload_texture (AssetManager & assets) {
        assets.UnloadTextureAsset(texture);
        texture = nullptr;
        return true;
    }
    
    bool load_texture_from_path (AssetManager & assets, const std::string & path) {
        if (joint_has_texture()) { unload_texture(assets); }
        texture = &assets.LoadTextureAsset(path);
        texturePath = path; 
        return true;
    }

    
    AvatarJoint (const std::string & _name) {
        this->name = _name;


    }

};

// Every avatar should have
//      - 1. active AvatarTextureRig
//      - 2. active KeyFrame
//      - 3. active

struct AvatarTextureRig {
    std::vector<AvatarJoint> joints;



};

struct KeyFrame {
    std::vector<JointFramePosition> joints;

    KeyFrame () {
        joints.clear();
    }

    KeyFrame (int size) {
        for (int i = 0; i < size; i++) {
            joints.push_back({});
        }
    }
};


class Animation;


class Avatar {
    public:
        std::string name;

        // Rendering position of Avatar
        Vec2 position;

        // Default texturing that is exhibited, should be what is edited in the editor
        AvatarTextureRig default_texturing;
        KeyFrame default_frame;

        // The active texturing options that are chosen
        AvatarTextureRig active_texturing;
        KeyFrame active_frame;

        Avatar (std::string _name) {
            this->name = _name;
        }

        Avatar () {
            position = {0.0f, 0.0f};
        }


        
        void assign_unique_anchor_ids () {

            // Arbitrary ordering, simply for use of reference
            // Should be updated whenever updating the default frames joints

            for (int i = 0; i < default_frame.joints.size(); i++) {
                default_frame.joints[i].unique_id = i;
            }

            // layer_id order (or draw order) should be set to be the same as unique id
            // order on the default/anchor frame, but can be varied, and
            // transitions between joints in animation are only worried about the same unique id's existing.
            //
            // animation application should only worry about this as well


        }


        // A tick is what could often be called a frame of animation,
        // but the problem is frames are constant measured in ms
        // --> More common use of frames in my code is in reference to KEY FRAMES which happen at
        // arbitrary user defined times, but always occur on some tick

        void IM_DrawAvatar (Vec2 position, float scale, const Animation& animation, int tick_ms);

        void DrawAvatar (Vec2 position, float scale, const Animation& animation, int tick_ms);




    private:

    

};



struct KeyAnimFrame {

    enum class TransitionMode {
        Linear = 0,
        Instant = 1,
        EaseInOut = 2
    };

    std::vector<AnimJointAdjustmentFrame> joints;

    int sequence_id = 0;
    int time_to_next = 8;
   
    TransitionMode transition_mode = TransitionMode::Linear;

    // Constructor to build key anim frame from Avatar (ex. into animation list) by making same # joints per key frame for an avatar
    KeyAnimFrame (const Avatar& _avatar) {

        for (int i = 0; i < _avatar.default_frame.joints.size(); i++) {

            int jidx = 0;
            while (jidx != _avatar.default_frame.joints[i].unique_id || jidx > _avatar.default_frame.joints.size()) {
                jidx++;
            }

            // assert the joint id we found is valid within the avatar bounds
            assert (jidx < _avatar.default_frame.joints.size());

            // Default draw order should for a key animation frame
            joints.push_back(AnimJointAdjustmentFrame({0.0f, 0.0f}, 0.0f, jidx));
        }

    }

    


};



struct Animation {

    std::string name;
    int joints_defined = 0; // Joints per frame
    std::vector<KeyAnimFrame> frames;


    Animation (std::string _name) {
        name = _name;
        joints_defined = 0;
        frames.clear();
    }

    void sync_frame_order_seq_id () {

        int i = 0;
        for (KeyAnimFrame & frame : frames) {
            frame.sequence_id = i;
            i++;
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

    // ============== DANGER CODE ==========================================
    // This code is scary bc i dont remember waht it does...
    // I dont think it does anything, so I think I need to delete it and rework the idea it was getting at

    void resize_joints_defined (const Avatar & _avatar) {

        // If new size is a clipping of all joints there in the past,
        if (_avatar.default_frame.joints.size() < joints_defined) {

            for (int c = 0; c < frames.size(); c++) {

                for (int i = 0; i < _avatar.default_frame.joints.size(); i++) {

                    int to_delete = _avatar.default_frame.joints.size() - joints_defined;

                    for (int j = 0; j < frames[c].joints.size(); j++) {
                        if (to_delete > 0) {
                            frames.erase(frames.begin() + j);
                            to_delete--;
                        }
                    }
                }

                
            }

            joints_defined = _avatar.default_frame.joints.size();

            
        } else {

            // Otherwise is new size is just an expansion, add the additional joints to each frame

        }
    }

};

// =============================================================
// Avatar Interpolation Drawing Definitions:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


#endif


