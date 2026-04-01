// main.cpp

/**    =======================================================================================================
 *      > MAIN FILE, Version E-Build Alpha 1.5.1
 *      > by IMAWarriors
 *      --------------------------------------
 *      > Stable working version pushed to Git on 04-01-2026, 12:05 PM 
 * 
 *      >>> Most Recently Added Features
 *       - Cleaned up Registry exception handling
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


