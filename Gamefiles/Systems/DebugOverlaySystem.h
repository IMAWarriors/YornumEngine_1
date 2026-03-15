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
#include "../../Engine/Core/Overhead/DebugStats.h"


class DebugOverlaySystem : public System {

    private:

        Renderer & renderer;

        DebugStats & debug_info;

    public: 

        DebugOverlaySystem (Renderer & _renderer, DebugStats & debug) : renderer(_renderer), debug_info(debug) {}
        
    
        void update (Registry & registry, float deltatime) override;

    


};

















#endif
