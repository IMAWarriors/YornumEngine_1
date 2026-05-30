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

    // Rotate a 2D point around the origin.
    static Vec2 Rotate2D(Vec2 p, float cos_a, float sin_a) {
        return { p.x * cos_a - p.y * sin_a,
                 p.x * sin_a + p.y * cos_a };
    }

    int TotalTickFrames(const Animation& anim) {
        int total = 0;
        for (const KeyAnimFrame& k : anim.frames) total += (k.time_to_next + 1);
        return std::max(1, total);
    }

    static std::vector<AnimJointAdjustmentFrame> SampleAnimPose(const Animation& animation, int tick_frame) {
        if (animation.frames.empty()) return {};

        const int total_ticks = TotalTickFrames(animation);
        const int wrapped_tick = ((tick_frame % total_ticks) + total_ticks) % total_ticks;

        int interp_frame_idx = 0;
        int interp_frame_start_tick = 0;
        int tick_cursor = 0;
        for (int k = 0; k < (int)animation.frames.size(); k++) {
            int segment_len = animation.frames[k].time_to_next;
            int segment_end = tick_cursor + segment_len + 1;
            if (wrapped_tick >= tick_cursor && wrapped_tick < segment_end) {
                interp_frame_idx = k;
                interp_frame_start_tick = tick_cursor;
                break;
            }
            tick_cursor = segment_end;
        }

        int next_frame_idx = (interp_frame_idx + 1) % (int)animation.frames.size();
        int segment_len = animation.frames[interp_frame_idx].time_to_next;
        float t = (float)(wrapped_tick - interp_frame_start_tick) / ((float)segment_len + 1.0f);
        t = std::clamp(t, 0.0f, 1.0f);

        const KeyAnimFrame::TransitionMode transition_mode = animation.frames[interp_frame_idx].transition_mode;
        if (transition_mode == KeyAnimFrame::TransitionMode::Instant) t = 0.0f;
        else if (transition_mode == KeyAnimFrame::TransitionMode::EaseInOut) t = t * t * (3.0f - 2.0f * t);

        std::vector<AnimJointAdjustmentFrame> out = animation.frames[interp_frame_idx].joints;
        const int joint_count = (int)out.size();
        for (int idx = 0; idx < joint_count && idx < (int)animation.frames[next_frame_idx].joints.size(); idx++) {
            const auto& a = animation.frames[interp_frame_idx].joints[idx];
            const auto& b = animation.frames[next_frame_idx].joints[idx];
            out[idx].origin.x = a.origin.x + ((b.origin.x - a.origin.x) * t);
            out[idx].origin.y = a.origin.y + ((b.origin.y - a.origin.y) * t);
            float raw_delta = (b.rotation - a.rotation);
            float shortest_delta = WrapDeg180(raw_delta);
            if (a.normal_rotation) out[idx].rotation = a.rotation + (shortest_delta * t);
            else {
                float long_delta = (shortest_delta >= 0.0f) ? (shortest_delta - 360.0f) : (shortest_delta + 360.0f);
                out[idx].rotation = a.rotation + (long_delta * t);
            }
        }
        return out;
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

// ============= Packaged Animation Joint Textures ==================

// Selective constructor
JointTexture::JointTexture (AssetManager& assets, const std::string& path, Vec2 tex_offset, Vec2 tex_scale, float tex_rot, Vec2 cmin, Vec2 cmax) {
    texture_path = path;
    texture_ptr = &assets.LoadTextureAsset(path);
    offset = tex_offset;     // position relative to joint
    scale = tex_scale;      // size multiplier
    rotation = tex_rot;    // rotation in radians
    crop_min = cmin;   // UV min (0–1)
    crop_max = cmax;   // UV max (0–1)
}

// Constructor from parent AvatarJoint
JointTexture::JointTexture (AssetManager& assets, const std::string& path, const AvatarJoint& joint) {
    texture_path = path;
    texture_ptr = &assets.LoadTextureAsset(path);
    offset = joint.offset;     // position relative to joint
    scale = joint.scale;      // size multiplier
    rotation = joint.rotation;    // rotation in radians
    crop_min = joint.crop_min;   // UV min (0–1)
    crop_max = joint.crop_max;   // UV max (0–1)
}

// ==================================================================



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


// Mirrors the editor's interpolation-playback draw path (EditorUISystem.cpp
// lines 1786-1951) exactly, translated from ImGui screen-space quads into
// Raylib world-space quads via Renderer::rdraw_quad_screen().
//
// Parameters
//   position   - world-space point the avatar is anchored to
//   rotation   - degrees; rotates the ENTIRE rendered avatar around `position`
//   scale      - scales the ENTIRE rendered avatar around `position`
//               (use {-1, 1} to mirror horizontally, {1,1} is normal)
//   animation  - which animation to sample
//   tick_frame - integer tick counter (incremented by AvatarRenderer each frame)
// -----------------------------------------------------------------------------
void Avatar::DrawAvatar(
    Renderer&        renderer,
    Vec2             position,
    bool             mirror_x,
    const Animation& animation,
    int              tick_frame) {

    // -----------
    
    if (default_frame.joints.empty() ||
        default_texturing.joints.empty() ||
        animation.frames.empty()) {
        return;
    }

    const int anchor_joint_count = (int)default_frame.joints.size();
    const int texture_joint_count = (int)default_texturing.joints.size();
    const int animation_joint_count = animation.joints_defined > 0 ? animation.joints_defined : (int)animation.frames.front().joints.size();

    if (anchor_joint_count != texture_joint_count ||
        anchor_joint_count != animation_joint_count) {
        // Animation/avatar mismatch: refuse draw instead of rendering scrambled joint mapping.
        return;
    }

    const int joint_count = anchor_joint_count;
    const int total_ticks = TotalTickFrames(animation);
    const int wrapped_tick = ((tick_frame % total_ticks) + total_ticks) % total_ticks;

    int interp_frame_idx = 0;
    int interp_frame_start_tick = 0;
    {
        int tick_cursor = 0;
        for (int k = 0; k < (int)animation.frames.size(); k++) {
            int segment_len = animation.frames[k].time_to_next;
            int segment_end = tick_cursor + segment_len + 1;
            if (wrapped_tick >= tick_cursor && wrapped_tick < segment_end) {
                interp_frame_idx = k;
                interp_frame_start_tick = tick_cursor;
                break;
            }
            tick_cursor = segment_end;
            interp_frame_idx = k;
            interp_frame_start_tick = tick_cursor;
        }
    }

    std::vector<int> draw_order_idx(joint_count);
    for (int i = 0; i < joint_count; i++) {
        int jidx = i;
        for (int j = 0; j < joint_count && j < (int)animation.frames[interp_frame_idx].joints.size(); j++) {
            if (i == animation.frames[interp_frame_idx].joints[j].draw_order) {
                jidx = j;
                break;
            }
        }
        draw_order_idx[i] = jidx;
    }

    int segment_len = animation.frames[interp_frame_idx].time_to_next;
    float t = (float)(wrapped_tick - interp_frame_start_tick) / ((float)segment_len + 1.0f);
    t = std::clamp(t, 0.0f, 1.0f);

    const KeyAnimFrame::TransitionMode transition_mode = animation.frames[interp_frame_idx].transition_mode;
    if (transition_mode == KeyAnimFrame::TransitionMode::Instant) {
        t = 0.0f;
    } else if (transition_mode == KeyAnimFrame::TransitionMode::EaseInOut) {
        t = t * t * (3.0f - 2.0f * t);
    }
    int next_frame_idx = (interp_frame_idx + 1) % (int)animation.frames.size();

    const Vector2 mirror_pivot_screen = {
        renderer.world_camera_transform(position).x,
        renderer.world_camera_transform(position).y
    };

    for (int i = 0; i < joint_count; i++) {
        const int idx = draw_order_idx[i];
        if (idx < 0 || idx >= joint_count || idx >= (int)animation.frames[interp_frame_idx].joints.size() || idx >= (int)animation.frames[next_frame_idx].joints.size()) {
            continue;
        }

        const auto& joint_anchor = default_frame.joints[idx];
        const auto& joint_texture = default_texturing.joints[idx];
        const auto& joint_anim = animation.frames[interp_frame_idx].joints[idx];
        const auto& joint_blend = animation.frames[next_frame_idx].joints[idx];

        AnimJointAdjustmentFrame joint_interp = joint_anim;
        joint_interp.origin.x = joint_anim.origin.x + ((joint_blend.origin.x - joint_anim.origin.x) * t);
        joint_interp.origin.y = joint_anim.origin.y + ((joint_blend.origin.y - joint_anim.origin.y) * t);

        float raw_delta = (joint_blend.rotation - joint_anim.rotation);
        float shortest_delta = WrapDeg180(raw_delta);
        if (joint_anim.normal_rotation) {
            joint_interp.rotation = joint_anim.rotation + (shortest_delta * t);
        } else {
            float long_delta = (shortest_delta >= 0.0f) ? (shortest_delta - 360.0f) : (shortest_delta + 360.0f);
            joint_interp.rotation = joint_anim.rotation + (long_delta * t);
        }

        Vec2 anchor_world = {
            position.x + joint_anchor.origin.x + joint_interp.origin.x,
            position.y + joint_anchor.origin.y + joint_interp.origin.y
        };

        // Always compute the non-mirrored pose first; if mirroring is requested,
        // reflect final screen-space geometry around the avatar pivot line.
        Vec2 new_dir = RotNewDirectionVec(joint_anchor.direction, joint_interp.rotation);

        if (joint_texture.texture == nullptr) continue;

        const float zoom = renderer.get_camera_zoom();
        Vector2 center = {(float)renderer.world_camera_transform(anchor_world).x, (float)renderer.world_camera_transform(anchor_world).y};

        float width = joint_texture.texture->width * joint_texture.scale.x * zoom;
        float height = joint_texture.texture->height * joint_texture.scale.y * zoom;

        float base_angle = atan2f(new_dir.y, new_dir.x);
        float angle = base_angle + joint_texture.rotation;
        float cosA = cosf(angle);
        float sinA = sinf(angle);

        Vector2 offset = {joint_texture.offset.x * zoom, joint_texture.offset.y * zoom};

        Vector2 half = {width * 0.5f, height * 0.5f};
        Vector2 corners_local[4] = {
            {-half.x, -half.y}, {half.x, -half.y}, {half.x, half.y}, {-half.x, half.y}
        };
        Vector2 corners[4];
        for (int k = 0; k < 4; k++) {
            float x = corners_local[k].x + offset.x;
            float y = corners_local[k].y + offset.y;
            corners[k].x = center.x + (x * cosA - y * sinA);
            corners[k].y = center.y + (x * sinA + y * cosA);
        }

        Vector2 uv_min = {joint_texture.crop_min.x, joint_texture.crop_min.y};
        Vector2 uv_max = {joint_texture.crop_max.x, joint_texture.crop_max.y};

        if (mirror_x) {
            for (int k = 0; k < 4; k++) {
                corners[k].x = mirror_pivot_screen.x - (corners[k].x - mirror_pivot_screen.x);
            }

            // Reflection flips winding; remap corners back to TL,TR,BR,BL ordering
            // expected by rdraw_quad_screen so mirrored quads don't get back-face culled.
            Vector2 remapped[4] = { corners[1], corners[0], corners[3], corners[2] };
            corners[0] = remapped[0];
            corners[1] = remapped[1];
            corners[2] = remapped[2];
            corners[3] = remapped[3];

            // Also mirror texture sampling horizontally.
            std::swap(uv_min.x, uv_max.x);
        }

        renderer.rdraw_quad_screen(*joint_texture.texture, corners, uv_min, uv_max, WHITE);
    }

}
 
 
// -----------------------------------------------------------------------------
// REPLACEMENT  DrawAvatarBlend()
//
// For blending between two DIFFERENT animations (e.g. transitioning from
// an idle to a run).  DrawAvatar no longer calls this.
// Left intact for future use.  The body can stay as-is or be cleared.
// -----------------------------------------------------------------------------
void Avatar::DrawAvatarBlend(
    Renderer& renderer,
    Vec2 position,
    bool mirror_x,
    const Animation& anim1, int anim1_tick_frame,
    const Animation& anim2, int anim2_tick_frame,
    int tick_frame, int total_blend_tick_frames)
{
    if (default_frame.joints.empty() ||
        default_texturing.joints.empty() ||
        anim1.frames.empty() ||
        anim2.frames.empty()) {
        return;
    }
    const int joint_count = (int)default_frame.joints.size();
    if (joint_count != (int)default_texturing.joints.size() ||
        joint_count != (int)anim1.frames.front().joints.size() ||
        joint_count != (int)anim2.frames.front().joints.size()) {
        return;
    }

    const auto pose_a = SampleAnimPose(anim1, anim1_tick_frame);
    const auto pose_b = SampleAnimPose(anim2, anim2_tick_frame);
    if ((int)pose_a.size() != joint_count || (int)pose_b.size() != joint_count) return;

    const float blend_t = (total_blend_tick_frames <= 0)
        ? 1.0f
        : std::clamp((float)tick_frame / (float)total_blend_tick_frames, 0.0f, 1.0f);

    std::vector<AnimJointAdjustmentFrame> blended_pose = pose_a;
    for (int idx = 0; idx < joint_count; idx++) {
        blended_pose[idx].origin.x = pose_a[idx].origin.x + (pose_b[idx].origin.x - pose_a[idx].origin.x) * blend_t;
        blended_pose[idx].origin.y = pose_a[idx].origin.y + (pose_b[idx].origin.y - pose_a[idx].origin.y) * blend_t;
        float shortest_delta = WrapDeg180(pose_b[idx].rotation - pose_a[idx].rotation);
        blended_pose[idx].rotation = pose_a[idx].rotation + shortest_delta * blend_t;
        blended_pose[idx].draw_order = pose_b[idx].draw_order;
    }

    std::vector<int> draw_order_idx(joint_count);
    for (int i = 0; i < joint_count; i++) {
        int jidx = i;
        for (int j = 0; j < joint_count; j++) {
            if (i == blended_pose[j].draw_order) {
                jidx = j;
                break;
            }
        }
        draw_order_idx[i] = jidx;
    }

    const Vector2 mirror_pivot_screen = {
        renderer.world_camera_transform(position).x,
        renderer.world_camera_transform(position).y
    };

    for (int i = 0; i < joint_count; i++) {
        int idx = draw_order_idx[i];
        const auto& joint_anchor = default_frame.joints[idx];
        const auto& joint_texture = default_texturing.joints[idx];
        const auto& joint_interp = blended_pose[idx];
        Vec2 anchor_world = { position.x + joint_anchor.origin.x + joint_interp.origin.x, position.y + joint_anchor.origin.y + joint_interp.origin.y };
        Vec2 new_dir = RotNewDirectionVec(joint_anchor.direction, joint_interp.rotation);
        if (joint_texture.texture == nullptr) continue;

        const float zoom = renderer.get_camera_zoom();
        Vector2 center = {(float)renderer.world_camera_transform(anchor_world).x, (float)renderer.world_camera_transform(anchor_world).y};
        float width = joint_texture.texture->width * joint_texture.scale.x * zoom;
        float height = joint_texture.texture->height * joint_texture.scale.y * zoom;
        float angle = atan2f(new_dir.y, new_dir.x) + joint_texture.rotation;
        float cosA = cosf(angle);
        float sinA = sinf(angle);
        Vector2 offset = {joint_texture.offset.x * zoom, joint_texture.offset.y * zoom};
        Vector2 half = {width * 0.5f, height * 0.5f};
        Vector2 corners_local[4] = {{-half.x, -half.y}, {half.x, -half.y}, {half.x, half.y}, {-half.x, half.y}};
        Vector2 corners[4];
        for (int k = 0; k < 4; k++) {
            float x = corners_local[k].x + offset.x;
            float y = corners_local[k].y + offset.y;
            corners[k].x = center.x + (x * cosA - y * sinA);
            corners[k].y = center.y + (x * sinA + y * cosA);
        }
        Vector2 uv_min = {joint_texture.crop_min.x, joint_texture.crop_min.y};
        Vector2 uv_max = {joint_texture.crop_max.x, joint_texture.crop_max.y};
        if (mirror_x) {
            for (int k = 0; k < 4; k++) corners[k].x = mirror_pivot_screen.x - (corners[k].x - mirror_pivot_screen.x);
            Vector2 remapped[4] = { corners[1], corners[0], corners[3], corners[2] };
            corners[0] = remapped[0]; corners[1] = remapped[1]; corners[2] = remapped[2]; corners[3] = remapped[3];
            std::swap(uv_min.x, uv_max.x);
        }
        renderer.rdraw_quad_screen(*joint_texture.texture, corners, uv_min, uv_max, WHITE);
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