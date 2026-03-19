// RenderSystem.cpp

#include "RenderSystem.h"
#include "../../Engine/Core/Rendering/Renderer.h"



void RenderSystem::update (Registry & registry, float deltatime) {



    for (Entity entity : registry.view<comp::Transform, comp::CircleRenderer>()) {      // For each iteration of Entity


            comp::Transform & transform     = registry.get_component<comp::Transform>(entity);
            comp::CircleRenderer & circle = registry.get_component<comp::CircleRenderer>(entity);


            unsigned char _r = circle.color.r;
            unsigned char _g = circle.color.g;
            unsigned char _b = circle.color.b;
            unsigned char _a = circle.color.a;


            Color col = Color({_r, _g, _b, _a});
            
            renderer.rdraw_circle(transform.position.x, transform.position.y, circle.radius, col);


    }

}