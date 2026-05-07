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
    Vec2 origin;
    Vec2 direction;
    int layer_id;

    JointFramePosition () {
        origin = {0.0f, 0.0f};
        direction = {32.0f, 0.0f};
        layer_id = 0;
    }


    JointFramePosition (Vec2 orig, Vec2 dir, int lid) {
        origin = orig;
        direction = dir;
        layer_id = lid;
    }
};

struct AnimJointAdjustmentFrame {

    Vec2 origin;
    float rotation;
    int layer_offset;

    AnimJointAdjustmentFrame (Vec2 orig_offset, float rot_offset, int lid_offset) {
        origin = orig_offset;
        rotation = rot_offset;
        layer_offset = lid_offset;
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

        void sync_default_layer_ids() {
            for (int i = 0; i < default_frame.joints.size(); i++) {
                default_frame.joints[i].layer_id = i;
            }
        }

        void wipe_joints () {

        }

        void wipe_active_frame () {

        }
        
        void wipe_animations () {
            // animations.clear();
        }



    private:

    

};



struct KeyAnimFrame {

    std::vector<AnimJointAdjustmentFrame> joints;

    // Constructor to build key anim frame (ex. into animation list) by making same # joints per key frame for an avatar
    KeyAnimFrame (const Avatar& _avatar) {

        for (int i = 0; i < _avatar.default_frame.joints.size(); i++) {
            joints.push_back(AnimJointAdjustmentFrame({0.0f, 0.0f}, 0.0f, 0));
        }

    }

    // Constructor to take in a previous frame and construct joints that match that frame,
    // I guess its just a copy constructor

    KeyAnimFrame (const KeyAnimFrame & frame) {

        for (int i = 0; i < frame.joints.size(); i++) {

            joints.push_back(AnimJointAdjustmentFrame(frame.joints[i].origin, frame.joints[i].rotation, frame.joints[i].layer_offset));

        }


    }

    // Constructor for the AnimJoints which are all relative to the default joint
    KeyAnimFrame (int joint_count) {
        for (int i = 0; i < joint_count; i++) {
            joints.push_back(AnimJointAdjustmentFrame({0.0f, 0.0f}, 0.0f, 0));
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

    void new_frame (const KeyAnimFrame & _frame) {

        if (frames.size() > 0)
            assert(joints_defined == _frame.joints.size());
        else
            joints_defined = _frame.joints.size();

        frames.push_back(_frame);
    }

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


#endif