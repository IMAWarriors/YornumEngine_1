// AvatarRenderer.h

#ifndef AVATARRENDERER_H
#define AVATARRENDERER_H

#include <algorithm>
#include <cmath>
#include <deque>

#include "../../../Engine/Core/Overhead/GameTypes.h"

#include "../../Elements/Avatar/Avatar.h"
#include "../../Elements/Avatar/Animation.h"

#include "Transform.h"


namespace comp {

struct AnimQueue {
    
    Animation* animation;
    int blend_out_time;

    AnimQueue(Animation* anim, float blend_out) {
        animation = anim;
    }
    
};

struct AvatarRenderer {
    
    // Connections to real game objects
    Avatar* avatar_to_render = nullptr;

    Transform* body_anchor = nullptr;

    void ConnectTransform (Transform* anchor) {
        body_anchor = anchor;
    }

    

    // Renderer specific drawing settings
    Vec2 offset_position = {0.0f, 0.0f};
    float offset_rotation = 0.0f;
    Vec2 scale = {1.0f, 1.0f};
    bool mirror = false;
    float mirror_offset_x = 55.0f;

    bool visible = true;
    std::string animation_state = "NOT_SET";

    void SetTransformOffset (Vec2 offset) {
        offset_position = offset;
    }

    // Priority animation controls
    bool playing_animation = false;
    bool loop_base_animation = true;
    float animation_speed = 1.0f;
    bool interpolate_btwn = true; // not even dealing with this shit right now, just interpolate everything

    // Current playing-animation controls
    Animation* animation_to_play = nullptr;
    Animation* prev_animation_to_blend = nullptr;
    int prev_animation_blend_frame = 0;
    // Frozen pose used as the start of the active blend. If a new blend starts
    // during an existing blend, this stores the already-interpolated pose so
    // the character can continue smoothly from exactly what was being drawn.
    std::vector<AnimJointAdjustmentFrame> blend_start_pose;
    int blend_out_time_left = 0; // If 0, blending is complete
    int blend_out_time_total = 0;
    int tick_frame_of_animation = 0;
    bool blend_mode = false;
    int ms_per_blend_frame = 18;


    // Animation that is played by default if nothing is queued
    // or after the queue has been cleared/fully executed.
    Animation* base_animation = nullptr;
    
    // Time bucket
    float accumulated_ms = 0.0f; // += (float)(1.0f / INTENDED_FPS) * (1000.0f);
                            // += (float)(deltatime) * (1000.0f); (deltatime is measured in... seconds?)


    std::deque<AnimQueue> animation_queue;
    
    void SetAnimationSpeedMultiplier (float mult) {
        if (mult > 0.0f)
            animation_speed = mult;
    }

    // Default constructor
    AvatarRenderer () {
        
    }

    // Constructor
    AvatarRenderer (Avatar& avatar, Animation& animation) {
        avatar_to_render = &avatar;
        base_animation = &animation;
        playing_animation = true;
    }

    // Set Mirror Offset X
    void SetMirrorOffsetX (float x) {
        mirror_offset_x = x;
    }

    // Get total number of queue animations
    int GetQueueCount () {
        return animation_queue.size();
    }

    // Pause animation playing 
    void PauseAnimation () {
        playing_animation = false;
    }

    // Start animation playing if an animation to play is on
    void PlayAnimation () {
        if (animation_to_play == nullptr) {
            playing_animation = false;
            return;
        }
        
        playing_animation = true;
    }

    // Reset the current animation tick frame to 0 and play the 
    // animation by default
    void ResetCurrentAnimation (bool play_animation = true) {
        playing_animation = play_animation;

        if (playing_animation)
            PlayAnimation();
        else
            accumulated_ms = 0.0f;

        tick_frame_of_animation = 0;

    }

    static float WrapDegrees180 (float angle) {
        float wrapped = fmodf(angle, 360.0f);
        if (wrapped > 180.0f) wrapped -= 360.0f;
        if (wrapped <= -180.0f) wrapped += 360.0f;
        return wrapped;
    }

    static int GetTotalTickFrames (const Animation& animation) {
        int total = 0;
        for (const KeyAnimFrame& frame : animation.frames) {
            total += frame.time_to_next + 1;
        }
        return std::max(1, total);
    }

