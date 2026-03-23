// WindowStats.h

#ifndef WINDOWSTATS_H
#define WINDOWSTATS_H

#include "raylib.h"

#include "Config.h"
#include "GameTypes.h"

namespace winstats {

    inline Vec2 ScreenMousePosition () {

        float mouse_position_x = GetMousePosition().x;
        float mouse_position_y = GetMousePosition().y;

        // normalize mouse to your game resolution
        float scaleX = (float)config::GAME_WORLD_WIDTH  / GetScreenWidth();
        float scaleY = (float)config::GAME_WORLD_HEIGHT / GetScreenHeight();

        float adjustedMouseX = mouse_position_x * scaleX;
        float adjustedMouseY = mouse_position_y * scaleY;

        Vec2 mouse_point = {adjustedMouseX, adjustedMouseY};

        return mouse_point;

    }

}



#endif
