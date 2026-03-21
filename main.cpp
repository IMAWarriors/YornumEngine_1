// main.cpp

/**    =======================================================================================================
 *      > MAIN FILE, Version E-Build Alpha 1.0.1
 *      > by IMAWarriors
 *      --------------------------------------
 *      > LAST STABLE BUILD BEFORE TRYING TO ADD WORLD AND TILE DATA
 *      > Stable working version pushed to Git on 03-19-2029, 2:00 AM 
 * 
 *      >>> Most Recently Added Features
 *       - Queue for Component/Entity Management
 *       - Stabilized Camera and Render interpolation
 *       - Added debug basic overlay
 *       - Split engine and application dependencies
 * 
 *      >>> Working-in-Progress Features (as of this push, goals/objectives)
 *       - Add scene, world, and tile infrastructure!
 * 
 * 
 *    ======================================================================================================= */


// C++ Util Includes
#include <algorithm>
#include <cmath>

#include <cstdlib>   // for rand(), srand()
#include <ctime>

#include "Engine/Core/Main/CoreApplication.h"
#include "Gamefiles/Game/GameEngine.h"



int main () {

    srand(time(NULL));

    CoreApplication engine;
    GameEngine game;
    
    engine.RunCoreEngine(game);

    return 0;

}


