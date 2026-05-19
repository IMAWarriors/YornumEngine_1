// Avatar.cpp

#include "Avatar.h"
#include "Animation.h"

#include "../../Assets/AssetManager.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace {
    // constexpr float PI = 3.14159265358979323846f;

    float WrapDeg180(float angle) {
        float wrapped = fmodf(angle, 360.0f);
        if (wrapped > 180.0f) wrapped -= 360.0f;
        if (wrapped <= -180.0f) wrapped += 360.0f;
        return wrapped;
    }

    Vec2 RotNewDirectionVec(Vec2 input_direction, float rotation_deg) {
        float rotation_rad = rotation_deg * PI / 180.0f;
        float cos_theta = cosf(rotation_rad);
        float sin_theta = sinf(rotation_rad);
        return Vec2{
            input_direction.x * cos_theta - input_direction.y * sin_theta,
            input_direction.x * sin_theta + input_direction.y * cos_theta
        };
    }

    int TotalTickFrames(const Animation& anim) {
        int total = 0;
        for (const KeyAnimFrame& k : anim.frames) total += (k.time_to_next + 1);
        return std::max(1, total);
    }
}

bool AvatarJoint::unload_texture (AssetManager & assets) {
    assets.UnloadTextureAsset(texture);
    texture = nullptr;
    return true;
}

bool AvatarJoint::load_texture_from_path (AssetManager & assets, const std::string & path) {
    if (path.empty()) {
        return false;
    }

    if (joint_has_texture()) {
        unload_texture(assets);
    }

    texture = &assets.LoadTextureAsset(path);
    texturePath = path;
    return true;
}

void Avatar::LoadInternalJointTextures (AssetManager& assets) {
    for (AvatarJoint& joint : default_texturing.joints) {
        joint.load_texture_from_path(assets, joint.texturePath);
    }
}

// =============================================================
// Avatar Interpolation Drawing Definitions:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
KeyAnimFrame::KeyAnimFrame (const Avatar& _avatar) {
    for (int i = 0; i < _avatar.default_frame.joints.size(); i++) {
        // Default draw order should for a key animation frame
        // match the anchor-frame joint order for consistency.
        joints.push_back(AnimJointAdjustmentFrame({0.0f, 0.0f}, 0.0f, i));
        joints.back().unique_id = i;

    }
}


// Draw regular avatar according to some frame and calculate the interpolated joint positions
// for which to draw the avatar---should essentially draw the exact same way the editor build
// EditorUISystem.cpp draws the avatar according to animation except with Raylib instead of
// ImGUI
void Avatar::DrawAvatar (Renderer& renderer, Vec2 position, float rotation, Vec2 scale, const Animation& animation, int tick_frame) {
    if (animation.frames.empty()) return;
    if (default_frame.joints.empty() || default_texturing.joints.empty()) return;

    int total_ticks = TotalTickFrames(animation);
    int wrapped_tick = ((tick_frame % total_ticks) + total_ticks) % total_ticks;

    int key_idx = 0;
    int key_start_tick = 0;
    int tick_cursor = 0;
    for (int k = 0; k < (int)animation.frames.size(); k++) {
        int segment_end = tick_cursor + animation.frames[k].time_to_next + 1;
        if (wrapped_tick >= tick_cursor && wrapped_tick < segment_end) {
            key_idx = k;
            key_start_tick = tick_cursor;
            break;
        }
        tick_cursor = segment_end;
    }

    int next_idx = (key_idx + 1) % (int)animation.frames.size();
    const KeyAnimFrame& frame_a = animation.frames[key_idx];
    const KeyAnimFrame& frame_b = animation.frames[next_idx];
    int segment_len = std::max(1, frame_a.time_to_next + 1);
    float t = std::clamp((float)(wrapped_tick - key_start_tick) / (float)segment_len, 0.0f, 1.0f);

    DrawAvatarBlend(renderer, position, rotation, scale, animation, wrapped_tick, animation, wrapped_tick + 1, (int)(t * 1000.0f), 1000);
}

