// PlayerControllerSystem.h

#ifndef PLAYERCONTROLLERSYSTEM_H
#define PLAYERCONTROLLERSYSTEM_H

// Get System
#include "../../Engine/ECS/System.h"

// Get Entity
#include "../../Engine/ECS/Entity.h"

// Get Specific Components
#include "../Components/Components.h"

#include "../Spawndefs/Spawndefs.h"

//
#include <cmath>

static float approach_x (float init_vel_x, float target_vel_x, float step_vel_x) {

    float change_x = 0.0f;

    if (init_vel_x < target_vel_x) {        // Moving to the right
        
        change_x = std::min(target_vel_x, init_vel_x + step_vel_x);

    } else {

        change_x = std::max(target_vel_x, init_vel_x - step_vel_x);

    }

    return change_x;

}


class PlayerControllerSystem : public System {

    public: 
    
        void update (Registry & registry, float deltatime) override {

            for (Entity entity : registry.view<comp::InputState, comp::Transform, comp::Velocity, tag::Player, comp::PlayerConfig>()) {

                comp::Transform & transform = registry.get_component<comp::Transform>(entity);
                comp::InputState & input = registry.get_component<comp::InputState>(entity);
                comp::PhysicsBody & body = registry.get_component<comp::PhysicsBody>(entity);
                comp::Velocity & player_velocity = registry.get_component<comp::Velocity>(entity);
                comp::PlayerConfig & config = registry.get_component<comp::PlayerConfig>(entity);
                comp::BodyAttackState & attack_state = registry.get_component<comp::BodyAttackState>(entity);

                

                float MAX_VELOCITY_X    = config.NatRunSpeed;
                float ACCELERATION      = config.NatRunAccel;
                float FRICTION          = config.NatRunFriction;
                float JUMP_FORCE        = config.NatJumpForce;

                float target = input.horz_axis * MAX_VELOCITY_X;
                float control_multiplier = body.onSolidGround ? 1.0f : 0.75f;

                /*
                if (player_velocity.magnitude.x > 0.5f) {
                    body.direction = 1;
                } else if (player_velocity.magnitude.x < -0.5f) {
                    body.direction = -1;
                }
                */

            
                if (body.walljumpBuffer <= 0) {
                    if (input.horz_axis == 1) {
                        body.direction = 1;
                    } else if (input.horz_axis == -1) {
                        body.direction = -1;
                    }
                }
                
                if (body.walljumpBuffer > 0) {

                    body.walljumpBuffer--;

                    if (body.walljumpBuffer < 0) {
                        body.walljumpBuffer = 0;
                    }

                    if (input.horz_axis == body.lastWalljumpDir) {
                        body.walljumpBuffer = 0;
                    } else {
                        control_multiplier = 0.5f;
                    }
                    
                }

                float accel = ACCELERATION * control_multiplier;
                float decel = FRICTION * control_multiplier;

                if (input.horz_axis == 0) {

                    player_velocity.magnitude.x = approach_x(player_velocity.magnitude.x, 0.0f, decel * deltatime);

                } else {

                    // Handle turning around when INTENDED DIRECTION and MOVING DIRECTION do not match
                    if (player_velocity.magnitude.x * input.horz_axis < 0) {
                        accel = std::max(ACCELERATION, FRICTION);
                    }

                    // Actually move 
                    if (attack_state.attacking) {
                        target = 0.0f;
                    }

                    player_velocity.magnitude.x = approach_x(player_velocity.magnitude.x, target, accel * deltatime);
                }
                
                if (player_velocity.magnitude.x > MAX_VELOCITY_X)  {    player_velocity.magnitude.x =  MAX_VELOCITY_X; }
                if (player_velocity.magnitude.x < -MAX_VELOCITY_X) {    player_velocity.magnitude.x = -MAX_VELOCITY_X; }

                // Velocity should be updated at this point
                
                // Normal Jump
                if ((0 < input.jump_key && input.jump_key < 20) && (body.falling < 9)) {
                    player_velocity.magnitude.y = -JUMP_FORCE;
                    body.vjump_window = 10;
                    input.jump_key = 20;
                    body.falling += 21;
                }

                // Variable Jump Height (continue jumping if vjump window is open)
                if ((input.jump_key > 0) && (body.vjump_window > 0)) {
                    player_velocity.magnitude.y = std::min(-JUMP_FORCE, player_velocity.magnitude.y);
                }

                // Clamp / Iterate jump values
                if (input.jump_key <= 0) {
                    body.vjump_window = 0;
                } 

                if (body.vjump_window > 0) {
                    body.vjump_window--;
                }

                // Wall Jump
                if (input.jump_key == 1 && body.falling > 5 && body.againstWall == true) {
                    player_velocity.magnitude.x = JUMP_FORCE * 2.0f * (body.lastWallPush * -1.0f);
                    player_velocity.magnitude.y = -JUMP_FORCE * 1.35f;
                    input.jump_key = 20;
                    body.walljumpBuffer = 30;
                    body.lastWalljumpDir = -body.lastWallPush;
                    body.direction = (player_velocity.magnitude.x > 0.0f) ? 1 : -1;
                }



                // Attack?
                if (input.attack_key_tapped && !attack_state.attacking) {
                    bool mirror_attack_direction = (body.direction == -1);
                    attack_state.BeginAttack(hbpos::STANDARD_SWING);
                    spawndef::SpawnAttack(registry, {0.0f, -40.0f}, hbpos::STANDARD_SWING, entity, mirror_attack_direction);
                }
                
                

            }

        

        }



};



#endif

