// DebugOverlaySystem.h

#ifndef DEBUGOVERLAYSYSTEM_H
#define DEBUGOVERLAYSYSTEM_H

// Get System
#include "../../Engine/ECS/System.h"

// Get Entity
#include "../../Engine/ECS/Entity.h"

// Get Specific Components
#include "../Components/Components.h"

#include "../../Engine/Core/Rendering/Renderer.h"
#include "../../Engine/Core/Overhead/FrameStats.h"
#include "../../Tooling/Debug/DebugManager.h"

#include <string>


class DebugOverlaySystem : public System {

    private:

        Renderer & renderer;
        FrameStats & frame;

    public: 

        DebugOverlaySystem (Renderer & _renderer, FrameStats & _frame) : renderer(_renderer), frame(_frame) {}
        
        void update (Registry & registry, float deltatime) override;

        //void draw_text (const std::string & text, Vec2 position, int size, ColorRGBA color);

};

















#endif
