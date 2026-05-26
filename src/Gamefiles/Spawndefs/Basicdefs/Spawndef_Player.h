// Spawndef_Player.h

#ifndef SPAWNDEF_PLAYER_H
#define SPAWNDEF_PLAYER_H

#include "../../../Engine/ECS/Registry.h"
#include "../../../Gamefiles/Components/Components.h"

#include "../../../Gamefiles/Assets/AssetManager.h"


namespace spawndef {

    inline Entity SpawnPlayer (Registry & registry, AssetManager& assets, Vec2 position = {200.0f, -1080.0f}) {

        Entity entity = registry.create_entity();

        // Basic tracking
        registry.apply_component<tag::Player>           (entity, {});
        registry.apply_component<comp::Transform>       (entity,  { position, position, 0.0f, {1.0f, 1.0f} });
        registry.apply_component<comp::Velocity>        (entity, {0.0f, 0.0f} );
        registry.apply_component<comp::InputState>      (entity, {0, false});

        // Physics body setup
        registry.apply_component<comp::PhysicsBody> (entity, 
        {
            {28.0f, 84.0f},     // Hitbox size, (x,y)
            1.0f,               //
            true,               //
            true,               //
            4000.0f,            //
            true,               //
            false,              //
            false,              //
            false,              //
            0,                  //
            0,                  //
            0,                  //
            false,              //
            0,                  //
            0,                  //
            0,                  //
            1                   //
        });

        // Avatar
        registry.apply_component<comp::AvatarRenderer>      (entity, comp::AvatarRenderer());
        registry.apply_component<comp::AnimationRepertoire> (entity, comp::AnimationRepertoire());
        registry.apply_component<comp::PlayerConfig>        (entity, {});
        
        comp::PlayerConfig& config = registry.get_component<comp::PlayerConfig>(entity);
        config.NatRunSpeed     = 650.0f;
        config.NatRunAccel     = 1500.0f;
        config.NatRunFriction  = 2700.0f;
        config.NatJumpForce    = 1150.0f;

        // Load entity animations
        Avatar* avatar = assets.LoadAvatarAsset("assets/avatars/24JSS_template.avr");

        comp::AnimationRepertoire& animationHandler = registry.get_component<comp::AnimationRepertoire>(entity);
        comp::AvatarRenderer& avatarRenderer = registry.get_component<comp::AvatarRenderer>(entity);

        // Load animations into Animation Repertoire
        animationHandler.ImportAnimationAsset("Idle", assets.LoadAnimationAsset("assets/animations/24JSS_IdleCalm.anim"));
        animationHandler.ImportAnimationAsset("Walk", assets.LoadAnimationAsset("assets/animations/24JSS_WalkCalm.anim"));
        animationHandler.ImportAnimationAsset("Jump", assets.LoadAnimationAsset("assets/animations/24JSS_JumpNormal.anim"));
        animationHandler.ImportAnimationAsset("Fall", assets.LoadAnimationAsset("assets/animations/24JSS_FallNormal.anim"));
        animationHandler.ImportAnimationAsset("Wallslide", assets.LoadAnimationAsset("assets/animations/24JSS_WallslideNormal.anim"));

        // Set up the Avatar Renderer
        avatarRenderer.ConnectTransform(&registry.get_component<comp::Transform>(entity));
        avatarRenderer.ConnectAvatar(avatar);
        avatarRenderer.SetTransformOffset({-64.0f, 44.0f});
        avatarRenderer.SetMirrorOffsetX(128);
        avatarRenderer.SetBaseAnimation(animationHandler.repertoire["Idle"]);
        avatarRenderer.PlayBaseAnimation();

        // Return entity
        return entity;
    }






}




#endif



