// Gwconst.h

#ifndef GWCONST_H
#define GWCONST_H

#include "../../../Engine/Core/Overhead/Config.h"

namespace gwconst {

    const int WORLD_TILEGRID_X_BOUND_MIN_TILE = -320; // Range of X is  { -320, 0, 320 }, all inclusive, but the 0 is used for segmenting
    const int WORLD_TILEGRID_X_BOUND_MAX_TILE =  320; //        The result of this is going to be that there will actually be 641 tiles with this example,
                                                      //        and the reason is because the "extra odd number" is actually just the 0 row between
                                                      //        the inclusive allowed rows of tiles -320 and 320

    const int WORLD_TILEGRID_Y_BOUND_MIN_TILE = -180; // Range of Y is... I'm not fucking writing that bullshit again you get the fucking idea.
    const int WORLD_TILEGRID_Y_BOUND_MAX_TILE =  180;

    const int WORLD_TILEGRID_WIDTH  =   (WORLD_TILEGRID_X_BOUND_MAX_TILE - WORLD_TILEGRID_X_BOUND_MIN_TILE) + 1;    // 641 tile rows total (-320, 320 inclusive)
    const int WORLD_TILEGRID_HEIGHT =   (WORLD_TILEGRID_Y_BOUND_MAX_TILE - WORLD_TILEGRID_Y_BOUND_MIN_TILE) + 1;    // 361 tile rows total (-180, 180 inclusive)

    const int SCREEN_WIDTH_GAMEPIXELS = config::GAME_WORLD_WIDTH;
    const int SCREEN_HEIGHT_GAMEPIXELS = config::GAME_WORLD_HEIGHT;

    const int SCREEN_BASE_TILESIZE_GAMEPIXELS = 64;

}


#endif
