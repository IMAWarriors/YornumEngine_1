// PlayerAnimationSystem.h

#ifndef PLAYERANIMATIONSYSTEM_H
#define PLAYERANIMATIONSYSTEM_H

// Get System
#include "../../Engine/ECS/System.h"

// Get Entity
#include "../../Engine/ECS/Entity.h"

// Get Specific Components
#include "../Components/Components.h"

//
#include <cmath>


class PlayerAnimationSystem : public System {

    private:

    
        // Best way to store animations and avatars per animation and avatar?
        

    public: 

        
    
        void update (Registry & registry, float deltatime) override {

            for (Entity entity : registry.view<comp::InputState, comp::Velocity, comp::AvatarRenderer, tag::Player, comp::AnimationRepertoire, comp::PlayerConfig>()) {

                // Player animation variables
                comp::InputState & input = registry.get_component<comp::InputState>(entity);
                comp::PhysicsBody & body = registry.get_component<comp::PhysicsBody>(entity);
                comp::Velocity & player_velocity = registry.get_component<comp::Velocity>(entity);
                comp::AvatarRenderer & animation_handler = registry.get_component<comp::AvatarRenderer>(entity);
                comp::PlayerConfig & config = registry.get_component<comp::PlayerConfig>(entity);
                comp::AnimationRepertoire & animation_repertoire = registry.get_component<comp::AnimationRepertoire>(entity);

                // Helper function to set animation state
                auto SetBaseAnimationState = [&](const std::string& new_state, int trans_frames, bool reverse_mirror = false){
                    if (reverse_mirror)
                        animation_handler.mirror = !animation_handler.mirror;
                    const std::string set_state = new_state;
                    if (animation_handler.animation_state != set_state) {
                        animation_handler.PlayBaseAnimation(animation_repertoire.repertoire[set_state], trans_frames);
                        animation_handler.animation_state = set_state;
                    }   
                };
                
                // Reset base animation settings for display
                animation_handler.animation_speed = 1.0f;
                animation_handler.mirror = (body.direction == 1) ? false : true;
                
                // Handle animation state switching
                if (body.onSolidGround == true) {

                    // If Player Velocity (abs) is > Speed Tolerance 50
                    if (std::abs(player_velocity.magnitude.x) >= 50.0f) {

                        // Animate walk animation speed according to percentage of max speed reached
                        float percentage_max_speed_x = std::abs(player_velocity.magnitude.x) / (config.NatRunSpeed);
                        float speed_factor = percentage_max_speed_x * 0.75f;
                        animation_handler.animation_speed = 0.4f + speed_factor;

                        // Set Walk Animation
                        SetBaseAnimationState("Walk", 6);

                    } else {
                        // If Player Velocity is around 0, 
                        // Set Idle Animation
                        SetBaseAnimationState("Idle", 4);
                    }

                } else {

                    // If the Player is Jumping, moving negatively in the Y direction
                    if (player_velocity.magnitude.y < 0.0f) {

                        // Set Jump Animation
                        SetBaseAnimationState("Jump", 6);
                        
                    } else {

                        if (body.againstWall)
                            SetBaseAnimationState("Wallslide", 5, true);     // Set Wallslide Animation
                        else 
                            SetBaseAnimationState("Fall", 8);                // Set Falling Animation
                           
                    }
                }
            
            }
        }
};



#endif

