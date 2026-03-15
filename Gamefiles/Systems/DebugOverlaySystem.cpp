// DebugOverlaySystem.cpp

#include "DebugOverlaySystem.h"



void DebugOverlaySystem::update (Registry & registry, float deltatime)  {

    renderer.rdraw_text(renderer.text("FPS: %d", GetFPS()), 10, 10, 20, GREEN);
    renderer.rdraw_text(renderer.text("Frame dt: %.4f", debug_info.frame_deltatime), 10, 40, 20, WHITE);
    renderer.rdraw_text(renderer.text("Physics steps: %d", debug_info.frame_simulation_ticks), 10, 70, 20, WHITE);
    renderer.rdraw_text(renderer.text("Accumulator: %.4f", debug_info.frame_accumulator), 10, 100, 20, WHITE);
    

}















