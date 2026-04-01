// main.cpp

/**    =======================================================================================================
 *      > MAIN FILE, Version E-Build Alpha 1.5.2
 *      > E-Build Alpha 1.5.2 [Stabilized Push!]: Fixed out-of-bounds crash because of stale selectedTileIndex after Scene loading
 *     -------------------------------------------------------------------------------------------------------
 *      > by IMAWarriors
 *      >  -------------------------------------------
 *          VERSION 
 *         -------------------------------------------
 *          E-Build Alpha 
 *          1.5.2
 *          [Stabilized Push!]
 *          Fixed out-of-bounds crash because of stale selectedTileIndex after Scene loading
 *      ------------------------------------------------------------------------------------------------------
 *      > Stable working version pushed to Git on 04-01-2026, 12:50 PM 
 *      ======================================================================================================
 *      >>> Most Recently Added Features
 *      ======================================================================================================
 * 
 *       - Fixed crash related to loading scene when selected 
 *          tile data persists about scene loads out of bounds
 * 
 *       - Cleaned up Registry exception handling
 * 
 *       - Implemented Scene saving and loading

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


