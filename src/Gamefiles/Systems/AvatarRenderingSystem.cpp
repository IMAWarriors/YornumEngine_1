// AvatarRenderingSystem.cpp

#include "AvatarRenderingSystem.h"

void AvatarRenderingSystem::update (Registry & registry, float deltatime) {

    for (Entity entity : registry.view<comp::Transform, comp::AvatarRenderer>()) {

        comp::AvatarRenderer& avatar_renderer = registry.get_component<comp::AvatarRenderer>(entity);
        comp::Transform& anchor_position = registry.get_component<comp::Transform>(entity);

        bool renderer_has_avatar = (avatar_renderer.avatar_to_render != nullptr);
        bool avatar_has_animation = (avatar_renderer.animation_to_play != nullptr);

        if (renderer_has_avatar) {
            Avatar& avatar_source = *avatar_renderer.avatar_to_render;

            if (avatar_has_animation) {
                avatar_source.DrawAvatar(renderer, anchor_position.position, 0.0f, {1.0f, 1.0f}, *avatar_renderer.animation_to_play, avatar_renderer.tick_frame_of_animation);
                avatar_renderer.TickAnimation(deltatime);
            } else {
                

            }
        }

    }

}