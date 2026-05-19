// Spawndef_Player.h

#ifndef SPAWNDEF_PLAYER_H
#define SPAWNDEF_PLAYER_H

#include "../../../Engine/ECS/Registry.h"
#include "../../../Gamefiles/Components/Components.h"

#include "../../../Gamefiles/Assets/AssetManager.h"


namespace spawndef {

    inline Entity SpawnPlayer (Registry & registry, AssetManager& assets, Vec2 position = {200.0f, -1080.0f}) {

        Entity entity = registry.create_entity();

        registry.apply_component<tag::Player>           (entity, {});

        registry.apply_component<comp::Transform>       (entity,  { position, position, 0.0f, {1.0f, 1.0f} });
        registry.apply_component<comp::Velocity>        (entity, {0.0f, 0.0f} );



        registry.apply_component<comp::PhysicsBody>     (entity, {{28.0f, 50.0f}, 1.0f, true, true, 4000.0f, true, false, false, false, 0, 0, 0, false, 0, 0, 0, 1});

        registry.apply_component<comp::AvatarRenderer>(entity, comp::AvatarRenderer());

        // registry.apply_component<comp::CircleRenderer>  (entity, {50.0f , { 255, 0 , 0 , 255 } });
        registry.apply_component<comp::InputState>      (entity, {0, false});

        // Need to load in an avatar and an animation somehow by adding loading system
        // into asset manager?
        // Just want to load a basic character with idle animation 
        // BRHumanoid_template.avr and BRHumanoid_Idle.anim, dont know best way
        // to set up this system, i want it to be fast and efficient

        Avatar* avatar = assets.LoadAvatarAsset("assets/avatars/BRHumanoid_template.avr");
        Animation* idleAnimation = assets.LoadAnimationAsset("assets/animations/BRHumanoid_Idle.anim");

        comp::AvatarRenderer& avatarRenderer = registry.get_component<comp::AvatarRenderer>(entity);
        avatarRenderer.ConnectTransform(&registry.get_component<comp::Transform>(entity));
        avatarRenderer.ConnectAvatar(avatar);
        avatarRenderer.PlayBaseAnimation(idleAnimation);

        return entity;

    }






}




#endif



