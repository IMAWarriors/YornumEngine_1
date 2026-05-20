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
    float            rotation,
    Vec2             scale,
    const Animation& animation,
    int              tick_frame)
{
    if (animation.frames.empty())         return;
    if (default_frame.joints.empty())     return;
    if (default_texturing.joints.empty()) return;
 
    const int joint_count = (int)std::min(
        default_frame.joints.size(),
        default_texturing.joints.size());
 
    // -------------------------------------------------------------------------
    // 1.  Wrap tick_frame into [0, total_ticks)
    // -------------------------------------------------------------------------
    const int total_ticks = TotalTickFrames(animation);
    const int wrapped     = ((tick_frame % total_ticks) + total_ticks) % total_ticks;
 
    // -------------------------------------------------------------------------
    // 2.  Find which keyframe segment `wrapped` falls in.
    //     Each segment k spans  [cursor, cursor + time_to_next + 1)  ticks.
    // -------------------------------------------------------------------------
    int key_idx        = 0;
    int key_start_tick = 0;
    {
        int cursor = 0;
        for (int k = 0; k < (int)animation.frames.size(); ++k) {
            int seg_end = cursor + animation.frames[k].time_to_next + 1;
            if (wrapped >= cursor && wrapped < seg_end) {
                key_idx        = k;
                key_start_tick = cursor;
                break;
            }
            cursor = seg_end;
        }
    }
 
    const int next_idx  = (key_idx + 1) % (int)animation.frames.size();
    const KeyAnimFrame& frame_a = animation.frames[key_idx];
    const KeyAnimFrame& frame_b = animation.frames[next_idx];
 
    // t = progress through this segment  [0, 1)
    const int   seg_len = std::max(1, frame_a.time_to_next + 1);
    const float t       = std::clamp(
                              (float)(wrapped - key_start_tick) / (float)seg_len,
                              0.0f, 1.0f);
 
    // -------------------------------------------------------------------------
    // 3.  Build draw-order table.
    //     jidx_draw_order[i] = joint array index whose draw_order field == i.
    //     This exactly mirrors the editor's jidx_draw_order construction
    //     (EditorUISystem.cpp lines 1786-1801).
    // -------------------------------------------------------------------------
    std::vector<int> jidx_draw_order(joint_count, 0);
    for (int i = 0; i < joint_count; ++i) {
        for (int j = 0; j < joint_count; ++j) {
            if (j < (int)frame_a.joints.size() &&
                (int)frame_a.joints[j].draw_order == i)
            {
                jidx_draw_order[i] = j;
                break;
            }
        }
    }
 
    // -------------------------------------------------------------------------
    // 4.  Pre-compute avatar-level transform (rotation + scale).
    //     This is applied to each joint's local position to move it into
    //     world space around `position`.  Matches the notion of "rotate/scale
    //     the entire canvas" rather than rotating individual sprites.
    // -------------------------------------------------------------------------
    const float avatar_rot_rad = rotation * PI / 180.0f;
    const float cos_av         = cosf(avatar_rot_rad);
    const float sin_av         = sinf(avatar_rot_rad);
 
    const float zoom = renderer.get_camera_zoom();
 
    // -------------------------------------------------------------------------
    // 5.  Draw each joint in draw order (back to front).
    // -------------------------------------------------------------------------
    for (int i = 0; i < joint_count; ++i) {
 
        const int idx = jidx_draw_order[i];
 
        // Safety: skip if index is out of range for any parallel array
        if (idx >= (int)default_frame.joints.size())     continue;
        if (idx >= (int)default_texturing.joints.size()) continue;
        if (idx >= (int)frame_a.joints.size())           continue;
        if (idx >= (int)frame_b.joints.size())           continue;
 
        const JointFramePosition& anchor   = default_frame.joints[idx];
        const AvatarJoint&        tex_info = default_texturing.joints[idx];
 
        if (tex_info.texture == nullptr) continue;
 
        const AnimJointAdjustmentFrame& ja = frame_a.joints[idx];
        const AnimJointAdjustmentFrame& jb = frame_b.joints[idx];
 
        // ---------------------------------------------------------------------
        // 5a.  Interpolate the joint adjustment between frame_a and frame_b.
        //      Mirrors editor lines 1860-1874 exactly.
        //        - origin:   linear lerp
        //        - rotation: shortest-arc or long-arc depending on normal_rotation
        // ---------------------------------------------------------------------
        AnimJointAdjustmentFrame ji{};
 
        ji.origin.x = ja.origin.x + (jb.origin.x - ja.origin.x) * t;
        ji.origin.y = ja.origin.y + (jb.origin.y - ja.origin.y) * t;
 
        {
            float shortest = WrapDeg180(jb.rotation - ja.rotation);
            float delta;
            if (ja.normal_rotation) {
                delta = shortest;                                    // short path
            } else {
                delta = (shortest >= 0.0f) ? shortest - 360.0f      // long path
                                           : shortest + 360.0f;
            }
            ji.rotation = ja.rotation + delta * t;
        }
 
        // ---------------------------------------------------------------------
        // 5b.  Compute the joint's world-space center position.
        //      local  = anchor.origin + interpolated_adjustment.origin
        //      world  = position + rotate(scale(local), avatar_rotation)
        //
        //      This is the exact equivalent of the editor's WorldToScreen() on
        //      (joint_anchor.origin + joint_interp.origin).
        // ---------------------------------------------------------------------
        Vec2 joint_local = {
            anchor.origin.x + ji.origin.x,
            anchor.origin.y + ji.origin.y
        };
 
        // Apply avatar scale, then avatar rotation, then translate to position
        Vec2 joint_scaled  = { joint_local.x * scale.x,
                                joint_local.y * scale.y };
        Vec2 joint_rotated = Rotate2D(joint_scaled, cos_av, sin_av);
        Vec2 joint_world   = { position.x + joint_rotated.x,
                                position.y + joint_rotated.y };
 
        // Convert to screen space using the renderer's camera transform
        Vec2 screen_center = renderer.world_camera_transform(joint_world);
 
        // ---------------------------------------------------------------------
        // 5c.  Compute the sprite's final rotation angle.
        //      newDir     = RotNewDirectionVec(anchor.direction, ji.rotation)
        //                   (rotates the anchor's direction vector by the anim
        //                    rotation in degrees - same as editor line 1908)
        //      base_angle = atan2(newDir.y, newDir.x)   (radians)
        //      final      = base_angle + tex_info.rotation + avatar_rot_rad
        //                   (tex_info.rotation is stored in radians)
        // ---------------------------------------------------------------------
        Vec2  new_dir    = RotNewDirectionVec(anchor.direction, ji.rotation);
        float base_angle = atan2f(new_dir.y, new_dir.x);
        float angle      = base_angle + tex_info.rotation + avatar_rot_rad;
 
        float cosA = cosf(angle);
        float sinA = sinf(angle);
 
        // ---------------------------------------------------------------------
        // 5d.  Sprite display dimensions in world units.
        //      width  = texture.width  * tex_info.scale.x
        //      height = texture.height * tex_info.scale.y
        //
        //      Avatar-level scale also stretches/squishes the limb size.
        //      We multiply by |scale| so e.g. a 2x avatar has 2x limbs.
        //      Using abs() so that mirroring (negative scale) doesn't invert
        //      the sprite size - it only moves the joint center.
        // ---------------------------------------------------------------------
        float w = tex_info.texture->width  * tex_info.scale.x * fabsf(scale.x);
        float h = tex_info.texture->height * tex_info.scale.y * fabsf(scale.y);
 
        // ---------------------------------------------------------------------
        // 5e.  Sprite offset (texture_joint.offset), applied in LOCAL space
        //      before rotation - mirrors editor lines 1929-1933 exactly.
        //
        //      Avatar scale affects the offset distance too.
        // ---------------------------------------------------------------------
        float ox = tex_info.offset.x * fabsf(scale.x);
        float oy = tex_info.offset.y * fabsf(scale.y);
 
        // ---------------------------------------------------------------------
        // 5f.  Build 4 screen-space quad corners.
        //      Start with a centered rectangle [-w/2, +w/2] x [-h/2, +h/2].
        //      Shift each corner by the offset.
        //      Rotate by `angle`.
        //      Translate to screen_center.
        //      Multiply the rotated offset by zoom to convert world->screen.
        //
        //      This exactly replicates editor lines 1920-1934:
        //        corners[k] = { -half.x, -half.y } ... { half.x, half.y }
        //        x = corners[k].x + offset.x
        //        y = corners[k].y + offset.y
        //        rotated.x = center.x + x*cosA - y*sinA
        //        rotated.y = center.y + x*sinA + y*cosA
        //      The editor's offset.x is already in screen pixels (offset * zoom),
        //      our ox/oy are world pixels so we multiply by zoom here.
        //
        //      corners[0]=TL  [1]=TR  [2]=BR  [3]=BL
        // ---------------------------------------------------------------------
        float hx = w * 0.5f;
        float hy = h * 0.5f;
 
        // Local (unrotated) corners, centered at zero
        const Vec2 local_corners[4] = {
            { -hx, -hy },   // TL
            {  hx, -hy },   // TR
            {  hx,  hy },   // BR
            { -hx,  hy }    // BL
        };
 
        Vector2 screen_corners[4];
        for (int k = 0; k < 4; ++k) {
            // Offset in local frame (world units)
            float lx = local_corners[k].x + ox;
            float ly = local_corners[k].y + oy;
            // Rotate in world units, then scale to screen pixels via zoom
            float rx = (lx * cosA - ly * sinA) * zoom;
            float ry = (lx * sinA + ly * cosA) * zoom;
            screen_corners[k].x = screen_center.x + rx;
            screen_corners[k].y = screen_center.y + ry;
        }
 
        // ---------------------------------------------------------------------
        // 5g.  UV coordinates - directly from crop_min / crop_max (0..1 range).
        //      The editor passes these straight to ImGui as UV0/UV1.
        // ---------------------------------------------------------------------
        Vector2 uv_min = { tex_info.crop_min.x, tex_info.crop_min.y };
        Vector2 uv_max = { tex_info.crop_max.x, tex_info.crop_max.y };
 
        // ---------------------------------------------------------------------
        // 5h.  Submit the quad to the renderer.
        // ---------------------------------------------------------------------
        renderer.rdraw_quad_screen(*tex_info.texture, screen_corners,
                                   uv_min, uv_max, WHITE);
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
    Vec2 position, float rotation, Vec2 scale,
    const Animation& anim1, int anim1_tick_frame,
    const Animation& anim2, int anim2_tick_frame,
    int tick_frame, int total_blend_tick_frames)
{
    // Not called by DrawAvatar anymore.
    // Implement later when cross-animation blending is needed.
    (void)renderer; (void)position; (void)rotation; (void)scale;
    (void)anim1; (void)anim1_tick_frame;
    (void)anim2; (void)anim2_tick_frame;
    (void)tick_frame; (void)total_blend_tick_frames;
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