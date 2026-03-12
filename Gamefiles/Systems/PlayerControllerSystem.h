// PlayerControllerSystem.h

#ifndef PLAYERCONTROLLERSYSTEM_H
#define PLAYERCONTROLLERSYSTEM_H

// Get System
#include "../../Engine/ECS/System.h"

// Get Entity
#include "../../Engine/ECS/Entity.h"

// Get Specific Components
#include "../Components/Components.h"

//
#include <cmath>



class PlayerControllerSystem : public System {

    public: 
    
        void update (Registry & registry, float deltatime) override {

            for (Entity entity : registry.view<comp::InputState, comp::Velocity, tag::Player>()) {

                comp::InputState input = registry.get_component<comp::InputState>(entity);

                comp::Velocity & player_velocity = registry.get_component<comp::Velocity>(entity);


                float MAX_VELOCITY_X    = 900.0f;
                float ACCELERATION      = 2100.0f;
                float FRICTION          = 2900.0f;

                float target_player_velocity_x = 0.0f;


                target_player_velocity_x = MAX_VELOCITY_X * input.horz_axis;
           
                if (std::abs(target_player_velocity_x) > 0) {
                    if ( (target_player_velocity_x > 0 && player_velocity.magnitude.x < 0) || (target_player_velocity_x < 0 && player_velocity.magnitude.x > 0) ) {
                        // Handle quick turn around for movement option with most velocity
                        // so it doesn't make more sense to let friciton take you to 0 first
                        // and then go other direction
                        player_velocity.magnitude.x += input.horz_axis * ( fmax(FRICTION, ACCELERATION) * deltatime);
                    } else {
                        player_velocity.magnitude.x += input.horz_axis * (ACCELERATION * deltatime);
                    }

                } else {

                    if (player_velocity.magnitude.x > 0) {

                        player_velocity.magnitude.x -= FRICTION * deltatime;
                        if (player_velocity.magnitude.x <= 0) {
                            player_velocity.magnitude.x = 0.0f;
                        }

                    } else if (player_velocity.magnitude.x < 0) {

                        player_velocity.magnitude.x += FRICTION * deltatime;
                        if (player_velocity.magnitude.x >= 0) {
                            player_velocity.magnitude.x = 0.0f;
                        }

                    }

                }
                
                if (player_velocity.magnitude.x > MAX_VELOCITY_X)  {    player_velocity.magnitude.x =  MAX_VELOCITY_X; }
                if (player_velocity.magnitude.x < -MAX_VELOCITY_X) {    player_velocity.magnitude.x = -MAX_VELOCITY_X; }
    

                // Velocity should be updated at this point
                
                

            }

        

        }



};



#endif

