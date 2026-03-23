// DebugOverlaySystem.cpp

#include "DebugOverlaySystem.h"



void DebugOverlaySystem::update (Registry & registry, float deltatime)  {

    /*
    renderer.rdraw_text(renderer.text("FPS: %d", GetFPS()), 10, 10, 20, GREEN);
    renderer.rdraw_text(renderer.text("Frame dt: %.4f", debug_info.frame_deltatime), 10, 40, 20, WHITE);
    renderer.rdraw_text(renderer.text("Physics steps: %d", debug_info.frame_simulation_ticks), 10, 70, 20, WHITE);
    renderer.rdraw_text(renderer.text("Accumulator: %.4f", debug_info.frame_accumulator), 10, 100, 20, WHITE);

    
    */

    for (auto port : debug_info.float_ports) {
        if (port.rounding == 0) {
            renderer.rdraw_text(renderer.text("%s: %.0f", port.label.c_str(), *(port.data)), (int)(port.x), (int)(port.y), 16, WHITE);
        } else if (port.rounding == 4) {
            renderer.rdraw_text(renderer.text("%s: %.4f", port.label.c_str(), *(port.data)), (int)(port.x), (int)(port.y), 16, WHITE);
        } 
    }
    

}