// Draw avatar blend transition between Tick Frame [anim1_tick_frame] of Animation [anim1]
// and [anim2_tick_frame] of Animation [anim2], choosing the point in the transition frame
// to draw the blend based on the progresss of blend animation [tick_frame] according to
// the total frames of the blend between animations [total_blend_tick_frames]
void Avatar::DrawAvatarBlend (Renderer& renderer, Vec2 position, float rotation, Vec2 scale, const Animation& anim1, int anim1_tick_frame, const Animation& anim2, int anim2_tick_frame, int tick_frame, int total_blend_tick_frames) {
    if (default_frame.joints.empty() || default_texturing.joints.empty()) return;
    if (anim1.frames.empty() || anim2.frames.empty()) return;

    auto sampleAnim = [](const Animation& anim, int tick) -> std::vector<AnimJointAdjustmentFrame> {
        int total_ticks = TotalTickFrames(anim);
        int wrapped_tick = ((tick % total_ticks) + total_ticks) % total_ticks;
        int key_idx = 0;
        int key_start_tick = 0;
        int tick_cursor = 0;
        for (int k = 0; k < (int)anim.frames.size(); k++) {
            int segment_end = tick_cursor + anim.frames[k].time_to_next + 1;
            if (wrapped_tick >= tick_cursor && wrapped_tick < segment_end) {
                key_idx = k;
                key_start_tick = tick_cursor;
                break;
            }
            tick_cursor = segment_end;
        }
        int next_idx = (key_idx + 1) % (int)anim.frames.size();
        const KeyAnimFrame& frame_a = anim.frames[key_idx];
        const KeyAnimFrame& frame_b = anim.frames[next_idx];
        int seg_len = std::max(1, frame_a.time_to_next + 1);
        float t = std::clamp((float)(wrapped_tick - key_start_tick) / (float)seg_len, 0.0f, 1.0f);
        std::vector<AnimJointAdjustmentFrame> out = frame_a.joints;
        for (size_t i = 0; i < out.size() && i < frame_b.joints.size(); i++) {
            const AnimJointAdjustmentFrame& a = frame_a.joints[i];
            const AnimJointAdjustmentFrame& b = frame_b.joints[i];
            out[i].origin.x = a.origin.x + (b.origin.x - a.origin.x) * t;
            out[i].origin.y = a.origin.y + (b.origin.y - a.origin.y) * t;
            float shortest_delta = WrapDeg180(b.rotation - a.rotation);
            float rot_delta = a.normal_rotation ? shortest_delta : ((shortest_delta >= 0.0f) ? shortest_delta - 360.0f : shortest_delta + 360.0f);
            out[i].rotation = a.rotation + rot_delta * t;
        }
        return out;
    };

    std::vector<AnimJointAdjustmentFrame> pose_a = sampleAnim(anim1, anim1_tick_frame);
    std::vector<AnimJointAdjustmentFrame> pose_b = sampleAnim(anim2, anim2_tick_frame);

    float blend_t = (total_blend_tick_frames <= 0) ? 1.0f : std::clamp((float)tick_frame / (float)total_blend_tick_frames, 0.0f, 1.0f);

    int joint_count = std::min({(int)default_frame.joints.size(), (int)default_texturing.joints.size(), (int)pose_a.size(), (int)pose_b.size()});
    std::vector<int> draw_order(joint_count);
    for (int i = 0; i < joint_count; i++) draw_order[i] = i;
    std::sort(draw_order.begin(), draw_order.end(), [&](int l, int r) {
        int dl = (int)(pose_a[l].draw_order + (pose_b[l].draw_order - pose_a[l].draw_order) * blend_t);
        int dr = (int)(pose_a[r].draw_order + (pose_b[r].draw_order - pose_a[r].draw_order) * blend_t);
        return dl < dr;
    });

    float root_rad = rotation * PI / 180.0f;
    float c = cosf(root_rad), s = sinf(root_rad);
    for (int idx : draw_order) {
        const JointFramePosition& anchor = default_frame.joints[idx];
        const AvatarJoint& texture_joint = default_texturing.joints[idx];
        if (texture_joint.texture == nullptr) continue;

        AnimJointAdjustmentFrame j = pose_a[idx];
        j.origin.x += (pose_b[idx].origin.x - pose_a[idx].origin.x) * blend_t;
        j.origin.y += (pose_b[idx].origin.y - pose_a[idx].origin.y) * blend_t;
        float d = WrapDeg180(pose_b[idx].rotation - pose_a[idx].rotation);
        j.rotation += d * blend_t;

        Vec2 base_joint_pos = {anchor.origin.x + j.origin.x, anchor.origin.y + j.origin.y};
        Vec2 scaled = {base_joint_pos.x * scale.x, base_joint_pos.y * scale.y};
        Vec2 rotated = {scaled.x * c - scaled.y * s, scaled.x * s + scaled.y * c};
        Vec2 world_anchor = {position.x + rotated.x, position.y + rotated.y};

        Vec2 anim_dir = RotNewDirectionVec(anchor.direction, j.rotation);
        float base_angle_rad = atan2f(anim_dir.y, anim_dir.x);
        float final_rotation_rad = root_rad + base_angle_rad + texture_joint.rotation;
        float final_rotation = final_rotation_rad * 180.0f / PI;

        Vec2 final_scale = {scale.x * texture_joint.scale.x, scale.y * texture_joint.scale.y};

        Vec2 offset = {texture_joint.offset.x * scale.x, texture_joint.offset.y * scale.y};
        float co = cosf(final_rotation_rad), so = sinf(final_rotation_rad);
        Vec2 o_rot = {offset.x * co - offset.y * so, offset.x * so + offset.y * co};
        
        world_anchor = {world_anchor.x + o_rot.x, world_anchor.y + o_rot.y};

        float crop_w = (texture_joint.crop_max.x - texture_joint.crop_min.x) * texture_joint.texture->width;
        float crop_h = (texture_joint.crop_max.y - texture_joint.crop_min.y) * texture_joint.texture->height;
        Rectangle crop = {
            texture_joint.crop_min.x * texture_joint.texture->width,
            texture_joint.crop_min.y * texture_joint.texture->height,
            crop_w,
            crop_h
        };
        
        renderer.rdraw_sprite_world_ext(*texture_joint.texture, crop, world_anchor, {crop_w * 0.5f, crop_h * 0.5f}, final_rotation, final_scale, WHITE);
    }
}


