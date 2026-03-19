// RenderSystem.cpp

#include "RenderSystem.h"
#include "../../Engine/Core/Rendering/Renderer.h"



void RenderSystem::update (Registry & registry, float deltatime) {


    // "DELTATIME" is technically i guess an alpha here that we have to use to interpolate

    // Set internal renderer camera position according to interpolation bullshit
    for (Entity entity : registry.view<comp::Camera, comp::Transform>()) {

        comp::Transform& transform = registry.get_component<comp::Transform>(entity);

        Vec2 camera_interp = {
            transform.previous_position.x + (transform.position.x - transform.previous_position.x) * deltatime,
            transform.previous_position.y + (transform.position.y - transform.previous_position.y) * deltatime
        };

        renderer.set_camera_position(camera_interp);

        break; // assume 1 camera
    }



    // Draw all Circles

    for (Entity entity : registry.view<comp::Transform, comp::CircleRenderer>()) {      // For each iteration of Entity


            comp::Transform & transform     = registry.get_component<comp::Transform>(entity);
            comp::CircleRenderer & circle = registry.get_component<comp::CircleRenderer>(entity);


            unsigned char _r = circle.color.r;
            unsigned char _g = circle.color.g;
            unsigned char _b = circle.color.b;
            unsigned char _a = circle.color.a;


            Color col = Color({_r, _g, _b, _a});

            Vec2 interpolation = {transform.previous_position.x + ((transform.position.x-transform.previous_position.x) * deltatime), transform.previous_position.y + ((transform.position.y-transform.previous_position.y) * deltatime)};

            renderer.rdraw_circle(interpolation.x, interpolation.y, circle.radius, col);


    }

}


