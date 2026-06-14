// Avatar.h

#ifndef AVATAR_H
#define AVATAR_H

#include "../../../Engine/Core/Overhead/GameTypes.h"
#include "../../../Engine/Core/Rendering/Renderer.h"

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

    int anim_texture_idx = -1;

    AnimJointAdjustmentFrame () {
        unique_id = 0;
        origin = {0.0f, 0.0f};
        rotation = 0.0f;
        draw_order = 0;
        anim_texture_idx = -1;  // Anchor frame texture == -1
    }

    AnimJointAdjustmentFrame (Vec2 orig_offset, float rot_offset, int d_order) {
        origin = orig_offset;
        rotation = rot_offset;
        draw_order = d_order;
        anim_texture_idx = -1;
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

class AssetManager;

struct AvatarJoint;

struct ResolvedJointTexture {
    Texture2D* texture = nullptr;
    Vec2 offset = {0, 0};
    Vec2 scale = {1, 1};
    float rotation = 0.0f;
    Vec2 crop_min = {0, 0};
    Vec2 crop_max = {1, 1};
};

struct JointTexture {

    // ==================== Attributes ===================== //

    // Image internal and pointer
    std::string texture_path = "DEFAULT_CONSTRUCTOR_TEXTURE_PATH";
    Texture2D* texture_ptr = nullptr;

    // Adjustments
    Vec2 offset = {0, 0};     // position relative to joint
    Vec2 scale = {1, 1};      // size multiplier
    float rotation = 0.0f;    // rotation in radians
    Vec2 crop_min = {0, 0};   // UV min (0–1)
    Vec2 crop_max = {1, 1};   // UV max (0–1)

    // ==================== Methods ===================== //

    // Default constructor
    JointTexture () {
        texture_path = "DEFAULT_CONSTRUCTOR_TEXTURE_PATH";
        texture_ptr = nullptr;
        offset = {0, 0};     // position relative to joint
        scale = {1, 1};      // size multiplier
        rotation = 0.0f;    // rotation in radians
        crop_min = {0, 0};   // UV min (0–1)
        crop_max = {1, 1};   // UV max (0–1)
    }

    // Selective constructor
    JointTexture (AssetManager& assets, const std::string& path, Vec2 tex_offset = {0.0f, 0.0f}, Vec2 tex_scale = {1.0f, 1.0f}, float tex_rot = 0.0f, Vec2 cmin = {0.0f, 0.0f}, Vec2 cmax = {1.0f, 1.0f});

    // Constructor from parent AvatarJoint
    JointTexture (AssetManager& assets, const std::string& path, const AvatarJoint& joint);

};

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


    // Storage of additional animation textures
    std::vector<JointTexture> animation_texture_library;

    // Loading functions
    bool joint_has_texture () { return (texture != nullptr);    }
    bool unload_texture (AssetManager & assets);
    bool load_texture_from_path (AssetManager & assets, const std::string & path);

    // Push back new anim texture
    bool push_back_new_anim_texture (AssetManager& assets, const std::string & path) {
        // Make sure path is not empty
        if (path.empty())
            return false;
        // Import new texture for the AvatarJoint group
        animation_texture_library.push_back(JointTexture(assets, path, *this));
        return true;
    } 

    bool change_anim_texture (AssetManager& assets, int index, const std::string& path);

    const JointTexture* get_anim_texture_or_null (int index) const {
        if (index < 0 || index >= (int)animation_texture_library.size())
            return nullptr;
        return &animation_texture_library[(size_t)index];
    }

    ResolvedJointTexture resolve_texture (int anim_texture_idx) const;
    
    // Unload all anim textures
    bool unload_all_anim_textures () {
        animation_texture_library.clear();
        return true;
    }

    // AvatarJoint constructors
    AvatarJoint ()                          {   name = "UNTITLED_AVATARJOINT_DEFAULT_CONSTRUCTOR";  }
    AvatarJoint (const std::string & _name) {   this->name = _name;                                 }

    /*  How to add textures...

        // 1.) Make a joint

        AvatarJoint this_joint = AvatarJoint("leg");
        this_joint.load_texture_from_path(assets, "image/address/img.jpg");             // original texture; id: -1

        // 2.) Add new animation textures

        this_joint.push_back_new_anim_texture(assets, "image/address/img_anim_1.jpg");  // anim id: 0
        this_joint.push_back_new_anim_texture(assets, "image/address/img_anim_2.jpg");  // anim id: 1
        this_joint.push_back_new_anim_texture(assets, "image/address/img_anim_3.jpg");  // anim id: 2
        this_joint.push_back_new_anim_texture(assets, "image/address/img_anim_4.jpg");  // anim id: 3
    
    */

};

// Every avatar should have
//      - 1. active AvatarTextureRig
//      - 2. active KeyFrame
//      - 3. active

struct AvatarTextureRig {

    // Storage of Joint Texturing native information
    std::vector<AvatarJoint> joints;

    

    // Clear joints helper
    void clear () {
        joints.clear();
    }


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
class Avatar;


struct KeyAnimFrame {

    enum class TransitionMode {
        Linear = 0,
        Instant = 1,
        EaseInOut = 2
    };

    std::vector<AnimJointAdjustmentFrame> joints;

    int sequence_id = 0;
    int time_to_next = 8;

    int start_tick_frame = 0;
   
    TransitionMode transition_mode = TransitionMode::Linear;

    
    void clear () {
        joints.clear();
    }

    KeyAnimFrame () {
        joints.clear();
    }

    // Constructor to build key anim frame from Avatar (ex. into animation list) by making same # joints per key frame for an avatar
    KeyAnimFrame (const Avatar& _avatar);

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
        KeyAnimFrame active_frame;

        Avatar (std::string _name) {
            this->name = _name;
            active_texturing.clear();
            active_frame.clear();
        }

        Avatar () {
            name = "UNTITLED_AVATAR_DEFAULT_CONSTRUCTOR";
            position = {0.0f, 0.0f};
            active_texturing.clear();
            active_frame.clear();
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

        void LoadInternalJointTextures (AssetManager& assets);


        // A tick is what could often be called a frame of animation,
        // but the problem is frames are constant measured in ms
        // --> More common use of frames in my code is in reference to KEY FRAMES which happen at
        // arbitrary user defined times, but always occur on some tick

        void DrawAvatar (Renderer& renderer, Vec2 position, bool mirror_x, const Animation& animation, int tick_frame);

        void DrawAvatarBlend (Renderer& renderer, Vec2 position, bool mirror_x, const Animation& anim1, int anim1_tick_frame, const Animation& anim2, int anim2_tick_frame, int tick_frame, int total_blend_tick_frames);

        void DrawAvatarBlendFromPose (Renderer& renderer, Vec2 position, bool mirror_x, const std::vector<AnimJointAdjustmentFrame>& pose_a, const Animation& anim2, int anim2_tick_frame, int tick_frame, int total_blend_tick_frames);

        
        bool SaveAvrFile (const std::string& filename, const std::string& path);

        bool LoadAvrFile (const std::string& filename, const std::string& path);


    private:

    

};







#endif