bool Avatar::SaveAvrFile (const std::string& filename, const std::string& path) {

    std::string fullpath = path + filename + ".avr";

    std::ofstream file(fullpath);


    if (!file.is_open())
        return false;


    // Items to serialize

    // --> Avatar's Name (not the same as the filename)     || std::string
    // --> Anchor Frame                                     || Avatar::AvatarTextureRig
    // --> Default Texturing                                || Avatar::KeyFrame

    // --------------------------
    // :: AvatarTextureRig
    // --------------------------
    // ----> std::vector<AvatarJoints> [Joint Count, Joint Data]

    // :: AvatarJoint -->
    // ----> Joint Name        || std::string
    // ----> Texture Path      || std::string
    // ----> Offset            || Vec2
    // ----> Scale             || Float
    // ----> Rotation          || Float
    // ----> Crop Min          || Vec2
    // ----> Crop Max          || Vec2
    

    // ---------------------------
    // :: KeyFrame 
    // ---------------------------
    // ----> std::vector<JointFramePosition>

    // :: JointFramePosition -->
    // ----> Unique ID      || int
    // ----> Origin         || Vec2
    // ----> Direction      || Vec2

    // ==============================================================================================
    // ==> Serialize Version
    // --------------------------->


    const std::string version = "VERSION_1";

    file << "~AVATAR_FILE" << '\n';
    file << version << '\n';

    // ==============================================================================================
    // ==> Serialize Avatar
    // --------------------------->
    
    const std::string avatar_name = name;

    file << "~AVATAR_NAME" << '\n';
    file << name << '\n';

    file << "~DEFAULT_TEXTURING" << '\n';

    const int number_of_joints = default_texturing.joints.size();
    file << number_of_joints << '\n';

    for (int i = 0; i < number_of_joints; i++) {

        const std::string jname = default_texturing.joints[i].name;
        const std::string jpath = default_texturing.joints[i].texturePath;
        const float joffset_x = default_texturing.joints[i].offset.x;
        const float joffset_y = default_texturing.joints[i].offset.y;
        const float jscale_x = default_texturing.joints[i].scale.x;
        const float jscale_y = default_texturing.joints[i].scale.y;
        const float jrotation = default_texturing.joints[i].rotation;
        const float jcmin_x = default_texturing.joints[i].crop_min.x;
        const float jcmin_y = default_texturing.joints[i].crop_min.y;
        const float jcmax_x = default_texturing.joints[i].crop_max.x;
        const float jcmax_y = default_texturing.joints[i].crop_max.y;

        file << jname << '\n';
        
        file << jpath << '\n';

        file << joffset_x << " " << joffset_y << " " << jscale_x << " " << jscale_y << " " << jrotation << " " << jcmin_x << " " << jcmin_y << " " << jcmax_x  << " " << jcmax_y << '\n';


    }

    

    file << "~ANCHOR_FRAME" << '\n';

    
    for (int i = 0; i < number_of_joints; i++) {

        const int juniqueid = default_frame.joints[i].unique_id;
        const float jox = default_frame.joints[i].origin.x;
        const float joy = default_frame.joints[i].origin.y;
        const float jdx = default_frame.joints[i].direction.x;
        const float jdy = default_frame.joints[i].direction.y;

        file << juniqueid << " " << jox << " " << joy << " " << jdx << " " << jdy << '\n';

    }

    file << "~END_OF_FILE" << '\n';



    return true;

}