    static std::vector<AnimJointAdjustmentFrame> SampleAnimationPose (const Animation& animation, int tick_frame) {
        if (animation.frames.empty()) return {};

        const int total_ticks = GetTotalTickFrames(animation);
        const int wrapped_tick = ((tick_frame % total_ticks) + total_ticks) % total_ticks;

        int current_frame_idx = 0;
        int current_frame_start_tick = 0;
        int tick_cursor = 0;
        for (int frame_idx = 0; frame_idx < (int)animation.frames.size(); frame_idx++) {
            const int segment_end = tick_cursor + animation.frames[frame_idx].time_to_next + 1;
            if (wrapped_tick >= tick_cursor && wrapped_tick < segment_end) {
                current_frame_idx = frame_idx;
                current_frame_start_tick = tick_cursor;
                break;
            }
            tick_cursor = segment_end;
        }

        const int next_frame_idx = (current_frame_idx + 1) % (int)animation.frames.size();
        const KeyAnimFrame& current_frame = animation.frames[current_frame_idx];
        const KeyAnimFrame& next_frame = animation.frames[next_frame_idx];

        float pose_t = (float)(wrapped_tick - current_frame_start_tick) / ((float)current_frame.time_to_next + 1.0f);
        pose_t = std::clamp(pose_t, 0.0f, 1.0f);

        // Transition mode handling
        if (current_frame.transition_mode == KeyAnimFrame::TransitionMode::Instant) {
            pose_t = 0.0f;
        } else if (current_frame.transition_mode == KeyAnimFrame::TransitionMode::EaseInOut) {
            pose_t = pose_t * pose_t * (3.0f - 2.0f * pose_t);
        }

        std::vector<AnimJointAdjustmentFrame> pose = current_frame.joints;
        for (int idx = 0; idx < (int)pose.size() && idx < (int)next_frame.joints.size(); idx++) {
            const AnimJointAdjustmentFrame& joint_a = current_frame.joints[idx];
            const AnimJointAdjustmentFrame& joint_b = next_frame.joints[idx];

            pose[idx].origin.x = joint_a.origin.x + ((joint_b.origin.x - joint_a.origin.x) * pose_t);
            pose[idx].origin.y = joint_a.origin.y + ((joint_b.origin.y - joint_a.origin.y) * pose_t);

            const float shortest_delta = WrapDegrees180(joint_b.rotation - joint_a.rotation);
            if (joint_a.normal_rotation) {
                pose[idx].rotation = joint_a.rotation + (shortest_delta * pose_t);
            } else {
                const float long_delta = (shortest_delta >= 0.0f) ? (shortest_delta - 360.0f) : (shortest_delta + 360.0f);
                pose[idx].rotation = joint_a.rotation + (long_delta * pose_t);
            }
        }
        return pose;
    }

    static std::vector<AnimJointAdjustmentFrame> BlendPoses (
        const std::vector<AnimJointAdjustmentFrame>& pose_a,
        const std::vector<AnimJointAdjustmentFrame>& pose_b,
        float blend_t) {

        if (pose_a.empty() || pose_a.size() != pose_b.size()) return {};

        blend_t = std::clamp(blend_t, 0.0f, 1.0f);
        std::vector<AnimJointAdjustmentFrame> pose = pose_a;
        for (int idx = 0; idx < (int)pose.size(); idx++) {
            pose[idx].origin.x = pose_a[idx].origin.x + ((pose_b[idx].origin.x - pose_a[idx].origin.x) * blend_t);
            pose[idx].origin.y = pose_a[idx].origin.y + ((pose_b[idx].origin.y - pose_a[idx].origin.y) * blend_t);
            const float shortest_delta = WrapDegrees180(pose_b[idx].rotation - pose_a[idx].rotation);
            pose[idx].rotation = pose_a[idx].rotation + (shortest_delta * blend_t);
            pose[idx].draw_order = pose_b[idx].draw_order;
            pose[idx].anim_texture_idx = pose_b[idx].anim_texture_idx;
        }
        return pose;
    }

    std::vector<AnimJointAdjustmentFrame> GetCurrentBlendPose () const {
        if (blend_mode && !blend_start_pose.empty() && animation_to_play != nullptr) {
            std::vector<AnimJointAdjustmentFrame> target_pose = SampleAnimationPose(*animation_to_play, 0);
            const float blend_tick = (float)(blend_out_time_total - blend_out_time_left);
            const float blend_t = (blend_out_time_total <= 0) ? 1.0f : (blend_tick / (float)blend_out_time_total);
            return BlendPoses(blend_start_pose, target_pose, blend_t);
        }

        if (animation_to_play != nullptr) {
            return SampleAnimationPose(*animation_to_play, tick_frame_of_animation);
        }

        return {};
    }

    void ClearBlendState () {
        blend_mode = false;
        blend_out_time_left = 0;
        blend_out_time_total = 0;
        prev_animation_blend_frame = 0;
        prev_animation_to_blend = nullptr;
        blend_start_pose.clear();
    }

