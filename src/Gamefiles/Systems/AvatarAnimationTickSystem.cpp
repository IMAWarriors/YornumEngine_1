// AvatarAnimationTickSystem.cpp

#include "AvatarAnimationTickSystem.h"

void AvatarAnimationTickSystem::update (Registry & registry, float deltatime) {

    for (Entity entity : registry.view<comp::Transform, comp::AvatarRenderer>()) {

        comp::AvatarRenderer& avatar_renderer = registry.get_component<comp::AvatarRenderer>(entity);
        comp::Transform& anchor_position = registry.get_component<comp::Transform>(entity);

        bool renderer_has_avatar = (avatar_renderer.avatar_to_render != nullptr);
        bool avatar_has_animation = (avatar_renderer.animation_to_play != nullptr);

        if (renderer_has_avatar) {
            Avatar& avatar_source = *avatar_renderer.avatar_to_render;

            if (avatar_has_animation) {
                
                avatar_renderer.TickAnimation(deltatime);
                
            } else {
                

            }
        }

    }

}