bool Avatar::LoadAvrFile (const std::string& filename, const std::string& path) {

    std::string fullpath = path + filename + ".avr";
    

    std::ifstream file(fullpath);

    if (!file.is_open())
        return false;

    std::string token;

    // =========================================================================================
    // ==> FILE HEADER
    // =========================================================================================

    file >> token;

    if (token != "~AVATAR_FILE")
        return false;

    std::string version;
    file >> version;

    const bool version_1 = (version == "VERSION_1");

    if (!version_1)
        return false;

    // =========================================================================================
    // ==> AVATAR NAME
    // =========================================================================================

    file >> token;

    if (token != "~AVATAR_NAME")
        return false;


    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(file, token, '\n');

    name = token;

    // =========================================================================================
    // ==> DEFAULT TEXTURING
    // =========================================================================================

    file >> token;

    if (token != "~DEFAULT_TEXTURING")
        return false;


    int number_of_joints = 0;

    file >> number_of_joints;

    default_texturing.joints.clear();

    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < number_of_joints; i++) {
        AvatarJoint joint;

        std::getline(file, joint.name, '\n');

        std::getline(file, joint.texturePath, '\n');
        file >> joint.offset.x
                >> joint.offset.y
                >> joint.scale.x
                >> joint.scale.y
                >> joint.rotation
                >> joint.crop_min.x
                >> joint.crop_min.y
                >> joint.crop_max.x
                >> joint.crop_max.y;
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        default_texturing.joints.push_back(joint);
    }

    // =========================================================================================
    // ==> ANCHOR FRAME
    // =========================================================================================

    file >> token;

    if (token != "~ANCHOR_FRAME")
        return false;

    default_frame.joints.clear();

    for (int i = 0; i < number_of_joints; i++) {
        JointFramePosition frame_joint;

        file >> frame_joint.unique_id
             >> frame_joint.origin.x
             >> frame_joint.origin.y
             >> frame_joint.direction.x
             >> frame_joint.direction.y;

        default_frame.joints.push_back(frame_joint);
    }

    // =========================================================================================
    // ==> END OF FILE
    // =========================================================================================

    file >> token;

    if (token != "~END_OF_FILE")
        return false;

    return true;



}