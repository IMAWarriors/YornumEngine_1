// MovementSystem.h

#ifndef MOVEMENTSYSTEM_H
#define MOVEMENTSYSTEM_H

// Get System
#include "../../Engine/ECS/System.h"

// Get Entity
#include "../../Engine/ECS/Entity.h"

// Get Specific Components
#include "../Components/Components.h"


class MovementSystem : public System {

    public: 
    
        void update (Registry & registry, float deltatime) override {


           for (Entity entity : registry.view<comp::Transform, comp::Velocity>()) {      // For each iteration of Entity


                // To get to this point, we have confirmed this entity has both VELOCITY and TRANSFORM, so should be valid to get component
                // Go ahead and grab struct references to the components we want to manipulate so we can edit them directly

                comp::Transform & transform = registry.get_component<comp::Transform>(entity);
                comp::Velocity & velocity = registry.get_component<comp::Velocity>(entity);
                
                // Transform

                transform.position.x = transform.position.x + (velocity.magnitude.x * deltatime);
                transform.position.y = transform.position.y + (velocity.magnitude.y * deltatime);


           }

        }

    


};

















#endif
