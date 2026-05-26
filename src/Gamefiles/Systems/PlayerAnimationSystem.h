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

                comp::InputState & input = registry.get_component<comp::InputState>(entity);
                comp::PhysicsBody & body = registry.get_component<comp::PhysicsBody>(entity);
                comp::Velocity & player_velocity = registry.get_component<comp::Velocity>(entity);
                comp::AvatarRenderer & animation_handler = registry.get_component<comp::AvatarRenderer>(entity);

                comp::PlayerConfig & config = registry.get_component<comp::PlayerConfig>(entity);
                comp::AnimationRepertoire & animation_repertoire = registry.get_component<comp::AnimationRepertoire>(entity);
                
                // Set the mirror handler
                animation_handler.mirror = (body.direction == 1) ? false : true;
                
                if (body.onSolidGround == true) {

                    if (std::abs(player_velocity.magnitude.x) >= 50.0f) {
                        // Set the play animation to be the animation
                        // in the assets folder represented by
                        // --> BRHumanoid_Walk

                        float percentage_max_speed_x = std::abs(player_velocity.magnitude.x) / (config.NatRunSpeed);
                        float speed_factor = percentage_max_speed_x * 0.75f;

                        animation_handler.animation_speed = 0.4f + speed_factor;

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
                        animation_handler.animation_speed = 1.0f;

                        if (animation_handler.animation_state != set_state) {
                            animation_handler.PlayBaseAnimation(animation_repertoire.repertoire[set_state]);
                            animation_handler.animation_state = set_state;
                        }
                    }

                } else {

                    if (player_velocity.magnitude.y < 0.0f) {
                        // Jumping, moving negatively Y
                        const std::string set_state = "Jump";

                        if (animation_handler.animation_state != set_state) {
                            animation_handler.PlayBaseAnimation(animation_repertoire.repertoire[set_state]);
                            animation_handler.animation_state = set_state;
                        }
                
                    } else {

                        if (body.againstWall) {
                            // Wall sliding and also do opposite mirror type
                            animation_handler.mirror = !animation_handler.mirror;
                            const std::string set_state = "Wallslide";

                            if (animation_handler.animation_state != set_state) {
                                animation_handler.PlayBaseAnimation(animation_repertoire.repertoire[set_state]);
                                animation_handler.animation_state = set_state;
                            }

                        } else {
                            // Jumping, moving positively Y
                            const std::string set_state = "Fall";

                            if (animation_handler.animation_state != set_state) {
                                animation_handler.PlayBaseAnimation(animation_repertoire.repertoire[set_state]);
                                animation_handler.animation_state = set_state;
                            }         
                           
                        }
                    }
                    
                }



            }

        

        }



};



#endif

