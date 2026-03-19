// main.cpp

/**    =======================================================================================================
 *      > MAIN FILE, Version E-Build Alpha 1.0.0
 *      > by IMAWarriors
 *      --------------------------------------
 *      > Stable working version pushed to Git on 03-19-2029, 2:00 AM 
 * 
 *      >>> Most Recently Added Features
 *       - Stabilized Camera and Render interpolation
 *       - Added debug basic overlay
 *       - Split engine and application dependencies
 * 
 *      >>> Working-in-Progress Features (as of this push, goals/objectives)
 *       - Add queue for consistnent entity / component management WITHOUT view reading sequentially messing things up
 *       - Tilemap...
 * 
 * 
 *    ======================================================================================================= */


// C++ Util Includes
#include <algorithm>
#include <cmath>

#include "Engine/Core/Main/CoreApplication.h"
#include "Gamefiles/Game/GameEngine.h"



int main () {

    CoreApplication engine;
    GameEngine game;
    
    engine.RunCoreEngine(game);

    return 0;

}