    void StartBlendToAnimation (Animation* target_anim, int blend_frames, int ms_per_blend_fr = 18) {
        blend_start_pose = GetCurrentBlendPose();
        if (blend_start_pose.empty() && animation_to_play != nullptr) {
            blend_start_pose = SampleAnimationPose(*animation_to_play, tick_frame_of_animation);
        }

        blend_mode = true;
        blend_out_time_left = blend_frames;
        blend_out_time_total = blend_frames;
        ms_per_blend_frame = ms_per_blend_fr;
        prev_animation_to_blend = animation_to_play;
        prev_animation_blend_frame = tick_frame_of_animation;

        animation_to_play = target_anim;
        tick_frame_of_animation = 0;
    }


    // Set the base animation without switching
    void SetBaseAnimation (Animation* animation, int blend_frames = 0, int ms_per_blend_fr = 18) {

        base_animation = animation;

        // If we are interpolating between animations
        if (interpolate_btwn && blend_frames > 0 && animation_to_play != nullptr && animation_to_play != animation && animation != nullptr) {
            StartBlendToAnimation(animation, blend_frames, ms_per_blend_fr);
            return;
        } 

        // If we are... not interpolating between animations
        ClearBlendState();
        animation_to_play = animation;
        tick_frame_of_animation = 0;

    }

    // Clear all other animations and play the base animation, setting it a 
    // custom animation is desired
    void PlayBaseAnimation (Animation* base_anim = nullptr, int blend_frames = 0, int ms_per_blend_fr = 18) {

        // Play
        Animation* target_anim = (base_anim != nullptr) ? base_anim : base_animation;

        if (base_anim != nullptr) { 
            base_animation = base_anim; 
        }

        // If we are interpolating between animations
        if (interpolate_btwn && blend_frames > 0 && animation_to_play != nullptr && animation_to_play != target_anim && target_anim != nullptr) {
            StartBlendToAnimation(target_anim, blend_frames, ms_per_blend_fr);
            PlayAnimation();
            return;
            //--------------
        }

        if (base_animation == nullptr) {
            animation_to_play = nullptr;
            playing_animation = false;
            return;
        }
        
        animation_to_play = base_animation;

        ResetCurrentAnimation();
        PlayAnimation();
    }

    // Set the current animation tick frame to a different frame of the animation
    void SetCurrentAnimationFrame (int frame) {
        assert(frame >= 0);
        assert(frame < animation_to_play->total_tick_frame_count());
        tick_frame_of_animation = frame;
        accumulated_ms = 0;
    }
    
    // Clear all animations in queue and base animation and stop playing animations
    void ClearAllAnimation () {
        base_animation = nullptr;
        animation_to_play = nullptr;
        playing_animation = false;
        tick_frame_of_animation = 0;
        accumulated_ms = 0;
        ClearBlendState();
    }
    
    // Progress animation according to deltatime and either go forward a tick, loop, or blend accordingly
    void TickAnimation(float deltatime) {

        if (animation_to_play == nullptr && base_animation != nullptr) {
            animation_to_play = base_animation;
        }

        if (!playing_animation || animation_to_play == nullptr)
            return;

        if (animation_to_play->frames.empty())
            return;

        // Tick
        if (blend_mode) {
            accumulated_ms += (deltatime * 1000.0f);
        } else {
            accumulated_ms += (deltatime * 1000.0f * animation_speed);
        }



        if (blend_mode) {

            while (accumulated_ms >= ms_per_blend_frame) {

                // Get rid of overflow and progress (but for blend state... basically calculate how many blend acculations = how many frames to change)
                accumulated_ms -= ms_per_blend_frame;
                
                blend_out_time_left--;

                if (blend_out_time_left <= 0) {
                    ClearBlendState();
                    tick_frame_of_animation = 0;
                }

            }

        } else {
            
            // Normal animation tick mode

            while (accumulated_ms >= animation_to_play->ms_per_tick_frame) {

                // Get rid of overflow and progress
                accumulated_ms -= animation_to_play->ms_per_tick_frame;
            
                tick_frame_of_animation += 1;
                const int totalTicks = animation_to_play->total_tick_frame_count();
                
                if (tick_frame_of_animation >= totalTicks) {

                    // If we are currently animating the base animation
                    if (animation_to_play == base_animation) {

                        if (loop_base_animation) {
                            // Loop base animation
                            ResetCurrentAnimation();
                            break;
                        } /*else {
                            // Set tick frame to 0 and stop animation
                            tick_frame_of_animation = 0;
                            accumulated_ms = 0.0f;
                            PauseAnimation();
                        }*/
                    }

                }

            }




        }




    }

    // Attach avatar to the renderer
    void ConnectAvatar (Avatar* avatar, bool stop_animation = true) {
        if (stop_animation) {
            ClearAllAnimation();
        }
        avatar_to_render = avatar;
    }

};








}


#endif