// Spawndef_Player.h

#ifndef SPAWNDEF_PLAYER_H
#define SPAWNDEF_PLAYER_H

#include "../../../Engine/ECS/Registry.h"
#include "../../../Gamefiles/Components/Components.h"


namespace spawndef {

    inline Entity SpawnPlayer (Registry & registry,  Vec2 position = {200.0f, -1080.0f}) {

        Entity entity = registry.create_entity();

        registry.apply_component<tag::Player>           (entity, {});

        registry.apply_component<comp::Transform>       (entity,  { position, position, 0.0f, {1.0f, 1.0f} });
        registry.apply_component<comp::Velocity>        (entity, {0.0f, 0.0f} );

        /* PHYSICS BODY
        Vec2 size;
        float skin;
        bool solid;
        bool gravitous;
        float gravity;
        bool render_hitbox;
        */

        registry.apply_component<comp::PhysicsBody>     (entity, {{32.0f, 64.0f}, 0.25f, true, true, 3400.0f, true});

        // registry.apply_component<comp::CircleRenderer>  (entity, {50.0f , { 255, 0 , 0 , 255 } });
        registry.apply_component<comp::InputState>      (entity, {0, false});

        return entity;

    }






}




#endif



