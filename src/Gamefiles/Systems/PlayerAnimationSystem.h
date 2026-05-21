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

            for (Entity entity : registry.view<comp::InputState, comp::Velocity, comp::AvatarRenderer, tag::Player, comp::AnimationRepertoire>()) {


                
                comp::InputState & input = registry.get_component<comp::InputState>(entity);
                comp::PhysicsBody & body = registry.get_component<comp::PhysicsBody>(entity);
                comp::Velocity & player_velocity = registry.get_component<comp::Velocity>(entity);
                comp::AvatarRenderer & animation_handler = registry.get_component<comp::AvatarRenderer>(entity);

                comp::AnimationRepertoire & animation_repertoire = registry.get_component<comp::AnimationRepertoire>(entity);
                
                if (body.direction == 1) {
                    animation_handler.mirror = false;
                } else if (body.direction == -1) {
                    animation_handler.mirror = true;
                }

                if (std::abs(player_velocity.magnitude.x) >= 0.5f) {
                    // Set the play animation to be the animation
                    // in the assets folder represented by
                    // --> BRHumanoid_Walk

                    const std::string set_state = "Walk";

                    if (animation_handler.animation_state != set_state) {
                        animation_handler.PlayBaseAnimation(animation_repertoire.repertoire[set_state]);
                        animation_handler.animation_state = set_state;
                    }
                    

                } else {
                    // Otherwise set to default idle animation
                    // in the assets folder represented by
                    // --> BRHumanoid_Idle

                    const std::string set_state = "Idle";

                    if (animation_handler.animation_state != set_state) {
                        animation_handler.PlayBaseAnimation(animation_repertoire.repertoire[set_state]);
                        animation_handler.animation_state = set_state;
                    }

                }

                




            }

        

        }



};



#endif

