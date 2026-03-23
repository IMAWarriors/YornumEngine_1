// main.cpp

/**    =======================================================================================================
 *      > MAIN FILE, Version E-Build Alpha 1.1.1
 *      > by IMAWarriors
 *      --------------------------------------
 *      > Stable working version pushed to Git on 03-22-2026, 10:35 PM 
 * 
 *      >>> Most Recently Added Features
 *       - Imported Dear ImGui and rlImGui
 *       - Fixed debug components
 *       - Added Tileset and Atlas splitting capabilities
 *       - Queue for Component/Entity Management
 *       - Stabilized Camera and Render interpolation
 * 
 *      >>> Working-in-Progress Features (as of this push, goals/objectives)
 *       - Trying to add more editor related features for easy debug/expansion
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


