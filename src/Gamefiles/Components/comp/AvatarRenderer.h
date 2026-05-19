// AvatarRenderer.h

#ifndef AVATARRENDERER_H
#define AVATARRENDERER_H


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
    bool visible = true;

    // Priority animation controls
    bool playing_animation = false;
    bool loop_base_animation = true;
    float animation_speed = 1.0f;
    bool interpolate_btwn = true; // not even dealing with this shit right now, just interpolate everything

    // Current playing-animation controls
    Animation* animation_to_play = nullptr;
    Animation* prev_animation_to_blend = nullptr;
    int prev_animation_blend_frame = 0;
    int blend_out_time_left = 0; // If 0, blending is complete
    int tick_frame_of_animation = 0;


    // Animation that is played by default if nothing is queued
    // or after the queue has been cleared/fully executed.
    Animation* base_animation = nullptr;
    
    // Time bucket
    int accumulated_ms = 0; // += (float)(1.0f / INTENDED_FPS) * (1000.0f);
                            // += (float)(deltatime) * (1000.0f); (deltatime is measured in... seconds?)


    std::deque<AnimQueue> animation_queue;

    // Default constructor
    AvatarRenderer () {
        
    }

    // Constructor
    AvatarRenderer (Avatar& avatar, Animation& animation) {
        avatar_to_render = &avatar;
        base_animation = &animation;
        playing_animation = true;
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

        tick_frame_of_animation = 0;
        accumulated_ms = 0;
    }

    // Set the base animation without switching
    void SetBaseAnimation (Animation* animation) {
        base_animation = animation;
    }

    // Clear all other animations and play the base animation, setting it a 
    // custom animation is desired
    void PlayBaseAnimation (Animation* base_anim = nullptr) {

        if (base_anim != nullptr) {
            base_animation = base_anim;
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
    }
    
    // Progress animation according to deltatime and either go forward a tick, loop, or blend accordingly
    void TickAnimation(float deltatime) {

        if (!playing_animation || animation_to_play == nullptr)
            return;

        // Tick
        accumulated_ms += (int)(deltatime * 1000.0f * animation_speed);

        if (accumulated_ms >= animation_to_play->ms_per_tick_frame) {

            // Get rid of overflow and progress
            accumulated_ms -= animation_to_play->ms_per_tick_frame;
            tick_frame_of_animation += 1;

            if (animation_to_play == base_animation && loop_base_animation) {

                if (tick_frame_of_animation >= animation_to_play->total_tick_frame_count()) {
                    ResetCurrentAnimation();
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