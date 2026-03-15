// config.h

#ifndef CONFIG_H
#define CONFIG_H

namespace config {

    // Constants
    constexpr int GAME_WORLD_WIDTH  = 1280;      // Width
    constexpr int GAME_WORLD_HEIGHT =  720;      // Height

    constexpr int INIT_WINDOW_DISPLAY_WIDTH  = 640;
    constexpr int INIT_WINDOW_DISPLAY_HEIGHT = 360;

    constexpr float GAME_WORLD_FPS    =   60.0f;
    constexpr float FIXED_DELTATIME   =   1.0f / (GAME_WORLD_FPS);
    constexpr float MAX_FRAME_LAG     =   0.25f;
    


}



#endif 